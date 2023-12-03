// TOML2Pbf.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "toml.hpp"
#include "ParamBinFile.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

uint32_t fnv1aHash(std::string_view text)
{
    const std::uint32_t prime = 0x01000193; // 16777619
    std::uint32_t hash = 0x811C9DC5; // 2166136261

    for (char c : text) {
        hash ^= static_cast<std::uint32_t>(c);
        hash *= prime;
    }

    return hash;
}

toml::table parseTOMLFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    return toml::parse(file);
}

struct BinaryKeyValuePair
{
    BinaryKeyValuePair() : hashedKey(0), binDataType(PBF::DataTypes::None)
    {
    }
    std::uint32_t hashedKey;
    std::string strValue;
    std::uint8_t value[12];
    PBF::DataTypes binDataType;
};

void writeBinaryFile(const std::vector<BinaryKeyValuePair>& data, const std::string& filePath)
{
    std::ofstream outFile(filePath, std::ios::binary);

    for (const auto& item : data)
    {
        outFile.write(reinterpret_cast<const char*>(&item.hashedKey), sizeof(item.hashedKey));
        // Write the value - this example assumes string values
       // outFile.write(item.value.c_str(), item.value.size());
        // You might need to write a delimiter or length prefix for variable-length data
    }
}

bool canConvertDoubleToFloat(double value)
{
    if (value == 0.00)
    {
        return true;
    }
    // Check if the value is within the range of float
    if (value > static_cast<double>(std::numeric_limits<float>::max()) ||
        value < static_cast<double>(std::numeric_limits<float>::lowest()))
    {
        return false; // Out of range for float
    }

    // Check if precision loss is acceptable
    double valueAsFloat = static_cast<float>(value);
    double precisionLoss = std::abs(value - valueAsFloat) / std::abs(value);

    // Set a threshold for acceptable precision loss
    const double threshold = 1e-8; // Adjust this threshold based on your needs
    
    return precisionLoss < threshold;
}

PBF::DataTypes getInt64type(int64_t value)
{
    using namespace PBF;

    if (value >= 0)
    {
        if (value <= static_cast<int64_t>(UINT8_MAX))
        {
            return DataTypes::UInt8;
        }
        else if (value <= static_cast<int64_t>(UINT16_MAX))
        {
            return DataTypes::UInt16;
        }
        else if (value <= static_cast<int64_t>(UINT32_MAX))
        {
            return DataTypes::UInt32;
        }
        else
        {
            return DataTypes::UInt64;
        }
    }
    else
    {
        if (value >= static_cast<int64_t>(INT8_MIN))
        {
            return DataTypes::Int8;
        }
        else if (value >= static_cast<int64_t>(INT16_MIN))
        {
            return DataTypes::Int16;
        }
        else if (value >= static_cast<int64_t>(INT32_MIN))
        {
            return DataTypes::Int32;
        }
        else
        {
             return DataTypes::Int64;
        }
    }
}

void serializeNormalTypeToBinary(const toml::key& key, const toml::node& value, std::string& p, BinaryKeyValuePair& kvp, std::vector<BinaryKeyValuePair>& binaryData)
{
    switch (value.type())
    {
        case  toml::node_type::string:
        {
            std::optional<std::string>  idt = value.value<std::string>();
            if (idt.has_value())
            {
                kvp.strValue = idt.value();
                kvp.binDataType = PBF::DataTypes::String;
            }
            break;
        }
        case  toml::node_type::integer:
        {
            if (value.is_number())
            {
                std::optional<int64_t>  idt = value.value<int64_t>();
                if (idt.has_value())
                {
                    kvp.binDataType = getInt64type(idt.value());
                    switch (kvp.binDataType)
                    {
                        case PBF::DataTypes::Int8:
                        {
                            int8_t v = static_cast<int8_t>( idt.value() );
                            memset(kvp.value, 0, sizeof(kvp.value));
                            memcpy(kvp.value, &v, sizeof(v));
                            break;
                        }
                        case PBF::DataTypes::UInt8:
                        {
                            uint8_t v = static_cast<uint8_t>(idt.value());
                            memset(kvp.value, 0, sizeof(kvp.value));
                            memcpy(kvp.value, &v, sizeof(v));
                            break;
                        }
                        case PBF::DataTypes::Int16:
                        {
                            int16_t v = static_cast<int16_t>(idt.value());
                            memset(kvp.value, 0, sizeof(kvp.value));
                            memcpy(kvp.value, &v, sizeof(v));
                            break;
                        }
                        case PBF::DataTypes::UInt16:
                        {
                            uint16_t v = static_cast<uint16_t>(idt.value());
                            memset(kvp.value, 0, sizeof(kvp.value));
                            memcpy(kvp.value, &v, sizeof(v));
                            break;
                        }

                        case PBF::DataTypes::Int32:
                        {
                            int32_t v = static_cast<int32_t>(idt.value());
                            memset(kvp.value, 0, sizeof(kvp.value));
                            memcpy(kvp.value, &v, sizeof(v));
                            break;
                        }
                        case PBF::DataTypes::UInt32:
                        {
                            uint32_t v = static_cast<uint32_t>(idt.value());
                            memset(kvp.value, 0, sizeof(kvp.value));
                            memcpy(kvp.value, &v, sizeof(v));
                            break;
                        }
                        case PBF::DataTypes::Int64:
                        {
                            int64_t v = idt.value();
                            memset(kvp.value, 0, sizeof(kvp.value));
                            memcpy(kvp.value, &v, sizeof(v));
                            break;
                        }
                        case PBF::DataTypes::UInt64:
                        {
                            uint64_t v = static_cast<uint64_t>(idt.value());
                            memset(kvp.value, 0, sizeof(kvp.value));
                            memcpy(kvp.value, &v, sizeof(v));
                            break;
                        }
                    default:
                        break;
                    }
                }
            }
            break;
        }
        case  toml::node_type::floating_point:
        {
            if (value.is_number())
            {
                std::optional<float>  idt = value.value<float>();
                if (idt.has_value())
                {
                    if( canConvertDoubleToFloat( idt.value() ) )
                    {
                        float f = idt.value();
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &f, sizeof(f));
                        kvp.binDataType = PBF::DataTypes::Float32;
                    }
                    else
                    {
                        double f = idt.value();
                        memcpy(kvp.value, &f, sizeof(double));
                        kvp.binDataType = PBF::DataTypes::Float64;
                    }
                }
            }
            break;
        }
        case  toml::node_type::boolean:
        {
            std::optional<bool>  idt = value.value<bool>();
            if (idt.has_value())
            {
                bool val = idt.value();
                if (val)
                {
                    memset(kvp.value, 0, sizeof(kvp.value));
                    memcpy(kvp.value, &val, sizeof(bool));
                    kvp.binDataType = PBF::DataTypes::Boolean;
                }
            }
            break;
        }
        case  toml::node_type::date:
        {
            std::optional<toml::date>  idt = value.value<toml::date>();
            if (idt.has_value())
            {
                toml::date d = idt.value();
                kvp.binDataType = PBF::DataTypes::Date;
                uint32_t vdate = (static_cast<uint32_t>(d.year) << 16) | (static_cast<uint32_t>(d.month) << 8) | static_cast<uint32_t>(d.day);                
                memset(kvp.value, 0, sizeof(kvp.value));
                memcpy(kvp.value, &vdate, sizeof(uint32_t));
            }            
            break;
        }
        case  toml::node_type::time:
        {
            std::optional<toml::time>  idt = value.value<toml::time>();
            if (idt.has_value())
            {
                toml::time t = idt.value();
                kvp.binDataType = PBF::DataTypes::Time;
                uint64_t vtime = static_cast<uint64_t>(t.hour) << 48 | static_cast<uint64_t>(t.minute) << 40
                    | static_cast<uint64_t>(t.second) << 32 | static_cast<uint64_t>(t.nanosecond);
                memset(kvp.value, 0, sizeof(kvp.value));
                memcpy(kvp.value, &vtime, sizeof(uint64_t));
            }
            break;
        }
        case  toml::node_type::date_time:
        {
            
            std::optional<toml::date_time>  idt = value.value<toml::date_time>();
            if (idt.has_value())
            {
                toml::date_time dt = idt.value();
                kvp.binDataType = PBF::DataTypes::DateTime;
                uint32_t vdate = (static_cast<uint32_t>(dt.date.year) << 16) |
                                 static_cast<uint32_t>(dt.date.month) << 8 |
                                 static_cast<uint32_t>(dt.date.day);
                uint64_t vtime = static_cast<uint64_t>(dt.time.hour) << 48 |
                                 static_cast<uint64_t>(dt.time.minute) << 40 |
                                 static_cast<uint64_t>(dt.time.second) << 32 |
                                 static_cast<uint64_t>(dt.time.nanosecond);

                memset(kvp.value, 0, sizeof(kvp.value));
                
                void* pDate = static_cast<void*>( kvp.value );
                uint32_t* pDate32 = static_cast<uint32_t*>(pDate);
                *pDate32 = vdate;
                pDate32++;

                void* pTime = static_cast<void*>(pDate32);
                uint64_t* pTime64 = static_cast<uint64_t*>(pTime);
                *pTime64 = vtime;
            }
            break;
        }
        default:
        {
            kvp.binDataType = PBF::DataTypes::None;
            break;
        }
    }
    binaryData.push_back(kvp);
}

void serializeToBinary(toml::table& tomlData, std::string& parent, std::vector<BinaryKeyValuePair>& binaryData)
{   
    for (const auto& [key, value] : tomlData)
    {   
        std::string p;
        if (!parent.empty())
        {
            p = parent + "." + std::string(key.str());
        }
        else
        {
            p = std::string(key.str());
        }
         
        std::uint32_t hk = fnv1aHash(p);
        BinaryKeyValuePair kvp;
        kvp.hashedKey = hk;
        switch (value.type())
        {
            case  toml::node_type::table:
            {
                auto table = value.as_table();
                serializeToBinary(*table, p, binaryData);
                int a = 0;
                a++;
                break;
            }
            case toml::node_type::array:
            {
                auto arr = value.as_array();
                for (size_t i = 0; i < arr->size(); ++i)
                {
                    const auto elem = arr->get(i);
                    std::string arrayElementKey = p + "[" + std::to_string(i) + "]";

                    if (elem->is_table())
                    {
                        serializeToBinary(*elem->as_table(), arrayElementKey, binaryData);
                    }
                    else
                    {                       
                        std::uint32_t hk = fnv1aHash(arrayElementKey);
                        BinaryKeyValuePair kvp;
                        kvp.hashedKey = hk;
                        serializeNormalTypeToBinary(key, value, p, kvp, binaryData);
                    }
                }
                break;
            }
            default:
            {
                serializeNormalTypeToBinary(key, value, p, kvp, binaryData);
                break;
            }
        }
    }
}

int main(int argc, char** argv)
{
    std::string binaryFilePath = "config.bin";

    const auto tomlFilePath = argc > 1 ? std::string{ argv[1] } : "example.toml";

    toml::table tbl;
    std::vector<BinaryKeyValuePair> binaryData;
    binaryData.clear();
    std::string root = "";
    try
    {
        auto tomlData = parseTOMLFile(tomlFilePath);
        serializeToBinary(tomlData, root, binaryData);
        writeBinaryFile(binaryData, binaryFilePath);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}


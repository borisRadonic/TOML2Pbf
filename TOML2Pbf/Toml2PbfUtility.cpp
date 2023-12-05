/******************************************************************************
The MIT License(MIT)

TOML2Pbf
https://github.com/borisRadonic/RTSHA

Copyright(c) 2023 Boris Radonic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#include "Toml2PbfUtility.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace TOML2PBUF
{
    void Toml2PbfUtility::serializeToArray(toml::table& tomlData, std::string& parent)
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

            std::uint32_t hk = PBF::pbfHash(p);
            BinaryKeyValuePair kvp;
            kvp.hashedKey = hk;
            kvp.strKey = p;
            switch (value.type())
            {
                case  toml::node_type::table:
                {
                    auto table = value.as_table();
                    serializeToArray(*table, p);
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
                            serializeToArray(*elem->as_table(), arrayElementKey);
                        }
                        else
                        {
                            const toml::node* node = arr->get(i);
                            std::uint32_t hk = PBF::pbfHash(arrayElementKey);
                            BinaryKeyValuePair kvp;
                            kvp.hashedKey = hk;
                            kvp.strKey = arrayElementKey;
                            toml::node_type nt = node->type();
                            serializeNormalTypeToBinary(key, *node, p, kvp);
                        }
                    }
                    break;
                }
                default:
                {
                    serializeNormalTypeToBinary(key, value, p, kvp);
                    break;
                }
            }
        }
    }

    std::uint32_t Toml2PbfUtility::calculateRequiredMemorySize() const
    {
        std::uint32_t size(0U);

        //header size
        size = PBF::PBF_FILE_HEADER_SIZE;

        //for every record
        for (auto& [key, value] : _key_values)
        {

            size += PBF::PBF_FILE_RECORD_HEADER_SIZE;
            switch (value.binDataType)
            {
            case PBF::DataTypes::String:
            {
                std::uint32_t size2 = (value.size + 1U + sizeof(uint32_t) - 1) / sizeof(uint32_t); // Align to next 32-bit boundary
                size2 = size2 * sizeof(std::uint32_t);
                size += size2;
                break;
            }
            #ifdef ENABLE_PBF_8BIT_TYPES
            case PBF::DataTypes::Int8:
            case PBF::DataTypes::UInt8:
            #endif
            case PBF::DataTypes::Boolean:
            #ifdef ENABLE_PBF_16BIT_TYPES
            case PBF::DataTypes::Int16:
            case PBF::DataTypes::UInt16:
            #endif
            case PBF::DataTypes::Int32:
            case PBF::DataTypes::UInt32:
            case PBF::DataTypes::Float32:
            case PBF::DataTypes::Date:
            {
                size += 4U;
                break;
            }
            case PBF::DataTypes::Int64:
            case PBF::DataTypes::UInt64:
            case PBF::DataTypes::Float64:
            case PBF::DataTypes::Time:
            {
                size += 8U;
                break;
            }
            case PBF::DataTypes::DateTime:
            {
                size += 12U;
                break;;
            }
            default:
                throw new std::exception("Wrong data type!");
            }
        }
        return size;
    }

    bool Toml2PbfUtility::canConvertDoubleToFloat(double value)
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

    PBF::DataTypes Toml2PbfUtility::getInt64type(int64_t value)
    {
        using namespace PBF;

        if (value >= 0)
        {
            #ifdef ENABLE_PBF_8BIT_TYPES
            if (value <= static_cast<int64_t>(UINT8_MAX))
            {
                return DataTypes::UInt8;
            }
            #endif

            #ifdef ENABLE_PBF_16BIT_TYPES
            if (value <= static_cast<int64_t>(UINT16_MAX))
            {
                return DataTypes::UInt16;
            }
            #endif
            
            if (value <= static_cast<int64_t>(UINT32_MAX))
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
            #ifdef ENABLE_PBF_8BIT_TYPES
            if (value >= static_cast<int64_t>(INT8_MIN))
            {
                return DataTypes::Int8;
            }
            #endif

            #ifdef ENABLE_PBF_16BIT_TYPES
            if (value >= static_cast<int64_t>(INT16_MIN))
            {
                return DataTypes::Int16;
            }
            #endif

            if (value >= static_cast<int64_t>(INT32_MIN))
            {
                return DataTypes::Int32;
            }
            else
            {
                return DataTypes::Int64;
            }
        }
    }

    void Toml2PbfUtility::serializeNormalTypeToBinary(const toml::key& key, const toml::node& value, std::string& p, BinaryKeyValuePair& kvp)
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
                kvp.size = static_cast<std::uint32_t>( kvp.strValue.length() );
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
                    kvp.strValue = std::to_string(idt.value());
                    switch (kvp.binDataType)
                    {
                    
                    #ifdef ENABLE_PBF_8BIT_TYPES
                    case PBF::DataTypes::Int8:
                    {
                        int8_t v = static_cast<int8_t>(idt.value());
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &v, sizeof(v));
                        kvp.size = 1U;
                        break;
                    }
                    case PBF::DataTypes::UInt8:
                    {
                        uint8_t v = static_cast<uint8_t>(idt.value());
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &v, sizeof(v));
                        kvp.size = 1U;
                        break;
                    }
                    #endif

                    #ifdef ENABLE_PBF_16BIT_TYPES
                    case PBF::DataTypes::Int16:
                    {
                        int16_t v = static_cast<int16_t>(idt.value());
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &v, sizeof(v));
                        kvp.size = 2U;
                        break;
                    }
                    case PBF::DataTypes::UInt16:
                    {
                        uint16_t v = static_cast<uint16_t>(idt.value());
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &v, sizeof(v));
                        kvp.size = 2U;
                        break;
                    }
                    #endif

                    case PBF::DataTypes::Int32:
                    {
                        int32_t v = static_cast<int32_t>(idt.value());
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &v, sizeof(v));
                        kvp.size = 4U;
                        break;
                    }
                    case PBF::DataTypes::UInt32:
                    {
                        uint32_t v = static_cast<uint32_t>(idt.value());
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &v, sizeof(v));
                        kvp.size = 4U;
                        break;
                    }
                    case PBF::DataTypes::Int64:
                    {
                        int64_t v = idt.value();
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &v, sizeof(v));
                        kvp.size = 8U;
                        break;
                    }
                    case PBF::DataTypes::UInt64:
                    {
                        uint64_t v = static_cast<uint64_t>(idt.value());
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &v, sizeof(v));
                        kvp.size = 8U;
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
                std::optional<double>  idt = value.value<double>();
                if (idt.has_value())
                {
                    kvp.strValue = std::to_string(idt.value());
                    if (canConvertDoubleToFloat(idt.value()))
                    {
                        float f = static_cast<float>(idt.value());
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &f, sizeof(f));
                        kvp.binDataType = PBF::DataTypes::Float32;
                        kvp.size = 4U;
                    }
                    else
                    {
                        double f = idt.value();
                        memset(kvp.value, 0, sizeof(kvp.value));
                        memcpy(kvp.value, &f, sizeof(double));
                        kvp.binDataType = PBF::DataTypes::Float64;
                        kvp.size = 8U;
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
                kvp.strValue = std::to_string(idt.value());
                bool val = idt.value();
                memset(kvp.value, 0, sizeof(kvp.value));
                memcpy(kvp.value, &val, sizeof(bool));
                kvp.binDataType = PBF::DataTypes::Boolean;
                kvp.size = 1U;                
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

                std::tm date = {};
                date.tm_year = static_cast<int>(d.year) - 1900;
                date.tm_mon = static_cast<int>(d.month);
                date.tm_mday = static_cast<int>(d.day);
                kvp.strValue = formatDate(date);

                memset(kvp.value, 0, sizeof(kvp.value));
                memcpy(kvp.value, &vdate, sizeof(uint32_t));
                kvp.size = 4U;
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
                kvp.strValue = formatTime(t.hour, t.minute, t.second, t.nanosecond);
                kvp.size = 8U;
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

                memcpy(kvp.value, &vdate, sizeof(uint32_t));

                uint8_t* pTime = static_cast<uint8_t*>(kvp.value);
                pTime += 4U;

                memcpy(pTime, &vtime, sizeof(uint64_t));
              

                std::tm date = {};
                date.tm_year = static_cast<int>(dt.date.year) - 1900;
                date.tm_mon = static_cast<int>(dt.date.month);
                date.tm_mday = static_cast<int>(dt.date.day);

                kvp.strValue = formatDate(date) + " " + formatTime(dt.time.hour, dt.time.minute, dt.time.second, dt.time.nanosecond);
                kvp.size = 12U;
            }
            break;
        }
        default:
        {
            kvp.binDataType = PBF::DataTypes::None;
            break;
        }
        }
        if (kvp.binDataType != PBF::DataTypes::None)
        {
            auto result = _key_values.insert(std::make_pair(kvp.hashedKey, kvp));
            if (!result.second)
            {
                std::string message = "Key " + kvp.strValue + " already exists.";
                throw new std::exception(message.c_str());
            }
        }
    }

    std::string Toml2PbfUtility::formatDate(const std::tm& date)
    {
        std::ostringstream oss;
        oss << std::put_time(&date, "%Y-%m-%d");
        return oss.str();
    }

    std::string Toml2PbfUtility::formatTime(int hour, int minute, int second, int nanosecond)
    {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << hour << ':'
            << std::setfill('0') << std::setw(2) << minute << ':'
            << std::setfill('0') << std::setw(2) << second << '.'
            << std::setfill('0') << std::setw(9) << nanosecond;
        return oss.str();
    }
}

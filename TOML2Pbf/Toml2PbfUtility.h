#pragma once
#include "toml.hpp"
#include "ParamBinFile.h"
#include <fstream>
#include <iostream>
#include <map>
#include <functional>
#include <algorithm>

namespace TOML2PBUF
{

    struct BinaryKeyValuePair
    {
        BinaryKeyValuePair() : hashedKey(0), binDataType(PBF::DataTypes::None), size(0)
        {
            memset(value, 0, sizeof(value));
        }
        std::uint32_t hashedKey;
        std::string strKey;
        std::string strValue;
        std::uint8_t value[12];
        PBF::DataTypes binDataType;
        std::uint32_t size;
    };

    class Toml2PbfUtility
    {
    public:

        Toml2PbfUtility()
        {
        }

        void serializeToArray(toml::table& tomlData, std::string& parent);

        std::uint32_t calculateRequiredMemorySize() const;

        void forEachElement(std::function<void(BinaryKeyValuePair)> func)
        {
            //for every record
            for (auto& [key, value] : _key_values)
            {
                func(value);
            }
        }

        void forEachElementOrderedByKey(std::function<void(BinaryKeyValuePair)> func)
        {
            std::vector<std::pair<std::uint32_t, BinaryKeyValuePair>> vec(_key_values.begin(), _key_values.end());

            std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) 
            {
                return a.second.strKey < b.second.strKey;
            });

            for (const auto& pair : vec)
            {
                func(pair.second);
            }
        }

    private:

        bool canConvertDoubleToFloat(double value);

        PBF::DataTypes getInt64type(int64_t value);

        void serializeNormalTypeToBinary(const toml::key& key, const toml::node& value, std::string& p, BinaryKeyValuePair& kvp);

        std::string formatDate(const std::tm& date);

        std::string formatTime(int hour, int minute, int second, int nanosecond);

        std::map<std::uint32_t, BinaryKeyValuePair> _key_values;

    };
}
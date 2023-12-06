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

#pragma once
#include "toml.hpp"
#include "ParamBinFileWriter.h"
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
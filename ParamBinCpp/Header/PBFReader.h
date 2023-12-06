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
#include <cstdint>
#include <variant>
#include <string>
#include <chrono>
#include <memory>
#include "Pbf.h"




namespace PBF
{
    #define PBF_MEM_ALIGMENT	4U

    using DataVariant = std::variant <
        std::string,     // String

#ifdef ENABLE_PBF_8BIT_TYPES
        int8_t,          // Int8
        uint8_t,         // UInt8
#endif

#ifdef ENABLE_PBF_16BIT_TYPES
        int16_t,         // Int16
        uint16_t,        // UInt16
#endif

        int32_t,         // Int32
        uint32_t,        // UInt32
        int64_t,         // Int64
        uint64_t,        // UInt64
        float,           // Float32
        double,          // Float64
        bool,            // Boolean
        Date,            // Date
        Time,            // Time
        DateTime         // DateTime
    > ;

    struct VariantBinRecord
    {
        VariantBinRecord()
        {
        }
        DataVariant data;
        DataTypes   type = DataTypes::None; //It field simplifies the code and is used for quick type checks
    };

    class PBFReader
    {
    public:

        PBFReader() : _size(0U), _version(0U)
        {
        }

        bool read(void* memory)
        {
            VariantBinRecord rec;
            std::uint32_t hashKey(0U);
            std::uint32_t done(PBF_FILE_HEADER_SIZE);
            std::uint32_t reg1(0U);
            std::uint32_t data32(0U);

            /*Read size*/
            uint32_t* pMem = static_cast<uint32_t*>(memory);
            _size = *pMem;
            pMem++;

            if (_size == 0U)
            {
                return false;
            }

            std::uint32_t vr = *pMem;
            pMem++;

            _version = vr >> 16U;

            uint32_t reserved = *pMem;;
            pMem++;

            while (done < _size)
            {
                // read the hash
                hashKey = *pMem;
                pMem++;

                reg1 = *pMem;
                std::uint8_t utype = static_cast<std::uint8_t>(reg1 >> 24U);
                rec.type = static_cast<DataTypes>(utype);
                std::uint16_t data_size = static_cast<std::uint16_t>(reg1 & 0x0000FFFF);
                pMem++;
                done += 8U;
                switch (rec.type)
                {
                case DataTypes::String:
                {                    

                    const char* charData = static_cast<const char*>(static_cast<void*>(pMem));
                    std::size_t str_size = strlen(charData);
                    if (data_size < str_size)
                    {
                        return false;
                    }
                    std::string str(charData, str_size);
                    rec.data = str;

                    std::uint16_t size_32 = data_size / (sizeof(std::uint32_t));
                    pMem += size_32;

                    done += data_size;
                    break;
                }
#ifdef ENABLE_PBF_8BIT_TYPES
                case DataTypes::Int8:
                {
                    rec.data = readData32<int8_t>(pMem, done);
                    break;
                }
                case DataTypes::UInt8:
                {
                    rec.data = readData32<uint8_t>(pMem, done);
                    break;
                }
#endif

#ifdef ENABLE_PBF_16BIT_TYPES
                case DataTypes::Int16:
                {
                    rec.data = readData32<int16_t>(pMem, done);
                    break;
                }
                case DataTypes::UInt16:
                {
                    rec.data = readData32<uint16_t>(pMem, done);
                    break;
                }
#endif
                case DataTypes::Int32:
                {
                    rec.data = readData32<int32_t>(pMem, done);
                    break;
                }
                case DataTypes::UInt32:
                {
                    rec.data = readData32<uint32_t>(pMem, done);
                    break;
                }
                case DataTypes::Int64:
                {
                    data32 = *pMem;
                    pMem++;

                    uint64_t data64 = data32;
                    data64 = data64 << 32;
                    data64 = data64 | *pMem;
                    rec.data = static_cast<int64_t>(data64);
                    pMem++;
                    done += 8U;
                    break;
                }
                case DataTypes::UInt64:
                {
                    data32 = *pMem;
                    pMem++;

                    uint64_t data64 = data32;
                    data64 = data64 << 32;
                    data64 = data64 | *pMem;
                    rec.data = data64;
                    pMem++;
                    done += 8U;
                    break;
                }
                case DataTypes::Float32:
                {
                    rec.data = readData32<float>(pMem, done);
                    break;
                }
                case DataTypes::Float64:
                {
                    rec.data = readData64<double>(pMem, done);
                    break;
                }
                case DataTypes::Boolean:
                {
                    rec.data = readData32<bool>(pMem, done);
                    break;
                }
                case DataTypes::Date:
                {                   
                    data32 = readData32<std::uint32_t>(pMem, done);
                    
                    Date date;
                    date.year = static_cast<std::uint16_t>((data32 & 0xFFFF0000) >> 16U);
                    date.month = static_cast<std::uint8_t>((data32 & 0xFF00) >> 8U);
                    date.day = static_cast<std::uint8_t>(data32 & 0xFF);
                    rec.data = date;
                    break;
                }
                case DataTypes::Time:
                {
                    std::uint64_t data64 = readData64<std::uint64_t>(pMem, done);
                    Time time;
                    time.hour = static_cast<std::uint8_t>((data64 & 0xFF000000000000) >> 48U);
                    time.minute = static_cast<std::uint8_t>((data64 & 0x00FF0000000000) >> 40U);
                    time.second = static_cast<std::uint8_t>((data64 & 0x0000FF00000000) >> 32U);
                    time.nanosecond = static_cast<std::uint32_t>((data64 & 0xFFFFFFFF));
                    rec.data = time;
                    break;
                }
                case DataTypes::DateTime:
                {
                    DateTime date_time;

                    data32 = readData32<std::uint32_t>(pMem, done);
                    date_time.date.year = static_cast<std::uint16_t>((data32 & 0xFFFF0000) >> 16U);
                    date_time.date.month = static_cast<std::uint8_t>((data32 & 0xFF00) >> 8U);
                    date_time.date.day = static_cast<std::uint8_t>(data32 & 0xFF);

                    std::uint64_t data64 = readData64<std::uint64_t>(pMem, done);                   
                    date_time.time.hour = static_cast<std::uint8_t>((data64 & 0xFF000000000000) >> 48U);
                    date_time.time.minute = static_cast<std::uint8_t>((data64 & 0x00FF0000000000) >> 40U);
                    date_time.time.second = static_cast<std::uint8_t>((data64 & 0x0000FF00000000) >> 32U);
                    date_time.time.nanosecond = static_cast<std::uint32_t>((data64 & 0xFFFFFFFF));

                    rec.data = date_time;
                   
                    break;
                }
                default:
                {
                    return false;
                }
                }
                auto result = _pairs.insert(std::make_pair(hashKey, rec));
                if (!result.second)
                {
                    return false;
                }
            }
            return true;
        }


        PBF::DataTypes getType(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return DataTypes::None;
            }
            VariantBinRecord vr;
            vr = rec.value();
            return vr.type;
        }

        /*template specialization*/
        template<typename T>
        std::optional<T> getParam(const std::string& str_key)
        {
            // General template, might use static_assert to generate a compile-time error for unsupported types
            static_assert(std::is_same<T, double>::value ||
                std::is_same<T, std::string>::value ||
#ifdef ENABLE_PBF_8BIT_TYPES
                std::is_same<T, std::uint8_t>::value ||
                std::is_same<T, std::int8_t>::value ||
#endif
#ifdef ENABLE_PBF_16BIT_TYPES
                std::is_same<T, std::uint16_t>::value ||
                std::is_same<T, std::int16_t>::value ||
#endif
                std::is_same<T, std::int32_t>::value ||
                std::is_same<T, std::uint32_t>::value ||
                std::is_same<T, std::uint64_t>::value ||
                std::is_same<T, std::int64_t>::value ||
                std::is_same<T, float>::value ||
                std::is_same<T, bool>::value ||
                std::is_same<T, PBF::Date>::value ||
                std::is_same<T, PBF::Time>::value ||
                std::is_same<T, PBF::DateTime>::value ||
                std::is_same<T, std::string>::value, "Unsupported type for getParam");
            return std::nullopt;
        }
              
        // Specialization for std::string
        template<>
        std::optional<std::string> getParam<std::string>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();
            if (vr.type != DataTypes::String)
            {
                return std::nullopt;
            }
            return convertVariantToType<std::string>(rec->data);
        }

        // Specialization for bool
        template<>
        std::optional<bool> getParam<bool>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();
            if (vr.type != DataTypes::Boolean)
            {
                return std::nullopt;
            }
            return convertVariantToType<bool>(rec->data);
        }

        // Specialization for PBF::Date
        template<>
        std::optional<PBF::Date> getParam<PBF::Date>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();
            if (vr.type != DataTypes::Date)
            {
                return std::nullopt;
            }
            return convertVariantToType<PBF::Date>(rec->data);
        }

        // Specialization for PBF::Time
        template<>
        std::optional<PBF::Time> getParam<PBF::Time>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();
            if (vr.type != DataTypes::Time)
            {
                return std::nullopt;
            }
            return convertVariantToType<PBF::Time>(rec->data);
        }

        // Specialization for PBF::DateTime
        template<>
        std::optional<PBF::DateTime> getParam<PBF::DateTime>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();
            if (vr.type != DataTypes::DateTime)
            {
                return std::nullopt;
            }
            return convertVariantToType<PBF::DateTime>(rec->data);
        }


        // Specialization for float
        template<>
        std::optional<float> getParam<float>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();
            if ((vr.type != DataTypes::Float32) && (vr.type != DataTypes::Float64))
            {
                return std::nullopt;
            }

            if (vr.type == DataTypes::Float32)
            {
                return convertVariantToType<float>(rec->data);
            }
            else
            {
                const double* dPtr = std::get_if<double>(&vr.data);
                if (!dPtr)
                {
                    return std::nullopt;
                }
                //check range

                double dbl = *dPtr;

                if (dbl == 0.00)
                {
                    return 0.0f;
                }
                // Check if the value is within the range of float
                if (dbl > static_cast<double>(std::numeric_limits<float>::max()) ||
                    dbl < static_cast<double>(std::numeric_limits<float>::lowest()))
                {
                    return std::nullopt; // Out of range for float
                }

                return static_cast<float>(dbl);
            }
            return std::nullopt;
        }

        // Specialization for double
        template<>
        std::optional<double> getParam<double>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();
            if ((vr.type != DataTypes::Float32) && (vr.type != DataTypes::Float64))
            {
                return std::nullopt;
            }

            if (vr.type == DataTypes::Float32)
            {
                std::optional<float> optf = convertVariantToType<float>(rec->data);
                if (optf == std::nullopt)
                {
                    return std::nullopt;
                }
                return static_cast<double>(optf.value());
            }
            else
            {
                return convertVariantToType<double>(rec->data);
            }

            return std::nullopt;
        }

        // Specialization for std::int32_t
        template<>
        std::optional<std::int32_t> getParam<std::int32_t>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();

            switch (vr.type)
            {
            case DataTypes::Int32:
            {
                return convertVariantToType<std::int32_t>(rec->data);
            }
            case DataTypes::UInt32:
            {
                const std::uint32_t* ui32Ptr = std::get_if<std::uint32_t>(&vr.data);
                std::uint32_t val = *ui32Ptr;

                if (abs((long)val) > static_cast<int32_t>(INT32_MAX))
                {
                    return std::nullopt;
                }
                return static_cast<int32_t>(val);
            }
            case DataTypes::UInt64:
            {
                const std::uint64_t* ui64Ptr = std::get_if<std::uint64_t>(&vr.data);
                std::uint64_t val = *ui64Ptr;

                if (abs((long)val) > static_cast<int32_t>(INT32_MAX))
                {
                    return std::nullopt;
                }
                return static_cast<int32_t>(val);
            }
            case DataTypes::Int64:
            {
                const std::int64_t* ui64Ptr = std::get_if<std::int64_t>(&vr.data);
                std::int64_t val = *ui64Ptr;

                if (abs((long)val) > static_cast<int32_t>(INT32_MAX))
                {
                    return std::nullopt;
                }
                return static_cast<int32_t>(val);
            }
#ifdef ENABLE_PBF_8BIT_TYPES
            case DataTypes::Int8:
            {
                const std::int8_t* i8Ptr = std::get_if<std::int8_t>(&vr.data);
                return static_cast<int32_t>(*i8Ptr);
            }
            case DataTypes::UInt8:
            {
                const std::uint8_t* ui8Ptr = std::get_if<std::uint8_t>(&vr.data);
                return static_cast<int32_t>(*ui8Ptr);
            }
#endif
#ifdef ENABLE_PBF_16BIT_TYPES
            case DataTypes::int16:
            {
                const std::int16_t* i16Ptr = std::get_if<std::int16_t>(&vr.data);
                return static_cast<int32_t>(*i16Ptr);
            }
            case DataTypes::Uint16:
            {
                const std::uint16_t* ui16Ptr = std::get_if<std::uint16_t>(&vr.data);
                return static_cast<int32_t>(*ui16Ptr);
            }
#endif
            default:
            {
                return std::nullopt;
            }
            }
            return std::nullopt;
        }

        // Specialization for std::int64_t
        template<>
        std::optional<std::int64_t> getParam<std::int64_t>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();

            switch (vr.type)
            {
            case DataTypes::Int32:
            {
                const std::int32_t* i32Ptr = std::get_if<std::int32_t>(&vr.data);
                return static_cast<int64_t>(*i32Ptr);
            }
            case DataTypes::UInt32:
            {
                const std::uint32_t* ui32Ptr = std::get_if<std::uint32_t>(&vr.data);
                std::uint32_t val = *ui32Ptr;
                return static_cast<int64_t>(val);
            }
            case DataTypes::UInt64:
            {
                const std::uint64_t* ui64Ptr = std::get_if<std::uint64_t>(&vr.data);
                std::uint64_t val = *ui64Ptr;

                if (abs((long)val) > static_cast<int64_t>(INT64_MAX))
                {
                    return std::nullopt;
                }
                return static_cast<int64_t>(val);
            }
            case DataTypes::Int64:
            {
                const std::int64_t* i64Ptr = std::get_if<std::int64_t>(&vr.data);
                return static_cast<int64_t>(*i64Ptr);
            }
#ifdef ENABLE_PBF_8BIT_TYPES
            case DataTypes::Int8:
            {
                const std::int8_t* i8Ptr = std::get_if<std::int8_t>(&vr.data);
                return static_cast<int64_t>(*i8Ptr);
            }
            case DataTypes::UInt8:
            {
                const std::uint8_t* ui8Ptr = std::get_if<std::uint8_t>(&vr.data);
                return static_cast<int64_t>(*ui8Ptr);
            }
#endif
#ifdef ENABLE_PBF_16BIT_TYPES
            case DataTypes::int16:
            {
                const std::int16_t* i16Ptr = std::get_if<std::int16_t>(&vr.data);
                return static_cast<int64_t>(*i16Ptr);
            }
            case DataTypes::Uint16:
            {
                const std::uint16_t* ui16Ptr = std::get_if<std::uint16_t>(&vr.data);
                return static_cast<int64_t>(*ui16Ptr);
            }
#endif
            default:
            {
                return std::nullopt;
            }
            }
            return std::nullopt;
        }

        // Specialization for std::uint32_t
        template<>
        std::optional<std::uint32_t> getParam<std::uint32_t>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();

            switch (vr.type)
            {
            case DataTypes::UInt32:
            {
                return convertVariantToType<std::uint32_t>(rec->data);
            }
            case DataTypes::Int32:
            {
                const std::int32_t* i32Ptr = std::get_if<std::int32_t>(&vr.data);
                std::int32_t val = *i32Ptr;

                if (val < 0)
                {
                    return std::nullopt;
                }
                return static_cast<uint32_t>(val);
            }
            case DataTypes::UInt64:
            {
                const std::uint64_t* ui64Ptr = std::get_if<std::uint64_t>(&vr.data);
                std::uint64_t val = *ui64Ptr;

                if (abs((long)val) > static_cast<uint32_t>(UINT32_MAX))
                {
                    return std::nullopt;
                }
                return static_cast<uint32_t>(val);
            }
            case DataTypes::Int64:
            {
                const std::int64_t* ui64Ptr = std::get_if<std::int64_t>(&vr.data);
                std::int64_t val = *ui64Ptr;
                if (val < 0)
                {
                    return std::nullopt;
                }

                if (abs((long)val) > static_cast<uint32_t>(UINT32_MAX))
                {
                    return std::nullopt;
                }
                return static_cast<uint32_t>(val);
            }
#ifdef ENABLE_PBF_8BIT_TYPES
            case DataTypes::Int8:
            {
                const std::int8_t* i8Ptr = std::get_if<std::int8_t>(&vr.data);
                std::int8_t val = *i8Ptr;
                if (val < 0)
                {
                    return std::nullopt;
                }
                return static_cast<uint32_t>(val);
            }
            case DataTypes::UInt8:
            {
                const std::uint8_t* ui8Ptr = std::get_if<std::uint8_t>(&vr.data);
                return static_cast<uint32_t>(*ui8Ptr);
            }
#endif
#ifdef ENABLE_PBF_16BIT_TYPES
            case DataTypes::int16:
            {
                const std::int16_t* i16Ptr = std::get_if<std::int16_t>(&vr.data);
                std::int16_t val = *i16Ptr;
                if (val < 0)
                {
                    return std::nullopt;
                }
                return static_cast<uint32_t>(*i16Ptr);
            }
            case DataTypes::Uint16:
            {
                const std::uint16_t* ui16Ptr = std::get_if<std::uint16_t>(&vr.data);
                return static_cast<uint32_t>(*ui16Ptr);
            }
#endif
            default:
            {
                return std::nullopt;
            }
            }
            return std::nullopt;
        }

        // Specialization for std::uint64_t
        template<>
        std::optional<std::uint64_t> getParam<std::uint64_t>(const std::string& str_key)
        {
            auto rec = getRecord(str_key);
            if (!rec)
            {
                return std::nullopt;
            }
            VariantBinRecord vr;
            vr = rec.value();

            switch (vr.type)
            {
            case DataTypes::UInt32:
            {
                const std::uint32_t* ui32Ptr = std::get_if<std::uint32_t>(&vr.data);
                return static_cast<uint64_t> (*ui32Ptr);
            }
            case DataTypes::Int32:
            {
                const std::int32_t* i32Ptr = std::get_if<std::int32_t>(&vr.data);
                std::int32_t val = *i32Ptr;

                if (val < 0)
                {
                    return std::nullopt;
                }
                return static_cast<uint64_t>(val);
            }
            case DataTypes::Int64:
            {
                const std::int64_t* i64Ptr = std::get_if<std::int64_t>(&vr.data);
                std::int64_t val = *i64Ptr;

                if (abs((long)val) > static_cast<uint64_t>(UINT64_MAX))
                {
                    return std::nullopt;
                }
                return static_cast<uint64_t>(val);
            }
            case DataTypes::UInt64:
            {
                return convertVariantToType<std::uint64_t>(rec->data);
            }
#ifdef ENABLE_PBF_8BIT_TYPES
            case DataTypes::Int8:
            {
                const std::int8_t* i8Ptr = std::get_if<std::int8_t>(&vr.data);
                std::int8_t val = *i8Ptr;
                if (val < 0)
                {
                    return std::nullopt;
                }
                return static_cast<uint64_t>(val);
            }
            case DataTypes::UInt8:
            {
                const std::uint8_t* ui8Ptr = std::get_if<std::uint8_t>(&vr.data);
                return static_cast<uint64_t>(*ui8Ptr);
            }
#endif
#ifdef ENABLE_PBF_16BIT_TYPES
            case DataTypes::int16:
            {
                const std::int16_t* i16Ptr = std::get_if<std::int16_t>(&vr.data);
                std::int16_t val = *i16Ptr;
                if (val < 0)
                {
                    return std::nullopt;
                }
                return static_cast<uint64_t>(*i16Ptr);
            }
            case DataTypes::Uint16:
            {
                const std::uint16_t* ui16Ptr = std::get_if<std::uint16_t>(&vr.data);
                return static_cast<uint64_t>(*ui16Ptr);
            }
#endif
            default:
            {
                return std::nullopt;
            }
            }
            return std::nullopt;
        }

    private:

        std::optional<VariantBinRecord> getRecord(const std::string& str_key)
        {
            std::string_view strView(str_key);
            std::uint32_t key = pbfHash(strView);
            auto it = _pairs.find(key);
            if (it == _pairs.end())
            {
                return std::nullopt;
            }
            return it->second;
        }

        template<typename T>
        std::optional<T> convertVariantToType(const DataVariant& variantData)
        {
            if (auto ptr = std::get_if<T>(&variantData))
            {
                return *ptr;
            }
            return std::nullopt;
        }

        template<typename T>
        T readData32(std::uint32_t*& pMem, std::uint32_t& done)
        {
            // Generic implementation for simple types like 32 bit integers and floats
            T data;
            std::memcpy(&data, pMem, sizeof(T));
            pMem++;
            done += sizeof(std::uint32_t);
            return data;
        }

        template<typename T>
        T readData64(std::uint32_t*& pMem, std::uint32_t& done)
        {
            // Generic implementation for simple types like 64 bit integers and double
            T data;
            std::memcpy(&data, pMem, sizeof(T));
            pMem++;
            pMem++;
            done += sizeof(std::uint64_t);
            return data;
        }

        std::uint32_t _size = 0U;
        std::uint16_t _version = 0U;
        std::map<std::uint32_t, VariantBinRecord> _pairs;
    };
}


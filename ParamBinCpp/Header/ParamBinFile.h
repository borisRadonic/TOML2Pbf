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
#include <cstdint>
#include <variant>
#include <string>
#include <chrono>
#include <memory>

#define MIN_PBF_INT_SIZE 32

namespace PBF
{
    const std::uint32_t PBF_FILE_HEADER_SIZE = sizeof(std::uint32_t) * 3U;
    const std::uint32_t PBF_FILE_RECORD_HEADER_SIZE = 8U;

    const std::uint16_t PBF_FILE_VERSION = 1U;

    inline std::uint32_t pbfHash(std::string_view text)
    {
        //fnv1aHash
        const std::uint32_t prime = 0x01000193; // 16777619
        std::uint32_t hash = 0x811C9DC5; // 2166136261

        for (char c : text)
        {
            hash ^= static_cast<std::uint32_t>(c);
            hash *= prime;
        }
        return hash;
    };

    // Define some compound types
    struct Date
    {
        uint16_t year = 0;
        uint8_t month = 0;
        uint8_t day = 0;
    };

    struct Time
    {
        uint8_t reserved = 0;
        uint8_t hour = 0;
        uint8_t minute = 0;
        uint8_t second = 0;
        uint32_t nanosecond = 0;
    };

    struct DateTime
    {
        Date date;
        Time time;
    };

    /**
     * @enum DataTypes
     * @brief Enumeration for different data types supported in binary data records.
     *
     * This enumeration defines various data types that can be stored in binary data records.
     * Each enum value represents a specific type of data, like integers, floating-point values,
     * booleans, dates, and times, with specific sizes and formats.
     */
    enum class DataTypes
    {
        String = 1, /**< String */
        #ifdef ENABLE_PBF_8BIT_TYPES
        Int8 = 2, /**< 8-bit signed integer. */
        UInt8 = 3, /**< 8-bit unsigned integer. */
        #endif

        #ifdef ENABLE_PBF_16BIT_TYPES
        Int16 = 4, /**< 16-bit signed integer. */
        UInt16 = 5, /**< 16-bit unsigned integer. */
        #endif

        Int32 = 6, /**< 32-bit signed integer. */
        UInt32 = 7, /**< 32-bit unsigned integer. */

        Int64 = 8, /**< 64-bit signed integer. */
        UInt64 = 9, /**< 64-bit unsigned integer. */

        Float32 = 10, /**< 32-bit floating-point number. */

        Float64 = 11, /**< 64-bit floating-point number. */

        Boolean = 12, /**< Boolean type (1 Byte). */
        Date = 13, /**< Date type, stored as a uint32_t (year 16 bits, month 8 bits, day 8 bits). */
        Time = 14, /**< Time type, stored as a uint64_t (reserved 8 bits, hour (0-23) 8 bits, minute (0-59) 8 bits, second (0-59) 8 bits, nanosecond (0 - 999999999) 32 bits). */
        DateTime = 15, /**< DateTime type, combining uint32_t Date and uint64_t Time. */
        None = 0  /**< Represents no type. */
    };

    /**
     * @struct BinaryDataRecord
     * @brief Structure representing a single record of binary data.
     *
     * This structure defines a binary data record with a hash, type, and data.
     * It is used to store a single piece of data along with its type and size information.
     * The actual data is represented as a byte array of variable length.
     */
    struct BinaryDataRecord
    {
        std::uint32_t hash; /**< Hash of the data. */
        std::uint8_t type = static_cast<std::uint8_t>(DataTypes::None); /**< Data type, represented as a byte. */
        std::uint8_t reserved = 0U;
        std::uint16_t data_size = 0U; /**< Size of the data in bytes (from 1 to 65536). */
        std::string strData;
        //... data from Min. 2 to Max. 65536 bytes (Min. size with 32 bit padding)
    };

    //Structure of the param file is
    
    // Header
    //  4 bytes (UInt32)  Size (Including first 4 bytes for Size)
    //  4 bytes (UInt32)  2 bytes Version + 2 bytes Reserved
    //  4 bytes (UInt32)  Reserved
    // Binary Records
    //  ...


    inline std::string getTypeName(PBF::DataTypes type)
    {
        switch (type)
        {
#ifdef ENABLE_PBF_8BIT_TYPES
        case PBF::DataTypes::Int8:
        {
            return "Int8";
        }
        case PBF::DataTypes::UInt8:
        {
            return "UInt8";
        }
#endif

#ifdef ENABLE_PBF_16BIT_TYPES
        case PBF::DataTypes::Int16:
        {
            return "Int16";
        }
        case PBF::DataTypes::UInt16:
        {
            return "UInt16";
        }
#endif
        case PBF::DataTypes::Boolean:
        {
            return "Boolean";
        }
        case PBF::DataTypes::Date:
        {
            return "Date";
        }
        case PBF::DataTypes::DateTime:
        {
            return "DateTime";
        }
        case PBF::DataTypes::Time:
        {
            return "Time";
        }
        case PBF::DataTypes::Float32:
        {
            return "float";
        }
        case PBF::DataTypes::Float64:
        {
            return "double";
        }
        case PBF::DataTypes::Int32:
        {
            return "Int32";
        }
        case PBF::DataTypes::UInt32:
        {
            return "UInt32";
        }
        case PBF::DataTypes::Int64:
        {
            return "Int64";
        }
        case PBF::DataTypes::UInt64:
        {
            return "UInt64";
        }
        case PBF::DataTypes::String:
        {
            return "String";
        }
        case PBF::DataTypes::None:
        {
            return "None";
        }
        default:
            break;
        }
        return "Unknow!!!";
    }

	class ParamBinFileWriter
	{
    public:

        ParamBinFileWriter() = delete;

        explicit ParamBinFileWriter (void* memory, std::size_t size): _memory(memory), _start(memory)
        {
        }

        std::uint32_t writeHeader(std::uint32_t size, uint16_t version)
        {            
            if (_start != nullptr)
            {
                void* mem = _start;
                uint32_t* pMem = static_cast<uint32_t*>(mem);
                memcpy(pMem, static_cast<void*>(&size), sizeof(uint32_t));
                pMem++;

                uint32_t v = version << 16U;
                memcpy(pMem, static_cast<void*>(&v), sizeof(uint32_t));
                pMem++;

                uint32_t reserved(0U);
                memcpy(pMem, static_cast<void*>(&reserved), sizeof(uint32_t));
                pMem++;
 
                _memory = static_cast<void*>(pMem);

                return PBF_FILE_HEADER_SIZE;
            }
            return 0U;
        }

        std::uint32_t writeRecord(BinaryDataRecord& record, void* data)
        {
            uint32_t* pMem = static_cast<uint32_t*>(_memory);
            // Write the hash
            memcpy(pMem, static_cast<void*>(&(record.hash)), sizeof(uint32_t));
            pMem++;

            std::uint32_t recSize = PBF_FILE_RECORD_HEADER_SIZE;

            DataTypes type = static_cast<DataTypes>(record.type);
            switch (type)
            {
                case DataTypes::String:
                {
                    // Handle string; assume data_size gives the string length
                    char* str = const_cast<char*>( record.strData.c_str() );
 
                    size_t slen = strlen(str);

                    /*Always add \0 at the end of string*/
                    std::uint32_t a_size_32 = (record.data_size + 1U + sizeof(uint32_t) - 1) / sizeof(uint32_t); // Align to next 32-bit boundary
                    std::uint32_t a_size = a_size_32 * sizeof(std::uint32_t);

                    std::uint32_t reg1 = (record.type << 24U) | a_size;
                    memcpy(pMem, static_cast<void*>(&reg1), sizeof(uint32_t));
                    pMem++;
                                        
                    memset(pMem, 0U, a_size);
                    memcpy(pMem, static_cast<void*>(str), record.data_size);

                    pMem += a_size_32;
                    recSize += a_size;
                    break;
                }
                #ifdef ENABLE_PBF_8BIT_TYPES
                case DataTypes::Int8:
                case DataTypes::UInt8:
                #endif
                case DataTypes::Boolean:
                {
                    uint8_t byte = *static_cast<std::uint8_t*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    memcpy(pMem, static_cast<void*>(&reg1), sizeof(uint32_t));
                    pMem++;

                    std::uint32_t reg2 = byte;
                    memcpy(pMem, static_cast<void*>(&reg2), sizeof(uint32_t));
                    pMem++;

                    recSize += 4U;

                    break;
                }
                #ifdef ENABLE_PBF_16BIT_TYPES
                case DataTypes::Int16:
                case DataTypes::UInt16:
                {
                    std::uint16_t reg2 = *static_cast<std::uint16_t*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    memcpy(pMem, static_cast<void*>(&reg1), sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, static_cast<void*>(&reg2), sizeof(uint32_t));
                    pMem++;

                    recSize += 4U;

                    break;
                }
                #endif

                case DataTypes::Int32:
                case DataTypes::UInt32:
                case DataTypes::Float32:
                {
                    std::uint32_t reg2 = *static_cast<std::uint32_t*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    memcpy(pMem, static_cast<void*>(&reg1), sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, static_cast<void*>(&reg2), sizeof(uint32_t));
                    pMem++;

                    recSize += 4U;

                    break;
                }
                case DataTypes::Date:
                {
                    std::uint32_t reg2 = *static_cast<std::uint32_t*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;

                    memcpy(pMem, static_cast<void*>(&reg1), sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, static_cast<void*>(&reg2), sizeof(uint32_t));
                    pMem++;

                    recSize += 4U;

                    break;
                }
                case DataTypes::Int64:
                case DataTypes::UInt64:
                case DataTypes::Float64:
                case DataTypes::Time:
                {
                    std::uint64_t reg2 = *static_cast<std::uint64_t*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;

                    memcpy(pMem, static_cast<void*>(&reg1), sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, static_cast<void*>(&reg2), sizeof(uint64_t));
                    pMem++;
                    pMem++;

                    recSize += 8U;

                    break;
                }
                case DataTypes::DateTime:
                {

                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    uint32_t* pDate32 = static_cast<uint32_t*>(data);
                  
                    memcpy(pMem, static_cast<void*>(&reg1), sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, static_cast<void*>(pDate32), sizeof(uint32_t));
                    pMem++;

                    pDate32++;

                    memcpy(pMem, static_cast<void*>(pDate32), sizeof(uint64_t)); //use 32 bit pointer but copy 64 bits (8 bytes)
                    pMem++;
                    pMem++;

                    recSize += 12U;

                    break;;
                }
                default:
                    break;
            }
            _memory = static_cast<void*>(pMem);

            return recSize;
        }

    private:       
        void* _memory = nullptr;
        void* _start = nullptr;
 	};

    using DataVariant = std::variant<
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
    >;


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
                        data32 = *pMem;
                        pMem++;
                        rec.data = static_cast<int8_t>(data32);
                        done += 4U;
                        break;
                    }
                    case DataTypes::UInt8:
                    {
                        data32 = *pMem;
                        pMem++;
                        rec.data = static_cast<uint8_t>(data32);
                        done += 4U;
                        break;
                    }
                    #endif

                    #ifdef ENABLE_PBF_16BIT_TYPES
                    case DataTypes::Int16:
                    {
                        data32 = *pMem;
                        pMem++;
                        rec.data = static_cast<int16_t>(data32);
                        done += 4U;
                        break;
                    }
                    case DataTypes::UInt16:
                    {
                        data32 = *pMem;
                        pMem++;
                        rec.data = static_cast<uint16_t>(data32);
                        done += 4U;
                        break;
                    }
                    #endif

                    case DataTypes::Int32:
                    {
                        data32 = *pMem;
                        pMem++;
                        rec.data = static_cast<int32_t>(data32);
                        done += 4U;
                        break;
                    }
                    case DataTypes::UInt32:
                    {
                        data32 = *pMem;
                        pMem++;
                        rec.data = static_cast<uint32_t>(data32);
                        done += 4U;
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
                        float floatValue(0.0);
                        std::memcpy(&floatValue, pMem, sizeof(float));
                        rec.data = floatValue;
                        pMem++;
                        done += 4U;
                        break;
                    }
                    case DataTypes::Float64:
                    {
                        double dValue(0.0);
                        std::memcpy(&dValue, pMem, sizeof(double));
                        rec.data = dValue;
                        pMem++;
                        pMem++;
                        done += 8U;
                        break;
                    }
                    case DataTypes::Boolean:
                    {
                        data32 = *pMem;
                        pMem++;                       
                        rec.data = static_cast<bool>(data32);
                        done += 4U;
                        break;
                    }
                    case DataTypes::Date:
                    {
                        data32 = *pMem;
                        pMem++;

                        Date date;
                        date.year = static_cast<uint16_t>((data32 & 0xFFFF0000) >> 16U);
                        date.month = static_cast<uint8_t>((data32 & 0xFF00) >> 8U);
                        date.day = static_cast<uint8_t>(data32 & 0xFF);
                        rec.data = date;

                        done += 4U;
                        break;
                    }
                    case DataTypes::Time:
                    {
                        uint64_t data64(0);
                        std::memcpy(&data64, pMem, sizeof(uint64_t));
                        pMem++;
                        pMem++;
                        Time time;
                        time.hour       = static_cast<uint8_t>((data64 & 0xFF000000000000) >> 48U);
                        time.minute     = static_cast<uint8_t>((data64 & 0x00FF0000000000) >> 40U);
                        time.second     = static_cast<uint8_t>((data64 & 0x0000FF00000000) >> 32U);
                        time.nanosecond = static_cast<uint32_t>((data64 & 0xFFFFFFFF));
                        rec.data = time;
                        done += 8U;
                        break;
                    }
                    case DataTypes::DateTime:
                    {
                        DateTime date_time;

                        data32 = *pMem;
                        pMem++;

                        date_time.date.year = static_cast<uint16_t>((data32 & 0xFFFF0000) >> 16U);
                        date_time.date.month = static_cast<uint8_t>((data32 & 0xFF00) >> 8U);
                        date_time.date.day = static_cast<uint8_t>(data32 & 0xFF);
                        
                        uint64_t data64(0);
                        std::memcpy(&data64, pMem, sizeof(uint64_t));
                        pMem++;
                        pMem++;

                        date_time.time.hour = static_cast<uint8_t>((data64 & 0xFF000000000000) >> 48U);
                        date_time.time.minute = static_cast<uint8_t>((data64 & 0x00FF0000000000) >> 40U);
                        date_time.time.second = static_cast<uint8_t>((data64 & 0x0000FF00000000) >> 32U);
                        date_time.time.nanosecond = static_cast<uint32_t>((data64 & 0xFFFFFFFF));

                        rec.data = date_time;
                        
                        done += 12U;
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
            static_assert(std::is_same<T, double>::value|| 
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
           
            // Safely extract std::string from the variant
            const std::string* strPtr = std::get_if<std::string>(&vr.data);
            if (strPtr)
            {
                return *strPtr; // Dereference the pointer to get the string
            }
            return std::nullopt;
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
            const bool* b = std::get_if<bool>(&vr.data);
            if (b)
            {
                return *b;
            }
            return std::nullopt;
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
            const PBF::Date* d = std::get_if<PBF::Date>(&vr.data);
            if (d)
            {
                return *d; // Dereference the pointer to get the string
            }
            return std::nullopt;
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
            const PBF::Time* t = std::get_if<PBF::Time>(&vr.data);
            if (t)
            {
                return *t; // Dereference the pointer to get the string
            }
            return std::nullopt;
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
            const PBF::DateTime* dt= std::get_if<PBF::DateTime>(&vr.data);
            if (dt)
            {
                return *dt; // Dereference the pointer to get the string
            }
            return std::nullopt;
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
                const float* fPtr = std::get_if<float>(&vr.data);
                if (!fPtr)
                {
                    return std::nullopt;
                }
                return *fPtr;
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
            if ( (vr.type != DataTypes::Float32) && (vr.type != DataTypes::Float64) )
            {
                return std::nullopt;
            }

            if( vr.type == DataTypes::Float32 )
            {
                const float* fPtr = std::get_if<float>(&vr.data);
                if (!fPtr)
                {
                    return std::nullopt;
                }
                return static_cast<double>(*fPtr);
            }
            else
            {
                const double* dPtr = std::get_if<double>(&vr.data);
                if (!dPtr)
                {
                    return std::nullopt;
                }
                return *dPtr;
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
                    const std::int32_t* i32Ptr = std::get_if<std::int32_t>(&vr.data);
                    return *i32Ptr;
                }
                case DataTypes::UInt32:
                {
                    const std::uint32_t* ui32Ptr = std::get_if<std::uint32_t>(&vr.data);
                    std::uint32_t val = *ui32Ptr;

                    if (abs((long) val) > static_cast<int32_t>(INT32_MAX))
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
                const std::uint32_t* ui32Ptr = std::get_if<std::uint32_t>(&vr.data);
                return *ui32Ptr;
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
                const std::uint64_t* ui64Ptr = std::get_if<std::uint64_t>(&vr.data);
                return *ui64Ptr;
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

        std::uint32_t _size = 0U;
        std::uint16_t _version = 0U;
        std::map<std::uint32_t, VariantBinRecord> _pairs;
    };
}
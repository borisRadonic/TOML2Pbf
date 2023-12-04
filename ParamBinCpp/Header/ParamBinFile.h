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
#include<memory>

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
        Int8 = 2, /**< 8-bit signed integer. */
        UInt8 = 3, /**< 8-bit unsigned integer. */
        Int16 = 4, /**< 16-bit signed integer. */
        UInt16 = 5, /**< 16-bit unsigned integer. */
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
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    memcpy(pMem, static_cast<void*>(&reg1), sizeof(uint32_t));
                    pMem++;
                                       
                    memcpy(pMem, static_cast<void*>(str), record.data_size);

                    std::uint32_t size = (record.data_size + sizeof(uint32_t) - 1) / sizeof(uint32_t); // Align to next 32-bit boundary
                    pMem += size;
                    size = size * sizeof(std::uint32_t);
                    recSize += size;

                    break;
                }
                case DataTypes::Int8:
                case DataTypes::UInt8:
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
                case DataTypes::Int32:
                case DataTypes::UInt32:
                case DataTypes::Float32:
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
                    pDate32++;

                    memcpy(pMem, static_cast<void*>(&reg1), sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, static_cast<void*>(pDate32), sizeof(uint32_t));
                    pMem++;

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
}


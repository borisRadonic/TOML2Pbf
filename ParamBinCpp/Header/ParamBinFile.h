#pragma once
#include <cstdint>
#include<memory>

namespace PBF
{
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
       
        //... data from Min. 1 to Max. 250 bytes (Min. size with padding is 2)
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

        explicit ParamBinFileWriter (void* memory): _memory(memory), _start(memory)
        {
        }

        void writeHeader(std::uint32_t size, uint16_t version)
        {            
            if (_start != nullptr)
            {
                void* mem = _start;
                uint32_t* pMem = static_cast<uint32_t*>(mem);
                memcpy(pMem, &size, sizeof(uint32_t));
                pMem++;

                uint32_t v = version << 16U;
                memcpy(pMem, &v, sizeof(uint32_t));
                pMem++;

                uint32_t reserved(0U);
                memcpy(pMem, &reserved, sizeof(uint32_t));
                pMem++;
 
                _memory = static_cast<void*>(pMem);
            }
        }
        void writeRecord(BinaryDataRecord& record, void* data)
        {
            uint32_t* pMem = static_cast<uint32_t*>(_memory);
            // Write the hash
            memcpy(pMem, &(record.hash), sizeof(uint32_t));
            pMem++;

           /*
            std::uint32_t hash
            std::uint8_t type
            std::uint8_t reserved
            std::uint16_t data_size
           */

            DataTypes type = static_cast< DataTypes>(record.type);
            switch (type)
            {
                case DataTypes::String:
                {
                    // Handle string; assume data_size gives the string length
                    char* str = static_cast<char*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    memcpy(pMem, &reg1, sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, str, record.data_size);
                    pMem += (record.data_size + sizeof(uint32_t) - 1) / sizeof(uint32_t); // Align to next 32-bit boundary
                    
                    break;
                }
                case DataTypes::Int8:
                case DataTypes::UInt8:
                case DataTypes::Boolean:
                {
                    uint8_t byte = *static_cast<std::uint8_t*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    memcpy(pMem, &reg1, sizeof(uint32_t));
                    pMem++;

                    std::uint32_t reg2 = byte;
                    memcpy(pMem, &reg2, sizeof(uint32_t));
                    pMem++;

                    break;
                }               
                case DataTypes::Int16:
                case DataTypes::UInt16:
                {
                    std::uint16_t word = *static_cast<std::uint16_t*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    memcpy(pMem, &reg1, sizeof(uint32_t));
                    pMem++;

                    std::uint32_t reg2 = word;
                    memcpy(pMem, &reg2, sizeof(uint32_t));
                    pMem++;

                    break;
                }
                case DataTypes::Int32:
                case DataTypes::UInt32:
                case DataTypes::Float32:
                case DataTypes::Date:
                {
                    std::uint32_t reg2 = *static_cast<std::uint32_t*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    memcpy(pMem, &reg1, sizeof(uint32_t));
                    pMem++;
                                        
                    memcpy(pMem, &reg2, sizeof(uint32_t));
                    pMem++;

                    break;
                }
                case DataTypes::Int64:
                case DataTypes::UInt64:
                case DataTypes::Float64:
                case DataTypes::Time:
                {
                    std::uint64_t reg2 = *static_cast<std::uint64_t*>(data);
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                   
                    memcpy(pMem, &reg1, sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, &reg2, sizeof(uint64_t));
                    pMem++;
                    pMem++;
                   
                    break;
                }
                case DataTypes::DateTime:
                {
                   
                    std::uint32_t reg1 = (record.type << 24U) | record.data_size;
                    uint32_t* pDate32 = static_cast<uint32_t*>(data);
                    pDate32++;
                   
                    memcpy(pMem, &reg1, sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, pDate32, sizeof(uint32_t));
                    pMem++;

                    memcpy(pMem, pDate32, sizeof(uint64_t)); //use 32 bit pointer but copy 64 bits (8 bytes)
                    pMem++;
                    pMem++;

                    break;;
                }
                default:
                    break;
            }

            _memory = static_cast<void*>(pMem);
        }

    private:       
        void* _memory = nullptr;
        void* _start = nullptr;
 	};
}


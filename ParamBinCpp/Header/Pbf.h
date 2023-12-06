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
#include <string>

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
}

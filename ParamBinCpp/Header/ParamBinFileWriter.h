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
#include <string>
#include <chrono>
#include <memory>
#include "Pbf.h"

namespace PBF
{
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

        std::uint32_t writeRecord(PBF::BinaryDataRecord& record, void* data)
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
}
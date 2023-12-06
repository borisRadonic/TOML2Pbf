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
#include "toml.hpp"
#include "Toml2PbfUtility.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <string>
#include <memory>

toml::table parseTOMLFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    return toml::parse(file);
}

std::string changeFileExtension(const std::string& filename, const std::string& newExtension)
{
    std::size_t lastDotIndex = filename.find_last_of(".");
    if (lastDotIndex != std::string::npos) {
        return filename.substr(0, lastDotIndex) + newExtension;
    }
    return filename + newExtension;
}

int main(int argc, char** argv)
{
    using namespace TOML2PBUF;
    using namespace PBF;

    if (argc <= 1)
    {
        std::cerr << "Usage: " << argv[0] << " <inputfile.toml>" << std::endl;
        return 1;
    }
    std::string inputFilePath = argv[1];

    std::size_t lastDotIndex = inputFilePath.find_last_of(".toml");
    if (lastDotIndex == std::string::npos)
    {
        std::cerr << "Usage: " << argv[0] << " <inputfile.toml>" << std::endl;
        return 1;
    }

    std::string outputFilePathPbf = changeFileExtension(inputFilePath, ".pbf");
    std::string outputFilePathRpt = changeFileExtension(inputFilePath, ".rpt");


    std::ofstream outputFileRpt(outputFilePathRpt);

    if (!outputFileRpt.is_open())
    {
        std::cerr << "Could not create and open output file." << std::endl;
        return 1;
    }

    std::string root = "";

    Toml2PbfUtility util;
    try
    {
        auto tomlData = parseTOMLFile(inputFilePath);
        util.serializeToArray(tomlData, root);
        

        outputFileRpt << "Key  hash_value" << std::endl;
        /*write in intermediate stage output info file*/ 
        util.forEachElementOrderedByKey([&outputFileRpt](BinaryKeyValuePair elem)
        {
           outputFileRpt << elem.strKey << "\t" << std::to_string(elem.hashedKey) << std::endl;
        });
    
        // Close the file stream
        outputFileRpt.close();

        
        std::uint32_t mem_size = util.calculateRequiredMemorySize();
        if (mem_size == 0U)
        {
            std::cerr << "Error: File is empty." << std::endl;
            return -1;
        }

        std::unique_ptr<uint8_t[]> mem = std::make_unique<uint8_t[]>(mem_size);
        std::ofstream outFile(outputFilePathPbf, std::ios::binary);


        ParamBinFileWriter writer ( static_cast<void*>( mem.get() ), mem_size);
        std::uint32_t written = writer.writeHeader(mem_size, PBF_FILE_VERSION);

        util.forEachElement(   [&writer, &written, &mem_size](BinaryKeyValuePair elem)
                               {
                                    BinaryDataRecord record;
                                    record.strData = elem.strValue;                                    
                                    record.data_size = elem.size;
                                    record.hash = elem.hashedKey;
                                    record.type = static_cast<std::uint8_t>(elem.binDataType);
                                    written += writer.writeRecord(record, static_cast<void*>(elem.value));
                                    if (written > mem_size)
                                    {
                                        throw new std::exception("Wrong memory size calculated!");
                                    }
                               } );
                               
        outFile.write(reinterpret_cast<const char*>(mem.get()), mem_size);
        outFile.close();
       
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
}


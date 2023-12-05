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

#include "pch.h"
#include "toml.hpp"
#include <cstdint>
#include <string>
#include <memory>
#include <fstream>
#include <vector>
#include "ParamBinFile.h"
#include <windows.h>

void remove_substring(std::string& str, const std::string& remove)
{
    size_t pos = 0;

    while ((pos = str.find(remove, pos)) != std::string::npos)
    {
        str.erase(pos, remove.length());
    }
}

std::string getCurrentPath()
{
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    return std::string(buffer);
}

TEST(TestCaseName, TestName)
{
	std::string strpath = getCurrentPath();
	remove_substring(strpath, "TOML2Pbf-Test");
	std::string fileName1 = strpath + "example.pbf";

    std::ifstream inFile;
    inFile.open(fileName1, std::ios::binary | std::ios::ate);

    
    if (!inFile)
    {
        EXPECT_TRUE(false);
    }

    std::size_t size = static_cast<std::size_t>(inFile.tellg());
    inFile.seekg(0, std::ios::beg);

    char* buffer = new char[size];
    EXPECT_TRUE(buffer != nullptr);
   
    EXPECT_TRUE( inFile.read(buffer, size) ) ;
    
    PBF::PBFReader pbfReader;
    EXPECT_TRUE(pbfReader.read(buffer));

    delete[] static_cast<char*>(buffer);

    std::optional<PBF::Date> phoenixDate = pbfReader.getParam< PBF::Date>("phoenix_date");
    PBF::Date date = phoenixDate.value();

    EXPECT_EQ(26, date.day);
    EXPECT_EQ(6, date.month);
    EXPECT_EQ(2023, date.year);
        
    
    std::optional<float> fPeakTorque = pbfReader.getParam<float>("Plant.Motors[0].PeakTorque");
    std::optional<double> dPeakTorque = pbfReader.getParam<double>("Plant.Motors[0].PeakTorque");

    EXPECT_NEAR(14.300000, fPeakTorque.value(), 0.000001);
    EXPECT_NEAR(14.300000, fPeakTorque.value(), 0.000001);
      
    EXPECT_TRUE(true);
}
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
#include <algorithm>
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

template<typename T>
bool CheckParam(PBF::PBFReader& pbfReader, const std::string& key, T value)
{
    std::optional<T> val = pbfReader.getParam<T>(key);
    if (val == std::nullopt)
    {
        return false;
    }
    if (typeid(T) == typeid(float))
    {
        float fv = val.value();       
        float diff = abs(abs((float)value) - abs(fv));
        return (diff < (std::numeric_limits<float>::epsilon() * 10.0) );
    }
    return( val.value() == value);
}

bool CheckFloatFromDouble(PBF::PBFReader& pbfReader, const std::string& key, double value)
{
    std::optional<float> val = pbfReader.getParam<float>(key);
    if (val == std::nullopt)
    {
        return false;
    }
    float fv = val.value();
    double diff = abs(abs(value) - abs(fv));
    return (diff < std::numeric_limits<float>::epsilon());
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



    std::optional<std::string>  configVersion = pbfReader.getParam<std::string>("ConfigVersion");
    EXPECT_EQ("1.0", configVersion.value());

    EXPECT_EQ(PBF::DataTypes::Float32, pbfReader.getType("Controller.Motor1.CurrentController.DSide.Kb"));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.DSide.Kb", 1.000000));

    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.DSide.Kb", 1.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.DSide.Kd", 0.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.DSide.Ki", 20000.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.DSide.Kp", 20.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.DSide.UpSat", 300.000000));


    
    EXPECT_EQ(PBF::DataTypes::Float64, pbfReader.getType("Controller.Motor1.CurrentController.IIRFilter.a1"));
    EXPECT_TRUE(CheckParam<double>(pbfReader, "Controller.Motor1.CurrentController.IIRFilter.a1", -0.90483741803595952));
    EXPECT_TRUE(CheckFloatFromDouble(pbfReader,"Controller.Motor1.CurrentController.IIRFilter.a1", -0.90483741803595952));
    
    EXPECT_EQ(PBF::DataTypes::Float64, pbfReader.getType("Controller.Motor1.CurrentController.IIRFilter.b0"));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.IIRFilter.b0", 0.095162581964040482));
    EXPECT_TRUE(CheckFloatFromDouble(pbfReader, "Controller.Motor1.CurrentController.IIRFilter.b0", 0.095162581964040482));


    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.IIRFilter.b1", 0.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.QSide.Kb", 1.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.QSide.Kd", 0.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.QSide.Ki", 20000.000000));    
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.QSide.Kp", 20.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.CurrentController.QSide.UpSat", 300.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.MaxAcceleration", 200.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.MaxJerk", 5000.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.MaxTorque", 12.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.MaxVelocity", 300.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.NegativePositionLimit", -10000.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.PosController.FFAccelGain", 1.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.PosController.FFJerkGain", 0.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.PosController.FFVelocityGain", 1.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.PosController.Kp", 35.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.PositivePositionLimit", -10000.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.VelocityController.IIRFilter.a1", 0.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.VelocityController.IIRFilter.b0", 0.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.VelocityController.IIRFilter.b1" , 0.000000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.VelocityController.Ki" , 0.600000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.VelocityController.Kp" ,3.600000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.VelocityController.PreFilterFrequency" , 3200.000000));
    //EXPECT_TRUE(CheckParam<bool>(pbfReader,"Controller.Motor1.VelocityController.UsePreFilter",  false));



    EXPECT_NEAR(14.300000, fPeakTorque.value(), 0.000001);
    EXPECT_NEAR(14.300000, fPeakTorque.value(), 0.000001);
      
    EXPECT_TRUE(true);
}
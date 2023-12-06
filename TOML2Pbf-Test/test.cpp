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
#include "PBFReader.h"
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
       
        if (!val.has_value())
        {
            return false;
        }
        std::optional<float> f_val = pbfReader.getParam<float>(key);
        if (f_val == std::nullopt)
        {
            return false;
        }
        float fv = f_val.value();
        double aval = abs(static_cast<double>(value));
        double diff = aval - abs(static_cast<double>(fv));
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
 
    std::optional<PBF::DateTime> test_date_time = pbfReader.getParam< PBF::DateTime>("test_date_time");
    PBF::DateTime test_date_time_value = test_date_time.value();
    
    EXPECT_EQ(26, test_date_time_value.date.day);
    EXPECT_EQ(6, test_date_time_value.date.month);
    EXPECT_EQ(2023, test_date_time_value.date.year);

    EXPECT_EQ(8, test_date_time_value.time.hour);
    EXPECT_EQ(30, test_date_time_value.time.minute);
    EXPECT_EQ(0, test_date_time_value.time.second);
    EXPECT_EQ(0, test_date_time_value.time.nanosecond);

    std::optional<PBF::Time> test_time = pbfReader.getParam< PBF::Time>("test_time");
    PBF::Time test_time_value = test_time.value();
    EXPECT_EQ(8, test_time_value.hour);
    EXPECT_EQ(30, test_time_value.minute);
    EXPECT_EQ(0, test_time_value.second);
    EXPECT_EQ(0, test_time_value.nanosecond);
           
    std::optional<float> fPeakTorque = pbfReader.getParam<float>("Plant.Motors[0].PeakTorque");
    std::optional<double> dPeakTorque = pbfReader.getParam<double>("Plant.Motors[0].PeakTorque");
    EXPECT_NEAR(14.300000, fPeakTorque.value(), 0.000001);
    EXPECT_NEAR(14.300000, fPeakTorque.value(), 0.000001);

    std::optional<std::string>  title = pbfReader.getParam<std::string>("title");
    EXPECT_EQ("Configuration Example", title.value());
     
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
    EXPECT_TRUE(CheckParam<double>(pbfReader, "Controller.Motor1.CurrentController.IIRFilter.a1", (double) - 0.90483741803595952));

    EXPECT_TRUE(CheckFloatFromDouble(pbfReader,"Controller.Motor1.CurrentController.IIRFilter.a1", -0.90483741803595952));
    
    EXPECT_EQ(PBF::DataTypes::Float64, pbfReader.getType("Controller.Motor1.CurrentController.IIRFilter.b0"));
    EXPECT_TRUE(CheckParam<double>(pbfReader,"Controller.Motor1.CurrentController.IIRFilter.b0", 0.095162581964040482));
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
    EXPECT_TRUE(CheckParam<double>(pbfReader,"Controller.Motor1.VelocityController.Ki" , 0.600000));
    EXPECT_TRUE(CheckParam<double>(pbfReader,"Controller.Motor1.VelocityController.Kp" ,3.600000));
    EXPECT_TRUE(CheckParam<float>(pbfReader,"Controller.Motor1.VelocityController.PreFilterFrequency" , 3200.000000));
    
    EXPECT_TRUE(CheckParam<bool>(pbfReader,"Controller.Motor1.VelocityController.UsePreFilter",  false));
 
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Plant.Motors[0].Fs",0.01f ));
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Plant.Motors[0].J",0.074f ));
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Plant.Motors[0].Kemf", 0.1327f ));
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Plant.Motors[0].Ktq", 0.83f ));
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Plant.Motors[0].Ld", 0.00784f ));
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Plant.Motors[0].Lq", 0.00784f));
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Plant.Motors[0].PeakTorque", 14.3f ));
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Plant.Motors[0].Rs", 1.1f) );
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Plant.Motors[0].b", 0.00047f));

    EXPECT_TRUE(CheckParam<std::uint32_t>(pbfReader, "Plant.Motors[0].p", 1));
    EXPECT_TRUE(CheckParam<std::uint64_t>(pbfReader, "Plant.Motors[0].p", 1));
    EXPECT_TRUE(CheckParam<std::int32_t>(pbfReader, "Plant.Motors[0].p", true));
   
    EXPECT_TRUE(CheckParam<std::int64_t>(pbfReader, "Plant.Motors[0].p", 1));
    EXPECT_TRUE(CheckParam<uint32_t>(pbfReader, "Plant.Motors[0].index", 0));
         
    EXPECT_TRUE(CheckParam<float>(pbfReader, "PowerSupply.DCBusVoltage", 320.00 ));
    EXPECT_TRUE(CheckParam<std::uint32_t>(pbfReader, "SystemClockFrequency", 1000 ));

    std::optional<std::string>  description = pbfReader.getParam<std::string>("DigitalOutputs[0].description");
    EXPECT_EQ("Main power relay", description.value());

    std::optional<std::string>  name = pbfReader.getParam<std::string>("DigitalOutputs[0].name");
    EXPECT_EQ("Output1", name.value());

    EXPECT_TRUE(CheckParam<std::uint32_t>(pbfReader, "DigitalOutputs[0].port", 1));
    EXPECT_TRUE(CheckParam<std::uint32_t>(pbfReader, "DigitalOutputs[0].state", 1));
 
    /*
    [UART1]
    BaudRate = 115200
        DataBits = 8
        StopBits = 1
        Parity = 0  # 0 = None, 1 = Odd, 2 = Even, 3 = Mark, 4 = Space
        FlowControl = 0  # 0 = None, 1 = RTS / CTS, 2 = XON / XOFF
        ReadTimeout = 1000 # in milliseconds
        WriteTimeout = 1000 # in milliseconds


        UART1.BaudRate	Type UInt32	2485881796
UART1.DataBits	Type UInt32	4062047686
UART1.FlowControl	Type UInt32	2180727143
UART1.Parity	Type UInt32	565918775
UART1.ReadTimeout	Type UInt32	2896442697
UART1.StopBits	Type UInt32	1906130078
UART1.WriteTimeout	Type UInt32	691488796
UART2.BaudRate	Type UInt32	1003050715
UART2.DataBits	Type UInt32	1989429485
UART2.FlowControl	Type UInt32	61592346
UART2.Parity	Type UInt32	3189432376
UART2.ReadTimeout	Type UInt32	515580492
UART2.StopBits	Type UInt32	3369312957
UART2.WriteTimeout	Type UInt32	740059371

        [NetworkSettings]
    UseDHCP = false
        IPAddress = "192.168.1.100"
        SubnetMask = "255.255.255.0"
        Gateway = "192.168.1.1"
        DNS = ["8.8.8.8", "8.8.4.4"]
    
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Testing.test1.NumberOfSteps	
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Testing.test1.Steps[0]	
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Testing.test1.Steps[1]	
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Testing.test1.Steps[2]	
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Testing.test1.Steps[3]	
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Testing.test1.Steps[4]	
    EXPECT_TRUE(CheckParam<float>(pbfReader, "Testing.test1.Steps[5]
    */
}
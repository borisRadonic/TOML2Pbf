# TOML2Pbf

## Overview
**TOML2Pbf** is a PC utility that converts configuration files from the TOML (Tom's Obvious, Minimal Language) format to the Parameters Binary File (PBF) format. This tool is particularly designed for systems where reading text-based configuration files such as XML, JSON, or YAML is resource-intensive and impractical, such as on embedded platforms.

## Status
**Warning**: This project is currently a Work In Progress.

## Key Features
- **Efficient Conversion to Binary**: Converts TOML files to the PBF format, optimizing for memory and processing constraints typical of embedded systems.
- **Embedded System Compatibility**: The binary files produced are intended for use in embedded platforms, ensuring efficient usage of limited resources.
- **PBF Reader (ParamBinCpp)**: A companion utility that enables reading and interpreting the binary configuration files on embedded systems.
- **Support for All TOML Data Types**: Capable of handling and converting all data types defined in TOML, including arrays and tables.
- **Hash-Based Key Management**: In the PBF format, keys are represented as hash values, allowing for fast and efficient data retrieval.
- **Diverse Data Type Support**: TOML2Pbf supports a wide range of data types, including String, Int8, UInt8, Int16, UInt16, Int32, UInt32, Int64, UInt64, Float32, Float64, Boolean, Date, Time, and DateTime.

![](https://github.com/borisRadonic/TOML2Pbf/blob/master/toml2pbf.png)

## Example Usage
For embedded platforms, TOML2Pbf facilitates easy access to configuration parameters from the PBF file. For instance, in C++:

```cpp
 std::optional<double> 		dPeakTorque = pbfReader.getParam<double>("Plant.Motors[0].PeakTorque");
 std::optional<PBF::Date> 	phoenixDate = pbfReader.getParam< PBF::Date>("phoenix_date");
 std::optional<std::string> title 		= pbfReader.getParam<std::string>("ConfigVersion");




// TOML2Pbf.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "toml.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

uint32_t fnv1aHash(std::string_view text)
{
    const std::uint32_t prime = 0x01000193; // 16777619
    std::uint32_t hash = 0x811C9DC5; // 2166136261

    for (char c : text) {
        hash ^= static_cast<std::uint32_t>(c);
        hash *= prime;
    }

    return hash;
}

toml::table parseTOMLFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    return toml::parse(file);
}

struct BinaryKeyValuePair
{
    std::uint32_t hashedKey;
    std::string value;

    BinaryKeyValuePair() : hashedKey(0), value("")
    {
    }
};


void writeBinaryFile(const std::vector<BinaryKeyValuePair>& data, const std::string& filePath)
{
    std::ofstream outFile(filePath, std::ios::binary);

    for (const auto& item : data)
    {
        outFile.write(reinterpret_cast<const char*>(&item.hashedKey), sizeof(item.hashedKey));
        // Write the value - this example assumes string values
        outFile.write(item.value.c_str(), item.value.size());
        // You might need to write a delimiter or length prefix for variable-length data
    }
}
std::vector<BinaryKeyValuePair> serializeToBinary(const toml::table& tomlData)
{
    std::vector<BinaryKeyValuePair> binaryData;

    for (const auto& [key, value] : tomlData)
    {       
        std::uint32_t hk = fnv1aHash(key);
        BinaryKeyValuePair kvp;
        kvp.hashedKey = hk;
        binaryData.push_back(kvp);
    }
    return binaryData;
}

int main()
{
    std::string tomlFilePath = "config.toml";
    std::string binaryFilePath = "config.bin";

    try
    {
        auto tomlData = parseTOMLFile(tomlFilePath);
        auto binaryData = serializeToBinary(tomlData);
        writeBinaryFile(binaryData, binaryFilePath);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}


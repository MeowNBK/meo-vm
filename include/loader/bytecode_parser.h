#pragma once

#include "core/objects.h"

class MemoryManager;

class BytecodeParser {
public:
    std::unordered_map<std::string, Proto> protos;
    BytecodeParser() = default;
    Bool parseFile(const std::string& filepath, MemoryManager& mm);
    Bool parseSource(const std::string& source, const std::string& sourceName = "<string>");
private:
    MemoryManager* memoryManager = nullptr;
    Proto currentProto = nullptr;
    Bool parseLine(const std::string& line, const std::string& sourceName, Int lineNumber);
    Bool parseDirective(const std::vector<std::string>& parts, const std::string& sourceName, Int lineNumber);
    Value parseConstValue(const std::string& token);
    std::vector<std::string> split(const std::string& s);
    void resolveAllLabels();
    void linkProtos();
};
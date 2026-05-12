#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class ScriptAssemblyDescriptor
{
public:
    virtual ~ScriptAssemblyDescriptor() = default;

    std::string AssemblyPath;

    std::string get_AssemblyPath();

    std::string ModuleId;

    std::string get_ModuleId();

    std::string OutputDirectoryPath;

    std::string get_OutputDirectoryPath();

    ScriptAssemblyDescriptor(std::string moduleId, std::string outputDirectoryPath, std::string assemblyPath);
};

#ifdef DrawText
#undef DrawText
#endif
#include "ScriptAssemblyDescriptor.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string ScriptAssemblyDescriptor::get_AssemblyPath()
{
return this->AssemblyPath;
}

std::string ScriptAssemblyDescriptor::get_ModuleId()
{
return this->ModuleId;
}

std::string ScriptAssemblyDescriptor::get_OutputDirectoryPath()
{
return this->OutputDirectoryPath;
}

ScriptAssemblyDescriptor::ScriptAssemblyDescriptor(std::string moduleId, std::string outputDirectoryPath, std::string assemblyPath) : AssemblyPath(), ModuleId(), OutputDirectoryPath()
{
    if (String::IsNullOrWhiteSpace(moduleId))
    {
throw ([&]() {
auto __ctor_arg_0000015A = "Module id must be provided.";
auto __ctor_arg_0000015B = "moduleId";
return new ArgumentException(__ctor_arg_0000015A, __ctor_arg_0000015B);
})();
    }
    if (String::IsNullOrWhiteSpace(outputDirectoryPath))
    {
throw ([&]() {
auto __ctor_arg_0000015C = "Output directory path must be provided.";
auto __ctor_arg_0000015D = "outputDirectoryPath";
return new ArgumentException(__ctor_arg_0000015C, __ctor_arg_0000015D);
})();
    }
    if (String::IsNullOrWhiteSpace(assemblyPath))
    {
throw ([&]() {
auto __ctor_arg_0000015E = "Assembly path must be provided.";
auto __ctor_arg_0000015F = "assemblyPath";
return new ArgumentException(__ctor_arg_0000015E, __ctor_arg_0000015F);
})();
    }
this->ModuleId = moduleId;
this->OutputDirectoryPath = outputDirectoryPath;
this->AssemblyPath = assemblyPath;
}


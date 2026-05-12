#ifdef DrawText
#undef DrawText
#endif
#include "ScriptTypeResolver.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/array.hpp"
#include "runtime/native_type.hpp"
#include "Assembly.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/string_comparer.hpp"

void ScriptTypeResolver::Clear()
{
this->AssembliesByModuleId->Clear();
}

void ScriptTypeResolver::Register(std::string moduleId, Assembly* assembly)
{
    if (String::IsNullOrWhiteSpace(moduleId))
    {
throw ([&]() {
auto __ctor_arg_00000270 = "Module id must be provided.";
auto __ctor_arg_00000271 = "moduleId";
return new ArgumentException(__ctor_arg_00000270, __ctor_arg_00000271);
})();
    }
    if (assembly == nullptr)
    {
throw new ArgumentNullException("assembly");
    }
(*this->AssembliesByModuleId)[moduleId] = assembly;
}

Type* ScriptTypeResolver::Resolve(std::string assemblyQualifiedTypeName)
{
    if (String::IsNullOrWhiteSpace(assemblyQualifiedTypeName))
    {
throw ([&]() {
auto __ctor_arg_00000272 = "Assembly-qualified type name must be provided.";
auto __ctor_arg_00000273 = "assemblyQualifiedTypeName";
return new ArgumentException(__ctor_arg_00000272, __ctor_arg_00000273);
})();
    }
Array<std::string> *parts = String::Split(assemblyQualifiedTypeName, ',', 2, StringSplitOptions::TrimEntries);
    if (parts->Length != 2)
    {
throw new InvalidOperationException(std::string("Type '") + assemblyQualifiedTypeName + std::string("' is not assembly-qualified."));
    }
const std::string typeName = (*parts)[0];
const std::string moduleId = (*parts)[1];
Assembly* assembly;
    if (!this->AssembliesByModuleId->TryGetValue(moduleId, assembly))
    {
throw new InvalidOperationException(std::string("Script assembly '") + moduleId + std::string("' is not loaded for type '") + assemblyQualifiedTypeName + std::string("'."));
    }
Type *resolvedType = assembly->GetType(typeName, false, false);
    if (resolvedType == nullptr)
    {
throw new InvalidOperationException(std::string("Type '") + typeName + std::string("' was not found in loaded script assembly '") + moduleId + std::string("'."));
    }
return resolvedType;}

ScriptTypeResolver::ScriptTypeResolver() : AssembliesByModuleId()
{
this->AssembliesByModuleId = new Dictionary<std::string, Assembly*>(StringComparer::OrdinalIgnoreCase);
}


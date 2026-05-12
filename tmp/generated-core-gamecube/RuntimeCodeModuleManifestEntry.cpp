#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeCodeModuleManifestEntry.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

Array<std::string>* RuntimeCodeModuleManifestEntry::get_DependencyModuleIds()
{
return this->DependencyModuleIds;
}

::RuntimeCodeModuleLoadState RuntimeCodeModuleManifestEntry::get_LoadState()
{
return this->LoadState;
}

std::string RuntimeCodeModuleManifestEntry::get_ModuleId()
{
return this->ModuleId;
}

std::string RuntimeCodeModuleManifestEntry::get_RuntimeSpecializationId()
{
return this->RuntimeSpecializationId;
}

RuntimeCodeModuleManifestEntry::RuntimeCodeModuleManifestEntry(std::string moduleId, std::string runtimeSpecializationId, ::RuntimeCodeModuleLoadState loadState, Array<std::string>* dependencyModuleIds) : DependencyModuleIds(), LoadState(), ModuleId(), RuntimeSpecializationId()
{
    if (String::IsNullOrWhiteSpace(moduleId))
    {
throw ([&]() {
auto __ctor_arg_0000011D = "Runtime code module id is required.";
auto __ctor_arg_0000011E = "moduleId";
return new ArgumentException(__ctor_arg_0000011D, __ctor_arg_0000011E);
})();
    }
    if (String::IsNullOrWhiteSpace(runtimeSpecializationId))
    {
throw ([&]() {
auto __ctor_arg_0000011F = "Runtime code module specialization id is required.";
auto __ctor_arg_00000120 = "runtimeSpecializationId";
return new ArgumentException(__ctor_arg_0000011F, __ctor_arg_00000120);
})();
    }
    if (dependencyModuleIds == nullptr)
    {
throw new ArgumentNullException("dependencyModuleIds");
    }
for (int32_t index = 0; index < dependencyModuleIds->Length; index++) {
const std::string dependencyModuleId = (*dependencyModuleIds)[index];
    if (String::IsNullOrWhiteSpace(dependencyModuleId))
    {
throw ([&]() {
auto __ctor_arg_00000121 = "Runtime code module dependencies cannot contain blank entries.";
auto __ctor_arg_00000122 = "dependencyModuleIds";
return new ArgumentException(__ctor_arg_00000121, __ctor_arg_00000122);
})();
    }
}
this->ModuleId = moduleId;
this->RuntimeSpecializationId = runtimeSpecializationId;
this->LoadState = loadState;
this->DependencyModuleIds = dependencyModuleIds;
}


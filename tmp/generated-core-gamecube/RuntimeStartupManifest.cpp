#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeStartupManifest.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeStartupManifest::get_StartupSceneId()
{
return this->StartupSceneId;
}

::RuntimeStorageProfileId* RuntimeStartupManifest::get_StorageProfileId()
{
return this->StorageProfileId;
}

RuntimeStartupManifest::RuntimeStartupManifest(std::string startupSceneId, ::RuntimeStorageProfileId* storageProfileId) : StartupSceneId(), StorageProfileId()
{
    if (String::IsNullOrWhiteSpace(startupSceneId))
    {
throw ([&]() {
auto __ctor_arg_0000013D = "Startup scene id is required.";
auto __ctor_arg_0000013E = "startupSceneId";
return new ArgumentException(__ctor_arg_0000013D, __ctor_arg_0000013E);
})();
    }
    if (storageProfileId == nullptr)
    {
throw new ArgumentNullException("storageProfileId");
    }
this->StartupSceneId = startupSceneId;
this->StorageProfileId = storageProfileId;
}


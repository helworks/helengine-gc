#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeStorageProfileId;

#include "runtime/native_string.hpp"
#include "RuntimeStorageProfileId.hpp"

class RuntimeStartupManifest
{
public:
    virtual ~RuntimeStartupManifest() = default;

    std::string StartupSceneId;

    std::string get_StartupSceneId();

    ::RuntimeStorageProfileId* StorageProfileId;

    ::RuntimeStorageProfileId* get_StorageProfileId();

    RuntimeStartupManifest(std::string startupSceneId, ::RuntimeStorageProfileId* storageProfileId);
};

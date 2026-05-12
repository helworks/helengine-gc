#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeSceneCatalogEntry;

#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/native_dictionary.hpp"

class RuntimeSceneCatalog
{
public:
    virtual ~RuntimeSceneCatalog() = default;

    Array<::RuntimeSceneCatalogEntry*>* Entries;

    Array<::RuntimeSceneCatalogEntry*>* get_Entries();

    RuntimeSceneCatalog(Array<::RuntimeSceneCatalogEntry*>* entries);

    bool TryGetEntry(std::string sceneId, ::RuntimeSceneCatalogEntry*& entry);
private:
    Dictionary<std::string, ::RuntimeSceneCatalogEntry*>* EntriesBySceneId;
};

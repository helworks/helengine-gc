#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ModelAssetIndexData;
class RuntimeSubmesh;
class ModelAsset;
class ModelSubmeshAsset;

#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "ModelAssetIndexData.hpp"
#include "ModelAssetIndexData.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "RuntimeSubmesh.hpp"
#include "ModelAsset.hpp"
#include "runtime/array.hpp"
#include "ModelSubmeshAsset.hpp"

class ModelSubmeshResolver
{
public:
    virtual ~ModelSubmeshResolver() = default;

    static Array<::RuntimeSubmesh*>* BuildRuntimeSubmeshes(::ModelAsset* asset);

    static Array<::ModelSubmeshAsset*>* ResolveAssetSubmeshes(::ModelAsset* asset);
private:
    static int32_t ResolveElementCount(::ModelAsset* asset);

    static void ValidateSubmeshes(Array<::ModelSubmeshAsset*>* submeshes, int32_t elementCount);
};

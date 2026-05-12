#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;
class float3;
class ModelSubmeshAsset;
class float2;

#include "Asset.hpp"
#include "float3.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "ModelSubmeshAsset.hpp"
#include "runtime/array.hpp"
#include "float2.hpp"

class ModelAsset : public Asset
{
public:
    virtual ~ModelAsset() = default;

    ModelAsset();

    ::float3 BoundsMax;

    ::float3 BoundsMin;

    Array<uint16_t>* Indices16;

    Array<uint32_t>* Indices32;

    Array<::float3>* Normals;

    Array<::float3>* Positions;

    Array<uint8_t>* Ps2PackedMeshBytes;

    Array<::ModelSubmeshAsset*>* Submeshes;

    Array<::float2>* TexCoords;

    std::string get_Id();

    void set_Id(std::string value);
};

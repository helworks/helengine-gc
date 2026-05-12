#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class EngineBinaryWriter;
class EngineBinaryReader;
class SceneAssetReference;

#include "runtime/native_exceptions.hpp"
#include "EngineBinaryWriter.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "SceneAssetReference.hpp"
#include "EngineBinaryReader.hpp"
#include "SceneAssetReference.hpp"
#include "runtime/array.hpp"
#include "EngineBinaryWriter.hpp"

class MeshComponentScenePayloadSerializer
{
public:
    virtual ~MeshComponentScenePayloadSerializer() = default;

    static uint8_t CurrentVersion;

    static void Read(::EngineBinaryReader* reader, ::SceneAssetReference*& modelReference, Array<::SceneAssetReference*>*& materialReferences, uint8_t& renderOrder3D);

    static void Write(::EngineBinaryWriter* writer, ::SceneAssetReference* modelReference, Array<::SceneAssetReference*>* materialReferences, uint8_t renderOrder3D);
private:
    static void ReadCurrentVersion(::EngineBinaryReader* reader, ::SceneAssetReference*& modelReference, Array<::SceneAssetReference*>*& materialReferences, uint8_t& renderOrder3D);

    static ::SceneAssetReference* ReadOptionalReference(::EngineBinaryReader* reader);

    static void WriteOptionalReference(::EngineBinaryWriter* writer, ::SceneAssetReference* reference);
};

#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class EngineBinaryHeader;
class EngineBinaryHeaderSerializer;
class EngineBinaryWriter;
class Asset;
class EngineBinaryReader;
class TextureAsset;
class ModelAsset;
class ModelSubmeshAsset;
class TextAsset;
class MaterialAsset;
class Ps2MaterialAsset;
class AnimationClipAsset;
class PositionKeyframeTrackAsset;
class PositionOffsetKeyframeTrackAsset;
class ScaleKeyframeTrackAsset;
class RotationKeyframeTrackAsset;
class PositionKeyframeAsset;
class RotationKeyframeAsset;
class SceneAsset;
class SceneSettingsAsset;
class SceneCanvasProfile;
class float3;
class float4;
class SceneEntityAsset;
class SceneEntityPlatformTransformOverrideAsset;
class SceneEntityPlatformComponentOverrideAsset;
class SceneEntityPlatformAddedComponentAsset;
class SceneAssetReference;
class SceneComponentAssetRecord;
class MaterialRenderState;
class MaterialConstantBufferAsset;
class ShaderAsset;
class ShaderProgramAsset;
class ShaderBinaryAsset;
class ShaderBindingAsset;
class ShaderConstantMemberAsset;
class ShaderVariantAsset;
class ShaderVertexElementAsset;
class float2;

#include "runtime/native_exceptions.hpp"
#include "EditorAssetBinaryValueKind.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryHeaderSerializer.hpp"
#include "EngineBinaryHeaderSerializer.hpp"
#include "EngineBinaryWriter.hpp"
#include "EngineBinaryWriter.hpp"
#include "Asset.hpp"
#include "EngineBinaryReader.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "EditorAssetBinaryValueKind.hpp"
#include "TextureAsset.hpp"
#include "ModelAsset.hpp"
#include "ModelSubmeshAsset.hpp"
#include "TextAsset.hpp"
#include "MaterialAsset.hpp"
#include "MaterialAsset.hpp"
#include "Ps2MaterialAsset.hpp"
#include "AnimationClipAsset.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "PositionKeyframeAsset.hpp"
#include "RotationKeyframeAsset.hpp"
#include "AnimationInterpolationMode.hpp"
#include "SceneAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "SceneCanvasProfile.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "SceneEntityAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneAssetReference.hpp"
#include "runtime/array.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "MaterialRenderState.hpp"
#include "MaterialConstantBufferAsset.hpp"
#include "ShaderAsset.hpp"
#include "ShaderProgramAsset.hpp"
#include "ShaderBinaryAsset.hpp"
#include "ShaderBindingAsset.hpp"
#include "ShaderConstantMemberAsset.hpp"
#include "ShaderVariantAsset.hpp"
#include "ShaderVertexElementAsset.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "EditorBinaryRecordKind.hpp"
#include "EngineBinaryEndianness.hpp"
#include "Asset.hpp"
#include "system/io/stream.hpp"
#include "AnimationClipAsset.hpp"
#include "AnimationInterpolationMode.hpp"
#include "float2.hpp"
#include "MaterialConstantBufferAsset.hpp"
#include "MaterialRenderState.hpp"
#include "ModelAsset.hpp"
#include "ModelSubmeshAsset.hpp"
#include "PositionKeyframeAsset.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "Ps2MaterialAsset.hpp"
#include "RotationKeyframeAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "SceneAsset.hpp"
#include "SceneAssetReference.hpp"
#include "runtime/array.hpp"
#include "SceneCanvasProfile.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "runtime/array.hpp"
#include "SceneEntityAsset.hpp"
#include "runtime/array.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneSettingsAsset.hpp"
#include "ShaderAsset.hpp"
#include "ShaderBinaryAsset.hpp"
#include "ShaderBindingAsset.hpp"
#include "ShaderConstantMemberAsset.hpp"
#include "ShaderProgramAsset.hpp"
#include "ShaderVariantAsset.hpp"
#include "ShaderVertexElementAsset.hpp"
#include "TextAsset.hpp"
#include "TextureAsset.hpp"

class EditorAssetBinarySerializer
{
public:
    virtual ~EditorAssetBinarySerializer() = default;

    static uint8_t CurrentVersion;

    static uint16_t FormatId;

    static ::EditorBinaryRecordKind RecordKind;

    static ::Asset* Deserialize(::Stream* stream);

    static ::Asset* Deserialize(::Stream* stream, ::EngineBinaryHeader* header);

    static void Serialize(::Stream* stream, ::Asset* asset);
private:
    static ::EngineBinaryEndianness PayloadEndianness;

    static uint8_t SceneEntityPayloadVersion;

    static ::EditorAssetBinaryValueKind GetValueKind(::Asset* asset);

    static ::AnimationClipAsset* ReadAnimationClipAsset(::EngineBinaryReader* reader);

    static ::AnimationInterpolationMode ReadAnimationInterpolationMode(::EngineBinaryReader* reader);

    static ::Asset* ReadAssetPayload(::EngineBinaryReader* reader, ::EditorAssetBinaryValueKind valueKind, uint8_t version);

    static ::float2 ReadFloat2(::EngineBinaryReader* reader);

    static ::float3 ReadFloat3(::EngineBinaryReader* reader);

    static ::float4 ReadFloat4(::EngineBinaryReader* reader);

    static ::MaterialAsset* ReadMaterialAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::MaterialConstantBufferAsset* ReadMaterialConstantBufferAsset(::EngineBinaryReader* reader);

    static ::MaterialRenderState* ReadMaterialRenderState(::EngineBinaryReader* reader);

    static ::ModelAsset* ReadModelAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::ModelSubmeshAsset* ReadModelSubmeshAsset(::EngineBinaryReader* reader);

    static ::PositionKeyframeAsset* ReadPositionKeyframeAsset(::EngineBinaryReader* reader);

    static ::PositionKeyframeTrackAsset* ReadPositionKeyframeTrackAsset(::EngineBinaryReader* reader);

    static ::PositionOffsetKeyframeTrackAsset* ReadPositionOffsetKeyframeTrackAsset(::EngineBinaryReader* reader);

    static ::Ps2MaterialAsset* ReadPs2MaterialAsset(::EngineBinaryReader* reader);

    static ::RotationKeyframeAsset* ReadRotationKeyframeAsset(::EngineBinaryReader* reader);

    static ::RotationKeyframeTrackAsset* ReadRotationKeyframeTrackAsset(::EngineBinaryReader* reader);

    static ::ScaleKeyframeTrackAsset* ReadScaleKeyframeTrackAsset(::EngineBinaryReader* reader);

    static ::SceneAsset* ReadSceneAsset(::EngineBinaryReader* reader, uint8_t version);

    static ::SceneAssetReference* ReadSceneAssetReference(::EngineBinaryReader* reader);

    static Array<::SceneAssetReference*>* ReadSceneAssetReferenceArray(::EngineBinaryReader* reader);

    static ::SceneCanvasProfile* ReadSceneCanvasProfile(::EngineBinaryReader* reader);

    static ::SceneComponentAssetRecord* ReadSceneComponentAssetRecord(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion);

    static Array<::SceneComponentAssetRecord*>* ReadSceneComponentAssetRecordArray(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion);

    static ::SceneEntityAsset* ReadSceneEntityAsset(::EngineBinaryReader* reader, uint8_t version);

    static Array<::SceneEntityAsset*>* ReadSceneEntityAssetArray(::EngineBinaryReader* reader, uint8_t version);

    static ::SceneEntityPlatformAddedComponentAsset* ReadSceneEntityPlatformAddedComponentAsset(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion);

    static ::SceneEntityPlatformAddedComponentAsset* ReadSceneEntityPlatformAddedComponentAssetValue(::EngineBinaryReader* reader);

    static ::SceneEntityPlatformComponentOverrideAsset* ReadSceneEntityPlatformComponentOverrideAsset(::EngineBinaryReader* reader);

    static ::SceneEntityPlatformTransformOverrideAsset* ReadSceneEntityPlatformTransformOverrideAsset(::EngineBinaryReader* reader);

    static ::SceneSettingsAsset* ReadSceneSettingsAsset(::EngineBinaryReader* reader);

    static ::ShaderAsset* ReadShaderAsset(::EngineBinaryReader* reader);

    static ::ShaderBinaryAsset* ReadShaderBinaryAsset(::EngineBinaryReader* reader);

    static ::ShaderBindingAsset* ReadShaderBindingAsset(::EngineBinaryReader* reader);

    static ::ShaderConstantMemberAsset* ReadShaderConstantMemberAsset(::EngineBinaryReader* reader);

    static ::ShaderProgramAsset* ReadShaderProgramAsset(::EngineBinaryReader* reader);

    static ::ShaderVariantAsset* ReadShaderVariantAsset(::EngineBinaryReader* reader);

    static ::ShaderVertexElementAsset* ReadShaderVertexElementAsset(::EngineBinaryReader* reader);

    static std::string ReadStringValue(::EngineBinaryReader* reader);

    static ::TextAsset* ReadTextAsset(::EngineBinaryReader* reader);

    static ::TextureAsset* ReadTextureAsset(::EngineBinaryReader* reader);

    static uint16_t ReadUInt16Value(::EngineBinaryReader* reader);

    static uint32_t ReadUInt32Value(::EngineBinaryReader* reader);

    static void ValidateHeader(::EngineBinaryHeader* header);

    static void WriteAnimationClipAsset(::EngineBinaryWriter* writer, ::AnimationClipAsset* asset);

    static void WriteAnimationInterpolationMode(::EngineBinaryWriter* writer, ::AnimationInterpolationMode value);

    static void WriteAssetPayload(::EngineBinaryWriter* writer, ::Asset* asset);

    static void WriteFloat2(::EngineBinaryWriter* writer, ::float2 value);

    static void WriteFloat3(::EngineBinaryWriter* writer, ::float3 value);

    static void WriteFloat4(::EngineBinaryWriter* writer, ::float4 value);

    static void WriteMaterialAsset(::EngineBinaryWriter* writer, ::MaterialAsset* asset);

    static void WriteMaterialConstantBufferAsset(::EngineBinaryWriter* writer, ::MaterialConstantBufferAsset* asset);

    static void WriteMaterialRenderState(::EngineBinaryWriter* writer, ::MaterialRenderState* renderState);

    static void WriteModelAsset(::EngineBinaryWriter* writer, ::ModelAsset* asset);

    static void WriteModelSubmeshAsset(::EngineBinaryWriter* writer, ::ModelSubmeshAsset* submesh);

    static void WritePositionKeyframeAsset(::EngineBinaryWriter* writer, ::PositionKeyframeAsset* asset);

    static void WritePositionKeyframeTrackAsset(::EngineBinaryWriter* writer, ::PositionKeyframeTrackAsset* asset);

    static void WritePositionOffsetKeyframeTrackAsset(::EngineBinaryWriter* writer, ::PositionOffsetKeyframeTrackAsset* asset);

    static void WritePs2MaterialAsset(::EngineBinaryWriter* writer, ::Ps2MaterialAsset* asset);

    static void WriteRotationKeyframeAsset(::EngineBinaryWriter* writer, ::RotationKeyframeAsset* asset);

    static void WriteRotationKeyframeTrackAsset(::EngineBinaryWriter* writer, ::RotationKeyframeTrackAsset* asset);

    static void WriteScaleKeyframeTrackAsset(::EngineBinaryWriter* writer, ::ScaleKeyframeTrackAsset* asset);

    static void WriteSceneAsset(::EngineBinaryWriter* writer, ::SceneAsset* asset);

    static void WriteSceneAssetReference(::EngineBinaryWriter* writer, ::SceneAssetReference* reference);

    static void WriteSceneCanvasProfile(::EngineBinaryWriter* writer, ::SceneCanvasProfile* canvasProfile);

    static void WriteSceneComponentAssetRecord(::EngineBinaryWriter* writer, ::SceneComponentAssetRecord* record, uint8_t sceneEntityPayloadVersion);

    static void WriteSceneComponentAssetRecordValue(::EngineBinaryWriter* writer, ::SceneComponentAssetRecord* asset);

    static void WriteSceneEntityAsset(::EngineBinaryWriter* writer, ::SceneEntityAsset* asset);

    static void WriteSceneEntityPlatformAddedComponentAsset(::EngineBinaryWriter* writer, ::SceneEntityPlatformAddedComponentAsset* asset);

    static void WriteSceneEntityPlatformComponentOverrideAsset(::EngineBinaryWriter* writer, ::SceneEntityPlatformComponentOverrideAsset* asset);

    static void WriteSceneEntityPlatformTransformOverrideAsset(::EngineBinaryWriter* writer, ::SceneEntityPlatformTransformOverrideAsset* asset);

    static void WriteSceneSettingsAsset(::EngineBinaryWriter* writer, ::SceneSettingsAsset* sceneSettings);

    static void WriteShaderAsset(::EngineBinaryWriter* writer, ::ShaderAsset* asset);

    static void WriteShaderBinaryAsset(::EngineBinaryWriter* writer, ::ShaderBinaryAsset* asset);

    static void WriteShaderBindingAsset(::EngineBinaryWriter* writer, ::ShaderBindingAsset* asset);

    static void WriteShaderConstantMemberAsset(::EngineBinaryWriter* writer, ::ShaderConstantMemberAsset* asset);

    static void WriteShaderProgramAsset(::EngineBinaryWriter* writer, ::ShaderProgramAsset* asset);

    static void WriteShaderVariantAsset(::EngineBinaryWriter* writer, ::ShaderVariantAsset* asset);

    static void WriteShaderVertexElementAsset(::EngineBinaryWriter* writer, ::ShaderVertexElementAsset* asset);

    static void WriteStringValue(::EngineBinaryWriter* writer, std::string value);

    static void WriteTextAsset(::EngineBinaryWriter* writer, ::TextAsset* asset);

    static void WriteTextureAsset(::EngineBinaryWriter* writer, ::TextureAsset* asset);

    static void WriteUInt16Value(::EngineBinaryWriter* writer, uint16_t value);

    static void WriteUInt32Value(::EngineBinaryWriter* writer, uint32_t value);
};

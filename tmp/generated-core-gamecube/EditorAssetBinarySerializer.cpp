#ifdef DrawText
#undef DrawText
#endif
#include "EditorAssetBinarySerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "EditorAssetBinaryValueKind.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryHeaderSerializer.hpp"
#include "EngineBinaryWriter.hpp"
#include "Asset.hpp"
#include "EngineBinaryReader.hpp"
#include "TextureAsset.hpp"
#include "ModelAsset.hpp"
#include "ModelSubmeshAsset.hpp"
#include "TextAsset.hpp"
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
#include "SceneEntityAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneAssetReference.hpp"
#include "SceneComponentAssetRecord.hpp"
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
#include "EditorBinaryRecordKind.hpp"
#include "EngineBinaryEndianness.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

uint8_t EditorAssetBinarySerializer::CurrentVersion = 10;

uint16_t EditorAssetBinarySerializer::FormatId = 1;

::EditorBinaryRecordKind EditorAssetBinarySerializer::RecordKind = EditorBinaryRecordKind::Asset;

::Asset* EditorAssetBinarySerializer::Deserialize(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
::EngineBinaryHeader *header = EngineBinaryHeaderSerializer::Read(stream);
return Deserialize(stream, header);}

::Asset* EditorAssetBinarySerializer::Deserialize(::Stream* stream, ::EngineBinaryHeader* header)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
else     if (header == nullptr)
    {
throw new ArgumentNullException("header");
    }
ValidateHeader(header);
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, header->get_Endianness(), true);
return ReadAssetPayload(reader, static_cast<EditorAssetBinaryValueKind>(header->get_ValueKind()), header->get_Version());}
}

void EditorAssetBinarySerializer::Serialize(::Stream* stream, ::Asset* asset)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
else     if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
::EditorAssetBinaryValueKind valueKind = GetValueKind(asset);
::EngineBinaryHeader *header = new ::EngineBinaryHeader(PayloadEndianness, CurrentVersion, FormatId, static_cast<uint16_t>(RecordKind), static_cast<uint16_t>(valueKind));
EngineBinaryHeaderSerializer::Write(stream, header);
{
::EngineBinaryWriter *writer = EngineBinaryWriter::Create(stream, PayloadEndianness, true);
WriteAssetPayload(writer, asset);
}
}

::EngineBinaryEndianness EditorAssetBinarySerializer::PayloadEndianness = EngineBinaryEndianness::LittleEndian;

uint8_t EditorAssetBinarySerializer::SceneEntityPayloadVersion = 3;

::EditorAssetBinaryValueKind EditorAssetBinarySerializer::GetValueKind(::Asset* asset)
{
    if (he_cpp_try_cast<TextureAsset>(asset) != nullptr)
    {
return EditorAssetBinaryValueKind::TextureAsset;    }
else     if (he_cpp_try_cast<ModelAsset>(asset) != nullptr)
    {
return EditorAssetBinaryValueKind::ModelAsset;    }
else     if (he_cpp_try_cast<ShaderAsset>(asset) != nullptr)
    {
return EditorAssetBinaryValueKind::ShaderAsset;    }
else     if (he_cpp_try_cast<TextAsset>(asset) != nullptr)
    {
return EditorAssetBinaryValueKind::TextAsset;    }
else     if (he_cpp_try_cast<MaterialAsset>(asset) != nullptr)
    {
return EditorAssetBinaryValueKind::MaterialAsset;    }
else     if (he_cpp_try_cast<Ps2MaterialAsset>(asset) != nullptr)
    {
return EditorAssetBinaryValueKind::Ps2MaterialAsset;    }
else     if (he_cpp_try_cast<AnimationClipAsset>(asset) != nullptr)
    {
return EditorAssetBinaryValueKind::AnimationClipAsset;    }
else     if (he_cpp_try_cast<SceneAsset>(asset) != nullptr)
    {
return EditorAssetBinaryValueKind::SceneAsset;    }
throw new InvalidOperationException(std::string("Asset type '") + he_cpp_type_of<Asset>("Asset")->Name + std::string("' is not supported by the editor binary serializer."));
}

::AnimationClipAsset* EditorAssetBinarySerializer::ReadAnimationClipAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_0000003F = new ::AnimationClipAsset();
__object_0000003F->set_Id(reader->ReadString());
__object_0000003F->set_Duration(reader->ReadSingle());
__object_0000003F->set_PositionTracks(([&]() {
Array<::PositionKeyframeTrackAsset*>* __coalesce_00000040 = reader->ReadArray<PositionKeyframeTrackAsset*>(new Func<EngineBinaryReader*, PositionKeyframeTrackAsset*>(&EditorAssetBinarySerializer::ReadPositionKeyframeTrackAsset));
return __coalesce_00000040 != nullptr ? __coalesce_00000040 : Array<PositionKeyframeTrackAsset*>::Empty();
})());
__object_0000003F->set_PositionOffsetTracks(([&]() {
Array<::PositionOffsetKeyframeTrackAsset*>* __coalesce_00000041 = reader->ReadArray<PositionOffsetKeyframeTrackAsset*>(new Func<EngineBinaryReader*, PositionOffsetKeyframeTrackAsset*>(&EditorAssetBinarySerializer::ReadPositionOffsetKeyframeTrackAsset));
return __coalesce_00000041 != nullptr ? __coalesce_00000041 : Array<PositionOffsetKeyframeTrackAsset*>::Empty();
})());
__object_0000003F->set_ScaleTracks(([&]() {
Array<::ScaleKeyframeTrackAsset*>* __coalesce_00000042 = reader->ReadArray<ScaleKeyframeTrackAsset*>(new Func<EngineBinaryReader*, ScaleKeyframeTrackAsset*>(&EditorAssetBinarySerializer::ReadScaleKeyframeTrackAsset));
return __coalesce_00000042 != nullptr ? __coalesce_00000042 : Array<ScaleKeyframeTrackAsset*>::Empty();
})());
__object_0000003F->set_RotationTracks(([&]() {
Array<::RotationKeyframeTrackAsset*>* __coalesce_00000043 = reader->ReadArray<RotationKeyframeTrackAsset*>(new Func<EngineBinaryReader*, RotationKeyframeTrackAsset*>(&EditorAssetBinarySerializer::ReadRotationKeyframeTrackAsset));
return __coalesce_00000043 != nullptr ? __coalesce_00000043 : Array<RotationKeyframeTrackAsset*>::Empty();
})());
return __object_0000003F;
})();}

::AnimationInterpolationMode EditorAssetBinarySerializer::ReadAnimationInterpolationMode(::EngineBinaryReader* reader)
{
return static_cast<AnimationInterpolationMode>(reader->ReadByte());}

::Asset* EditorAssetBinarySerializer::ReadAssetPayload(::EngineBinaryReader* reader, ::EditorAssetBinaryValueKind valueKind, uint8_t version)
{
switch (valueKind) {
case EditorAssetBinaryValueKind::TextureAsset: {
return ReadTextureAsset(reader);}
case EditorAssetBinaryValueKind::ModelAsset: {
return ReadModelAsset(reader, version);}
case EditorAssetBinaryValueKind::ShaderAsset: {
return ReadShaderAsset(reader);}
case EditorAssetBinaryValueKind::TextAsset: {
return ReadTextAsset(reader);}
case EditorAssetBinaryValueKind::MaterialAsset: {
return ReadMaterialAsset(reader, version);}
case EditorAssetBinaryValueKind::Ps2MaterialAsset: {
return ReadPs2MaterialAsset(reader);}
case EditorAssetBinaryValueKind::AnimationClipAsset: {
return ReadAnimationClipAsset(reader);}
case EditorAssetBinaryValueKind::SceneAsset: {
return ReadSceneAsset(reader, version);}
default:  {
throw new InvalidOperationException(std::string("Unsupported asset value kind '") + std::to_string(static_cast<uint16_t>(valueKind)) + std::string("'."));
}
}

}

::float2 EditorAssetBinarySerializer::ReadFloat2(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_00000044 = reader->ReadSingle();
auto __ctor_arg_00000045 = reader->ReadSingle();
return ::float2(__ctor_arg_00000044, __ctor_arg_00000045);
})();}

::float3 EditorAssetBinarySerializer::ReadFloat3(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_00000046 = reader->ReadSingle();
auto __ctor_arg_00000047 = reader->ReadSingle();
auto __ctor_arg_00000048 = reader->ReadSingle();
return ::float3(__ctor_arg_00000046, __ctor_arg_00000047, __ctor_arg_00000048);
})();}

::float4 EditorAssetBinarySerializer::ReadFloat4(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_00000049 = reader->ReadSingle();
auto __ctor_arg_0000004A = reader->ReadSingle();
auto __ctor_arg_0000004B = reader->ReadSingle();
auto __ctor_arg_0000004C = reader->ReadSingle();
return ::float4(__ctor_arg_00000049, __ctor_arg_0000004A, __ctor_arg_0000004B, __ctor_arg_0000004C);
})();}

::MaterialAsset* EditorAssetBinarySerializer::ReadMaterialAsset(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else     if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(version) + std::string("'."));
    }
::MaterialAsset *materialAsset = ([&]() {
auto __object_0000004D = new ::MaterialAsset();
__object_0000004D->set_Id(reader->ReadString());
__object_0000004D->ShaderAssetId = reader->ReadString();
__object_0000004D->VertexProgram = reader->ReadString();
__object_0000004D->PixelProgram = reader->ReadString();
__object_0000004D->Variant = reader->ReadString();
__object_0000004D->DiffuseTextureAssetId = reader->ReadString();
__object_0000004D->CastsShadows = reader->ReadByte() != 0;
__object_0000004D->ReceivesShadows = reader->ReadByte() != 0;
__object_0000004D->RenderState = ReadMaterialRenderState(reader);
__object_0000004D->ConstantBuffers = ([&]() {
Array<::MaterialConstantBufferAsset*>* __coalesce_0000004E = reader->ReadArray<MaterialConstantBufferAsset*>(new Func<EngineBinaryReader*, MaterialConstantBufferAsset*>(&EditorAssetBinarySerializer::ReadMaterialConstantBufferAsset));
return __coalesce_0000004E != nullptr ? __coalesce_0000004E : Array<MaterialConstantBufferAsset*>::Empty();
})();
return __object_0000004D;
})();
return materialAsset;}

::MaterialConstantBufferAsset* EditorAssetBinarySerializer::ReadMaterialConstantBufferAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_0000004F = new ::MaterialConstantBufferAsset();
__object_0000004F->set_Name(reader->ReadString());
__object_0000004F->set_Data(reader->ReadByteArray());
return __object_0000004F;
})();}

::MaterialRenderState* EditorAssetBinarySerializer::ReadMaterialRenderState(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000050 = new ::MaterialRenderState();
__object_00000050->set_BlendMode(static_cast<MaterialBlendMode>(reader->ReadInt32()));
__object_00000050->set_CullMode(static_cast<MaterialCullMode>(reader->ReadInt32()));
__object_00000050->set_DepthTestEnabled(reader->ReadByte() != 0);
__object_00000050->set_DepthWriteEnabled(reader->ReadByte() != 0);
return __object_00000050;
})();}

::ModelAsset* EditorAssetBinarySerializer::ReadModelAsset(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else     if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(version) + std::string("'."));
    }
return ([&]() {
auto __object_00000051 = new ::ModelAsset();
__object_00000051->set_Id(reader->ReadString());
__object_00000051->Positions = reader->ReadArray<float3>(new Func<EngineBinaryReader*, float3>(&EditorAssetBinarySerializer::ReadFloat3));
__object_00000051->Normals = reader->ReadArray<float3>(new Func<EngineBinaryReader*, float3>(&EditorAssetBinarySerializer::ReadFloat3));
__object_00000051->TexCoords = reader->ReadArray<float2>(new Func<EngineBinaryReader*, float2>(&EditorAssetBinarySerializer::ReadFloat2));
__object_00000051->Indices16 = reader->ReadArray<uint16_t>(new Func<EngineBinaryReader*, uint16_t>(&EditorAssetBinarySerializer::ReadUInt16Value));
__object_00000051->Indices32 = reader->ReadArray<uint32_t>(new Func<EngineBinaryReader*, uint32_t>(&EditorAssetBinarySerializer::ReadUInt32Value));
__object_00000051->Submeshes = reader->ReadArray<ModelSubmeshAsset*>(new Func<EngineBinaryReader*, ModelSubmeshAsset*>(&EditorAssetBinarySerializer::ReadModelSubmeshAsset));
__object_00000051->Ps2PackedMeshBytes = reader->ReadByteArray();
return __object_00000051;
})();}

::ModelSubmeshAsset* EditorAssetBinarySerializer::ReadModelSubmeshAsset(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_00000052 = new ::ModelSubmeshAsset();
__object_00000052->set_MaterialSlotName(reader->ReadString());
__object_00000052->set_IndexStart(reader->ReadInt32());
__object_00000052->set_IndexCount(reader->ReadInt32());
return __object_00000052;
})();}

::PositionKeyframeAsset* EditorAssetBinarySerializer::ReadPositionKeyframeAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_00000053 = reader->ReadSingle();
auto __ctor_arg_00000054 = ReadFloat3(reader);
auto __ctor_arg_00000055 = ReadAnimationInterpolationMode(reader);
return new ::PositionKeyframeAsset(__ctor_arg_00000053, __ctor_arg_00000054, __ctor_arg_00000055);
})();}

::PositionKeyframeTrackAsset* EditorAssetBinarySerializer::ReadPositionKeyframeTrackAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000056 = new ::PositionKeyframeTrackAsset();
__object_00000056->set_Keyframes(([&]() {
Array<::PositionKeyframeAsset*>* __coalesce_00000057 = reader->ReadArray<PositionKeyframeAsset*>(new Func<EngineBinaryReader*, PositionKeyframeAsset*>(&EditorAssetBinarySerializer::ReadPositionKeyframeAsset));
return __coalesce_00000057 != nullptr ? __coalesce_00000057 : Array<PositionKeyframeAsset*>::Empty();
})());
return __object_00000056;
})();}

::PositionOffsetKeyframeTrackAsset* EditorAssetBinarySerializer::ReadPositionOffsetKeyframeTrackAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000058 = new ::PositionOffsetKeyframeTrackAsset();
__object_00000058->set_Keyframes(([&]() {
Array<::PositionKeyframeAsset*>* __coalesce_00000059 = reader->ReadArray<PositionKeyframeAsset*>(new Func<EngineBinaryReader*, PositionKeyframeAsset*>(&EditorAssetBinarySerializer::ReadPositionKeyframeAsset));
return __coalesce_00000059 != nullptr ? __coalesce_00000059 : Array<PositionKeyframeAsset*>::Empty();
})());
return __object_00000058;
})();}

::Ps2MaterialAsset* EditorAssetBinarySerializer::ReadPs2MaterialAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_0000005A = new ::Ps2MaterialAsset();
__object_0000005A->set_Id(reader->ReadString());
__object_0000005A->RendererFamilyId = reader->ReadString();
__object_0000005A->LightingMode = static_cast<Ps2MaterialLightingMode>(reader->ReadInt32());
__object_0000005A->AlphaMode = static_cast<Ps2MaterialAlphaMode>(reader->ReadInt32());
__object_0000005A->RenderClass = static_cast<Ps2RenderClass>(reader->ReadInt32());
__object_0000005A->BaseColorR = reader->ReadByte();
__object_0000005A->BaseColorG = reader->ReadByte();
__object_0000005A->BaseColorB = reader->ReadByte();
__object_0000005A->BaseColorA = reader->ReadByte();
__object_0000005A->TextureRelativePath = reader->ReadString();
__object_0000005A->DoubleSided = reader->ReadByte() != 0;
__object_0000005A->CastShadows = reader->ReadByte() != 0;
__object_0000005A->UseVertexColor = reader->ReadByte() != 0;
__object_0000005A->ExpensiveModeAllowed = reader->ReadByte() != 0;
__object_0000005A->Roughness = reader->ReadSingle();
__object_0000005A->SpecularStrength = reader->ReadSingle();
__object_0000005A->EmissiveStrength = reader->ReadSingle();
return __object_0000005A;
})();}

::RotationKeyframeAsset* EditorAssetBinarySerializer::ReadRotationKeyframeAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_0000005B = reader->ReadSingle();
auto __ctor_arg_0000005C = ReadFloat4(reader);
auto __ctor_arg_0000005D = ReadAnimationInterpolationMode(reader);
return new ::RotationKeyframeAsset(__ctor_arg_0000005B, __ctor_arg_0000005C, __ctor_arg_0000005D);
})();}

::RotationKeyframeTrackAsset* EditorAssetBinarySerializer::ReadRotationKeyframeTrackAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_0000005E = new ::RotationKeyframeTrackAsset();
__object_0000005E->set_Keyframes(([&]() {
Array<::RotationKeyframeAsset*>* __coalesce_0000005F = reader->ReadArray<RotationKeyframeAsset*>(new Func<EngineBinaryReader*, RotationKeyframeAsset*>(&EditorAssetBinarySerializer::ReadRotationKeyframeAsset));
return __coalesce_0000005F != nullptr ? __coalesce_0000005F : Array<RotationKeyframeAsset*>::Empty();
})());
return __object_0000005E;
})();}

::ScaleKeyframeTrackAsset* EditorAssetBinarySerializer::ReadScaleKeyframeTrackAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000060 = new ::ScaleKeyframeTrackAsset();
__object_00000060->set_Keyframes(([&]() {
Array<::PositionKeyframeAsset*>* __coalesce_00000061 = reader->ReadArray<PositionKeyframeAsset*>(new Func<EngineBinaryReader*, PositionKeyframeAsset*>(&EditorAssetBinarySerializer::ReadPositionKeyframeAsset));
return __coalesce_00000061 != nullptr ? __coalesce_00000061 : Array<PositionKeyframeAsset*>::Empty();
})());
return __object_00000060;
})();}

::SceneAsset* EditorAssetBinarySerializer::ReadSceneAsset(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else     if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(version) + std::string("'."));
    }
return ([&]() {
auto __object_00000062 = new ::SceneAsset();
__object_00000062->set_Id(reader->ReadString());
__object_00000062->set_RootEntities(([&]() {
Array<::SceneEntityAsset*>* __coalesce_00000063 = ReadSceneEntityAssetArray(reader, version);
return __coalesce_00000063 != nullptr ? __coalesce_00000063 : Array<SceneEntityAsset*>::Empty();
})());
__object_00000062->set_AssetReferences(([&]() {
Array<::SceneAssetReference*>* __coalesce_00000064 = ReadSceneAssetReferenceArray(reader);
return __coalesce_00000064 != nullptr ? __coalesce_00000064 : Array<SceneAssetReference*>::Empty();
})());
__object_00000062->set_Physics3DSceneFeatureFlags(reader->ReadUInt32());
__object_00000062->set_SceneSettings(ReadSceneSettingsAsset(reader));
return __object_00000062;
})();}

::SceneAssetReference* EditorAssetBinarySerializer::ReadSceneAssetReference(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000065 = new ::SceneAssetReference();
__object_00000065->set_SourceKind(static_cast<SceneAssetReferenceSourceKind>(reader->ReadInt32()));
__object_00000065->set_RelativePath(reader->ReadString());
__object_00000065->set_ProviderId(reader->ReadString());
__object_00000065->set_AssetId(reader->ReadString());
return __object_00000065;
})();}

Array<::SceneAssetReference*>* EditorAssetBinarySerializer::ReadSceneAssetReferenceArray(::EngineBinaryReader* reader)
{
return reader->ReadArray<SceneAssetReference*>(new Func<EngineBinaryReader*, SceneAssetReference*>(&EditorAssetBinarySerializer::ReadSceneAssetReference));}

::SceneCanvasProfile* EditorAssetBinarySerializer::ReadSceneCanvasProfile(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000066 = new ::SceneCanvasProfile();
__object_00000066->set_Width(reader->ReadInt32());
__object_00000066->set_Height(reader->ReadInt32());
return __object_00000066;
})();}

::SceneComponentAssetRecord* EditorAssetBinarySerializer::ReadSceneComponentAssetRecord(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_00000067 = new ::SceneComponentAssetRecord();
__object_00000067->set_ComponentKey(sceneEntityPayloadVersion >= 3 ? reader->ReadString() : String::Empty);
__object_00000067->set_ComponentTypeId(reader->ReadString());
__object_00000067->set_ComponentIndex(reader->ReadInt32());
__object_00000067->set_Payload(([&]() {
Array<uint8_t>* __coalesce_00000068 = reader->ReadByteArray();
return __coalesce_00000068 != nullptr ? __coalesce_00000068 : Array<uint8_t>::Empty();
})());
return __object_00000067;
})();}

Array<::SceneComponentAssetRecord*>* EditorAssetBinarySerializer::ReadSceneComponentAssetRecordArray(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion)
{
const int32_t length = reader->ReadInt32();
    if (length == -1)
    {
return nullptr;    }
else     if (length < -1)
    {
throw new InvalidOperationException("Array length cannot be negative.");
    }
else     if (length == 0)
    {
return Array<SceneComponentAssetRecord*>::Empty();    }
Array<::SceneComponentAssetRecord*> *values = new Array<SceneComponentAssetRecord*>(length);
for (int32_t index = 0; index < values->Length; index++) {
(*values)[index] = ReadSceneComponentAssetRecord(reader, sceneEntityPayloadVersion);
}
return values;}

::SceneEntityAsset* EditorAssetBinarySerializer::ReadSceneEntityAsset(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else     if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(version) + std::string("'."));
    }
const uint8_t payloadVersion = reader->ReadByte();
    if (payloadVersion != SceneEntityPayloadVersion)
    {
throw new InvalidOperationException(std::string("Unsupported scene entity payload version '") + std::to_string(payloadVersion) + std::string("'."));
    }
const std::string id = reader->ReadString();
const std::string name = reader->ReadString();
::float3 localPosition = reader->ReadFloat3();
::float3 localScale = reader->ReadFloat3();
::float4 localOrientation = reader->ReadFloat4();
Array<::SceneComponentAssetRecord*> *components = ([&]() {
Array<::SceneComponentAssetRecord*>* __coalesce_00000069 = ReadSceneComponentAssetRecordArray(reader, payloadVersion);
return __coalesce_00000069 != nullptr ? __coalesce_00000069 : Array<SceneComponentAssetRecord*>::Empty();
})();
Array<::SceneEntityPlatformTransformOverrideAsset*> *platformTransformOverrides = ([&]() {
Array<::SceneEntityPlatformTransformOverrideAsset*>* __coalesce_0000006A = reader->ReadArray<SceneEntityPlatformTransformOverrideAsset*>(new Func<EngineBinaryReader*, SceneEntityPlatformTransformOverrideAsset*>(&EditorAssetBinarySerializer::ReadSceneEntityPlatformTransformOverrideAsset));
return __coalesce_0000006A != nullptr ? __coalesce_0000006A : Array<SceneEntityPlatformTransformOverrideAsset*>::Empty();
})();
Array<::SceneEntityPlatformComponentOverrideAsset*> *platformComponentOverrides = ([&]() {
Array<::SceneEntityPlatformComponentOverrideAsset*>* __coalesce_0000006B = reader->ReadArray<SceneEntityPlatformComponentOverrideAsset*>(new Func<EngineBinaryReader*, SceneEntityPlatformComponentOverrideAsset*>(&EditorAssetBinarySerializer::ReadSceneEntityPlatformComponentOverrideAsset));
return __coalesce_0000006B != nullptr ? __coalesce_0000006B : Array<SceneEntityPlatformComponentOverrideAsset*>::Empty();
})();
return ([&]() {
auto __object_0000006C = new ::SceneEntityAsset();
__object_0000006C->set_Id(id);
__object_0000006C->set_Name(name);
__object_0000006C->set_LocalPosition(localPosition);
__object_0000006C->set_LocalScale(localScale);
__object_0000006C->set_LocalOrientation(localOrientation);
__object_0000006C->set_Components(components);
__object_0000006C->set_PlatformTransformOverrides(platformTransformOverrides);
__object_0000006C->set_PlatformComponentOverrides(platformComponentOverrides);
__object_0000006C->set_Children(([&]() {
Array<::SceneEntityAsset*>* __coalesce_0000006D = ReadSceneEntityAssetArray(reader, version);
return __coalesce_0000006D != nullptr ? __coalesce_0000006D : Array<SceneEntityAsset*>::Empty();
})());
return __object_0000006C;
})();}

Array<::SceneEntityAsset*>* EditorAssetBinarySerializer::ReadSceneEntityAssetArray(::EngineBinaryReader* reader, uint8_t version)
{
const int32_t length = reader->ReadInt32();
    if (length == -1)
    {
return nullptr;    }
else     if (length < -1)
    {
throw new InvalidOperationException("Array length cannot be negative.");
    }
else     if (length == 0)
    {
return Array<SceneEntityAsset*>::Empty();    }
Array<::SceneEntityAsset*> *values = new Array<SceneEntityAsset*>(length);
for (int32_t index = 0; index < values->Length; index++) {
(*values)[index] = ReadSceneEntityAsset(reader, version);
}
return values;}

::SceneEntityPlatformAddedComponentAsset* EditorAssetBinarySerializer::ReadSceneEntityPlatformAddedComponentAsset(::EngineBinaryReader* reader, uint8_t sceneEntityPayloadVersion)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_0000006E = new ::SceneEntityPlatformAddedComponentAsset();
__object_0000006E->set_Component(ReadSceneComponentAssetRecord(reader, sceneEntityPayloadVersion));
return __object_0000006E;
})();}

::SceneEntityPlatformAddedComponentAsset* EditorAssetBinarySerializer::ReadSceneEntityPlatformAddedComponentAssetValue(::EngineBinaryReader* reader)
{
return ReadSceneEntityPlatformAddedComponentAsset(reader, SceneEntityPayloadVersion);}

::SceneEntityPlatformComponentOverrideAsset* EditorAssetBinarySerializer::ReadSceneEntityPlatformComponentOverrideAsset(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_0000006F = new ::SceneEntityPlatformComponentOverrideAsset();
__object_0000006F->set_PlatformId(reader->ReadString());
__object_0000006F->set_RemovedComponentKeys(([&]() {
Array<std::string>* __coalesce_00000070 = reader->ReadArray<std::string>(new Func<EngineBinaryReader*, std::string>(&EditorAssetBinarySerializer::ReadStringValue));
return __coalesce_00000070 != nullptr ? __coalesce_00000070 : Array<std::string>::Empty();
})());
__object_0000006F->set_AddedComponents(([&]() {
Array<::SceneEntityPlatformAddedComponentAsset*>* __coalesce_00000071 = reader->ReadArray<SceneEntityPlatformAddedComponentAsset*>(new Func<EngineBinaryReader*, SceneEntityPlatformAddedComponentAsset*>(&EditorAssetBinarySerializer::ReadSceneEntityPlatformAddedComponentAssetValue));
return __coalesce_00000071 != nullptr ? __coalesce_00000071 : Array<SceneEntityPlatformAddedComponentAsset*>::Empty();
})());
return __object_0000006F;
})();}

::SceneEntityPlatformTransformOverrideAsset* EditorAssetBinarySerializer::ReadSceneEntityPlatformTransformOverrideAsset(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_00000072 = new ::SceneEntityPlatformTransformOverrideAsset();
__object_00000072->set_PlatformId(reader->ReadString());
__object_00000072->set_HasLocalPositionOverride(reader->ReadByte() != 0);
__object_00000072->set_LocalPosition(reader->ReadFloat3());
__object_00000072->set_HasLocalScaleOverride(reader->ReadByte() != 0);
__object_00000072->set_LocalScale(reader->ReadFloat3());
__object_00000072->set_HasLocalOrientationOverride(reader->ReadByte() != 0);
__object_00000072->set_LocalOrientation(reader->ReadFloat4());
return __object_00000072;
})();}

::SceneSettingsAsset* EditorAssetBinarySerializer::ReadSceneSettingsAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000073 = new ::SceneSettingsAsset();
__object_00000073->set_CanvasProfile(ReadSceneCanvasProfile(reader));
return __object_00000073;
})();}

::ShaderAsset* EditorAssetBinarySerializer::ReadShaderAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000074 = new ::ShaderAsset();
__object_00000074->set_Id(reader->ReadString());
__object_00000074->Name = reader->ReadString();
__object_00000074->TargetName = reader->ReadString();
__object_00000074->Programs = reader->ReadArray<ShaderProgramAsset*>(new Func<EngineBinaryReader*, ShaderProgramAsset*>(&EditorAssetBinarySerializer::ReadShaderProgramAsset));
__object_00000074->Binaries = reader->ReadArray<ShaderBinaryAsset*>(new Func<EngineBinaryReader*, ShaderBinaryAsset*>(&EditorAssetBinarySerializer::ReadShaderBinaryAsset));
return __object_00000074;
})();}

::ShaderBinaryAsset* EditorAssetBinarySerializer::ReadShaderBinaryAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000075 = new ::ShaderBinaryAsset();
__object_00000075->ProgramName = reader->ReadString();
__object_00000075->Stage = static_cast<ShaderStage>(reader->ReadInt32());
__object_00000075->TargetName = reader->ReadString();
__object_00000075->Variant = reader->ReadString();
__object_00000075->Bytecode = reader->ReadByteArray();
return __object_00000075;
})();}

::ShaderBindingAsset* EditorAssetBinarySerializer::ReadShaderBindingAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000076 = new ::ShaderBindingAsset();
__object_00000076->Name = reader->ReadString();
__object_00000076->Type = static_cast<ShaderResourceType>(reader->ReadInt32());
__object_00000076->Set = reader->ReadInt32();
__object_00000076->Slot = reader->ReadInt32();
__object_00000076->Size = reader->ReadInt32();
__object_00000076->Members = reader->ReadArray<ShaderConstantMemberAsset*>(new Func<EngineBinaryReader*, ShaderConstantMemberAsset*>(&EditorAssetBinarySerializer::ReadShaderConstantMemberAsset));
return __object_00000076;
})();}

::ShaderConstantMemberAsset* EditorAssetBinarySerializer::ReadShaderConstantMemberAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000077 = new ::ShaderConstantMemberAsset();
__object_00000077->Name = reader->ReadString();
__object_00000077->Type = reader->ReadString();
__object_00000077->Offset = reader->ReadInt32();
__object_00000077->Size = reader->ReadInt32();
return __object_00000077;
})();}

::ShaderProgramAsset* EditorAssetBinarySerializer::ReadShaderProgramAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000078 = new ::ShaderProgramAsset();
__object_00000078->Name = reader->ReadString();
__object_00000078->Stage = static_cast<ShaderStage>(reader->ReadInt32());
__object_00000078->EntryPoint = reader->ReadString();
__object_00000078->Bindings = reader->ReadArray<ShaderBindingAsset*>(new Func<EngineBinaryReader*, ShaderBindingAsset*>(&EditorAssetBinarySerializer::ReadShaderBindingAsset));
__object_00000078->Inputs = reader->ReadArray<ShaderVertexElementAsset*>(new Func<EngineBinaryReader*, ShaderVertexElementAsset*>(&EditorAssetBinarySerializer::ReadShaderVertexElementAsset));
__object_00000078->Outputs = reader->ReadArray<ShaderVertexElementAsset*>(new Func<EngineBinaryReader*, ShaderVertexElementAsset*>(&EditorAssetBinarySerializer::ReadShaderVertexElementAsset));
__object_00000078->Variants = reader->ReadArray<ShaderVariantAsset*>(new Func<EngineBinaryReader*, ShaderVariantAsset*>(&EditorAssetBinarySerializer::ReadShaderVariantAsset));
return __object_00000078;
})();}

::ShaderVariantAsset* EditorAssetBinarySerializer::ReadShaderVariantAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_00000079 = new ::ShaderVariantAsset();
__object_00000079->Name = reader->ReadString();
__object_00000079->Defines = reader->ReadArray<std::string>(new Func<EngineBinaryReader*, std::string>(&EditorAssetBinarySerializer::ReadStringValue));
return __object_00000079;
})();}

::ShaderVertexElementAsset* EditorAssetBinarySerializer::ReadShaderVertexElementAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_0000007A = new ::ShaderVertexElementAsset();
__object_0000007A->Semantic = reader->ReadString();
__object_0000007A->Index = reader->ReadInt32();
__object_0000007A->Format = reader->ReadString();
return __object_0000007A;
})();}

std::string EditorAssetBinarySerializer::ReadStringValue(::EngineBinaryReader* reader)
{
return reader->ReadString();}

::TextAsset* EditorAssetBinarySerializer::ReadTextAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_0000007B = new ::TextAsset();
__object_0000007B->set_Id(reader->ReadString());
__object_0000007B->Text = reader->ReadString();
return __object_0000007B;
})();}

::TextureAsset* EditorAssetBinarySerializer::ReadTextureAsset(::EngineBinaryReader* reader)
{
return ([&]() {
auto __object_0000007C = new ::TextureAsset();
__object_0000007C->set_Id(reader->ReadString());
__object_0000007C->Width = reader->ReadUInt16();
__object_0000007C->Height = reader->ReadUInt16();
__object_0000007C->Colors = reader->ReadByteArray();
return __object_0000007C;
})();}

uint16_t EditorAssetBinarySerializer::ReadUInt16Value(::EngineBinaryReader* reader)
{
return reader->ReadUInt16();}

uint32_t EditorAssetBinarySerializer::ReadUInt32Value(::EngineBinaryReader* reader)
{
return reader->ReadUInt32();}

void EditorAssetBinarySerializer::ValidateHeader(::EngineBinaryHeader* header)
{
    if (header->get_FormatId() != FormatId)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary format id '") + std::to_string(header->get_FormatId()) + std::string("'."));
    }
else     if (header->get_RecordKind() != static_cast<uint16_t>(RecordKind))
    {
throw new InvalidOperationException(std::string("Unexpected asset record kind '") + std::to_string(header->get_RecordKind()) + std::string("'."));
    }
else     if (header->get_Version() != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported asset binary version '") + std::to_string(header->get_Version()) + std::string("'."));
    }
}

void EditorAssetBinarySerializer::WriteAnimationClipAsset(::EngineBinaryWriter* writer, ::AnimationClipAsset* asset)
{
writer->WriteString(asset->Id);
writer->WriteSingle(asset->get_Duration());
writer->WriteArray<PositionKeyframeTrackAsset*>(asset->get_PositionTracks(), new Action<EngineBinaryWriter*, PositionKeyframeTrackAsset*>(&EditorAssetBinarySerializer::WritePositionKeyframeTrackAsset));
writer->WriteArray<PositionOffsetKeyframeTrackAsset*>(asset->get_PositionOffsetTracks(), new Action<EngineBinaryWriter*, PositionOffsetKeyframeTrackAsset*>(&EditorAssetBinarySerializer::WritePositionOffsetKeyframeTrackAsset));
writer->WriteArray<ScaleKeyframeTrackAsset*>(asset->get_ScaleTracks(), new Action<EngineBinaryWriter*, ScaleKeyframeTrackAsset*>(&EditorAssetBinarySerializer::WriteScaleKeyframeTrackAsset));
writer->WriteArray<RotationKeyframeTrackAsset*>(asset->get_RotationTracks(), new Action<EngineBinaryWriter*, RotationKeyframeTrackAsset*>(&EditorAssetBinarySerializer::WriteRotationKeyframeTrackAsset));
}

void EditorAssetBinarySerializer::WriteAnimationInterpolationMode(::EngineBinaryWriter* writer, ::AnimationInterpolationMode value)
{
writer->WriteByte(static_cast<uint8_t>(value));
}

void EditorAssetBinarySerializer::WriteAssetPayload(::EngineBinaryWriter* writer, ::Asset* asset)
{
    TextureAsset* textureAsset = he_cpp_try_cast<TextureAsset>(asset);
    if (textureAsset != nullptr)
    {
WriteTextureAsset(writer, textureAsset);
return;    }
else {
    ModelAsset* modelAsset = he_cpp_try_cast<ModelAsset>(asset);
    if (modelAsset != nullptr)
    {
WriteModelAsset(writer, modelAsset);
return;    }
else {
    ShaderAsset* shaderAsset = he_cpp_try_cast<ShaderAsset>(asset);
    if (shaderAsset != nullptr)
    {
WriteShaderAsset(writer, shaderAsset);
return;    }
else {
    TextAsset* textAsset = he_cpp_try_cast<TextAsset>(asset);
    if (textAsset != nullptr)
    {
WriteTextAsset(writer, textAsset);
return;    }
else {
    MaterialAsset* materialAsset = he_cpp_try_cast<MaterialAsset>(asset);
    if (materialAsset != nullptr)
    {
WriteMaterialAsset(writer, materialAsset);
return;    }
else {
    Ps2MaterialAsset* ps2MaterialAsset = he_cpp_try_cast<Ps2MaterialAsset>(asset);
    if (ps2MaterialAsset != nullptr)
    {
WritePs2MaterialAsset(writer, ps2MaterialAsset);
return;    }
else {
    AnimationClipAsset* animationClipAsset = he_cpp_try_cast<AnimationClipAsset>(asset);
    if (animationClipAsset != nullptr)
    {
WriteAnimationClipAsset(writer, animationClipAsset);
return;    }
else {
    SceneAsset* sceneAsset = he_cpp_try_cast<SceneAsset>(asset);
    if (sceneAsset != nullptr)
    {
WriteSceneAsset(writer, sceneAsset);
return;    }
}
}
}
}
}
}
}
throw new InvalidOperationException(std::string("Asset type '") + he_cpp_type_of<Asset>("Asset")->Name + std::string("' is not supported by the editor binary serializer."));
}

void EditorAssetBinarySerializer::WriteFloat2(::EngineBinaryWriter* writer, ::float2 value)
{
writer->WriteSingle(value.X);
writer->WriteSingle(value.Y);
}

void EditorAssetBinarySerializer::WriteFloat3(::EngineBinaryWriter* writer, ::float3 value)
{
writer->WriteSingle(value.X);
writer->WriteSingle(value.Y);
writer->WriteSingle(value.Z);
}

void EditorAssetBinarySerializer::WriteFloat4(::EngineBinaryWriter* writer, ::float4 value)
{
writer->WriteSingle(value.X);
writer->WriteSingle(value.Y);
writer->WriteSingle(value.Z);
writer->WriteSingle(value.W);
}

void EditorAssetBinarySerializer::WriteMaterialAsset(::EngineBinaryWriter* writer, ::MaterialAsset* asset)
{
writer->WriteString(asset->Id);
writer->WriteString(asset->ShaderAssetId);
writer->WriteString(asset->VertexProgram);
writer->WriteString(asset->PixelProgram);
writer->WriteString(asset->Variant);
writer->WriteString(asset->DiffuseTextureAssetId);
writer->WriteByte(asset->CastsShadows ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteByte(asset->ReceivesShadows ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
WriteMaterialRenderState(writer, asset->RenderState);
writer->WriteArray<MaterialConstantBufferAsset*>(asset->ConstantBuffers, new Action<EngineBinaryWriter*, MaterialConstantBufferAsset*>(&EditorAssetBinarySerializer::WriteMaterialConstantBufferAsset));
}

void EditorAssetBinarySerializer::WriteMaterialConstantBufferAsset(::EngineBinaryWriter* writer, ::MaterialConstantBufferAsset* asset)
{
    if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
else     if (String::IsNullOrWhiteSpace(asset->get_Name()))
    {
throw new InvalidOperationException("Material constant-buffer assets must define a binding name.");
    }
else     if (asset->get_Data() == nullptr)
    {
throw new InvalidOperationException("Material constant-buffer assets must define a byte payload.");
    }
writer->WriteString(asset->get_Name());
writer->WriteByteArray(asset->get_Data());
}

void EditorAssetBinarySerializer::WriteMaterialRenderState(::EngineBinaryWriter* writer, ::MaterialRenderState* renderState)
{
    if (renderState == nullptr)
    {
throw new ArgumentNullException("renderState");
    }
writer->WriteInt32(static_cast<int32_t>(renderState->get_BlendMode()));
writer->WriteInt32(static_cast<int32_t>(renderState->get_CullMode()));
writer->WriteByte(renderState->get_DepthTestEnabled() ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteByte(renderState->get_DepthWriteEnabled() ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
}

void EditorAssetBinarySerializer::WriteModelAsset(::EngineBinaryWriter* writer, ::ModelAsset* asset)
{
writer->WriteString(asset->Id);
writer->WriteArray<float3>(asset->Positions, new Action<EngineBinaryWriter*, float3>(&EditorAssetBinarySerializer::WriteFloat3));
writer->WriteArray<float3>(asset->Normals, new Action<EngineBinaryWriter*, float3>(&EditorAssetBinarySerializer::WriteFloat3));
writer->WriteArray<float2>(asset->TexCoords, new Action<EngineBinaryWriter*, float2>(&EditorAssetBinarySerializer::WriteFloat2));
writer->WriteArray<uint16_t>(asset->Indices16, new Action<EngineBinaryWriter*, uint16_t>(&EditorAssetBinarySerializer::WriteUInt16Value));
writer->WriteArray<uint32_t>(asset->Indices32, new Action<EngineBinaryWriter*, uint32_t>(&EditorAssetBinarySerializer::WriteUInt32Value));
writer->WriteArray<ModelSubmeshAsset*>(asset->Submeshes, new Action<EngineBinaryWriter*, ModelSubmeshAsset*>(&EditorAssetBinarySerializer::WriteModelSubmeshAsset));
writer->WriteByteArray(asset->Ps2PackedMeshBytes);
}

void EditorAssetBinarySerializer::WriteModelSubmeshAsset(::EngineBinaryWriter* writer, ::ModelSubmeshAsset* submesh)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (submesh == nullptr)
    {
throw new ArgumentNullException("submesh");
    }
writer->WriteString(submesh->get_MaterialSlotName());
writer->WriteInt32(submesh->get_IndexStart());
writer->WriteInt32(submesh->get_IndexCount());
}

void EditorAssetBinarySerializer::WritePositionKeyframeAsset(::EngineBinaryWriter* writer, ::PositionKeyframeAsset* asset)
{
writer->WriteSingle(asset->get_Time());
WriteFloat3(writer, asset->get_Value());
WriteAnimationInterpolationMode(writer, asset->get_InterpolationMode());
}

void EditorAssetBinarySerializer::WritePositionKeyframeTrackAsset(::EngineBinaryWriter* writer, ::PositionKeyframeTrackAsset* asset)
{
writer->WriteArray<PositionKeyframeAsset*>(asset->get_Keyframes(), new Action<EngineBinaryWriter*, PositionKeyframeAsset*>(&EditorAssetBinarySerializer::WritePositionKeyframeAsset));
}

void EditorAssetBinarySerializer::WritePositionOffsetKeyframeTrackAsset(::EngineBinaryWriter* writer, ::PositionOffsetKeyframeTrackAsset* asset)
{
writer->WriteArray<PositionKeyframeAsset*>(asset->get_Keyframes(), new Action<EngineBinaryWriter*, PositionKeyframeAsset*>(&EditorAssetBinarySerializer::WritePositionKeyframeAsset));
}

void EditorAssetBinarySerializer::WritePs2MaterialAsset(::EngineBinaryWriter* writer, ::Ps2MaterialAsset* asset)
{
writer->WriteString(asset->Id);
writer->WriteString(asset->RendererFamilyId);
writer->WriteInt32(static_cast<int32_t>(asset->LightingMode));
writer->WriteInt32(static_cast<int32_t>(asset->AlphaMode));
writer->WriteInt32(static_cast<int32_t>(asset->RenderClass));
writer->WriteByte(asset->BaseColorR);
writer->WriteByte(asset->BaseColorG);
writer->WriteByte(asset->BaseColorB);
writer->WriteByte(asset->BaseColorA);
writer->WriteString(asset->TextureRelativePath);
writer->WriteByte(asset->DoubleSided ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteByte(asset->CastShadows ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteByte(asset->UseVertexColor ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteByte(asset->ExpensiveModeAllowed ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteSingle(asset->Roughness);
writer->WriteSingle(asset->SpecularStrength);
writer->WriteSingle(asset->EmissiveStrength);
}

void EditorAssetBinarySerializer::WriteRotationKeyframeAsset(::EngineBinaryWriter* writer, ::RotationKeyframeAsset* asset)
{
writer->WriteSingle(asset->get_Time());
WriteFloat4(writer, asset->get_Value());
WriteAnimationInterpolationMode(writer, asset->get_InterpolationMode());
}

void EditorAssetBinarySerializer::WriteRotationKeyframeTrackAsset(::EngineBinaryWriter* writer, ::RotationKeyframeTrackAsset* asset)
{
writer->WriteArray<RotationKeyframeAsset*>(asset->get_Keyframes(), new Action<EngineBinaryWriter*, RotationKeyframeAsset*>(&EditorAssetBinarySerializer::WriteRotationKeyframeAsset));
}

void EditorAssetBinarySerializer::WriteScaleKeyframeTrackAsset(::EngineBinaryWriter* writer, ::ScaleKeyframeTrackAsset* asset)
{
writer->WriteArray<PositionKeyframeAsset*>(asset->get_Keyframes(), new Action<EngineBinaryWriter*, PositionKeyframeAsset*>(&EditorAssetBinarySerializer::WritePositionKeyframeAsset));
}

void EditorAssetBinarySerializer::WriteSceneAsset(::EngineBinaryWriter* writer, ::SceneAsset* asset)
{
writer->WriteString(asset->Id);
writer->WriteArray<SceneEntityAsset*>(asset->get_RootEntities(), new Action<EngineBinaryWriter*, SceneEntityAsset*>(&EditorAssetBinarySerializer::WriteSceneEntityAsset));
writer->WriteArray<SceneAssetReference*>(asset->get_AssetReferences(), new Action<EngineBinaryWriter*, SceneAssetReference*>(&EditorAssetBinarySerializer::WriteSceneAssetReference));
writer->WriteUInt32(asset->get_Physics3DSceneFeatureFlags());
WriteSceneSettingsAsset(writer, asset->get_SceneSettings());
}

void EditorAssetBinarySerializer::WriteSceneAssetReference(::EngineBinaryWriter* writer, ::SceneAssetReference* reference)
{
writer->WriteInt32(static_cast<int32_t>(reference->get_SourceKind()));
writer->WriteString(reference->get_RelativePath());
writer->WriteString(reference->get_ProviderId());
writer->WriteString(reference->get_AssetId());
}

void EditorAssetBinarySerializer::WriteSceneCanvasProfile(::EngineBinaryWriter* writer, ::SceneCanvasProfile* canvasProfile)
{
writer->WriteInt32(canvasProfile->get_Width());
writer->WriteInt32(canvasProfile->get_Height());
}

void EditorAssetBinarySerializer::WriteSceneComponentAssetRecord(::EngineBinaryWriter* writer, ::SceneComponentAssetRecord* record, uint8_t sceneEntityPayloadVersion)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
    if (sceneEntityPayloadVersion >= 3)
    {
writer->WriteString(record->get_ComponentKey());
    }
writer->WriteString(record->get_ComponentTypeId());
writer->WriteInt32(record->get_ComponentIndex());
writer->WriteByteArray(record->get_Payload());
}

void EditorAssetBinarySerializer::WriteSceneComponentAssetRecordValue(::EngineBinaryWriter* writer, ::SceneComponentAssetRecord* asset)
{
WriteSceneComponentAssetRecord(writer, asset, SceneEntityPayloadVersion);
}

void EditorAssetBinarySerializer::WriteSceneEntityAsset(::EngineBinaryWriter* writer, ::SceneEntityAsset* asset)
{
writer->WriteByte(SceneEntityPayloadVersion);
writer->WriteString(asset->get_Id());
writer->WriteString(asset->get_Name());
writer->WriteFloat3(asset->get_LocalPosition());
writer->WriteFloat3(asset->get_LocalScale());
writer->WriteFloat4(asset->get_LocalOrientation());
writer->WriteArray<SceneComponentAssetRecord*>(asset->get_Components(), new Action<EngineBinaryWriter*, SceneComponentAssetRecord*>(&EditorAssetBinarySerializer::WriteSceneComponentAssetRecordValue));
writer->WriteArray<SceneEntityPlatformTransformOverrideAsset*>(asset->get_PlatformTransformOverrides(), new Action<EngineBinaryWriter*, SceneEntityPlatformTransformOverrideAsset*>(&EditorAssetBinarySerializer::WriteSceneEntityPlatformTransformOverrideAsset));
writer->WriteArray<SceneEntityPlatformComponentOverrideAsset*>(asset->get_PlatformComponentOverrides(), new Action<EngineBinaryWriter*, SceneEntityPlatformComponentOverrideAsset*>(&EditorAssetBinarySerializer::WriteSceneEntityPlatformComponentOverrideAsset));
writer->WriteArray<SceneEntityAsset*>(asset->get_Children(), new Action<EngineBinaryWriter*, SceneEntityAsset*>(&EditorAssetBinarySerializer::WriteSceneEntityAsset));
}

void EditorAssetBinarySerializer::WriteSceneEntityPlatformAddedComponentAsset(::EngineBinaryWriter* writer, ::SceneEntityPlatformAddedComponentAsset* asset)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
else     if (asset->get_Component() == nullptr)
    {
throw new InvalidOperationException("Platform-added component assets must define a serialized component record.");
    }
WriteSceneComponentAssetRecord(writer, asset->get_Component(), SceneEntityPayloadVersion);
}

void EditorAssetBinarySerializer::WriteSceneEntityPlatformComponentOverrideAsset(::EngineBinaryWriter* writer, ::SceneEntityPlatformComponentOverrideAsset* asset)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
writer->WriteString(asset->get_PlatformId());
writer->WriteArray<std::string>(asset->get_RemovedComponentKeys(), new Action<EngineBinaryWriter*, std::string>(&EditorAssetBinarySerializer::WriteStringValue));
writer->WriteArray<SceneEntityPlatformAddedComponentAsset*>(asset->get_AddedComponents(), new Action<EngineBinaryWriter*, SceneEntityPlatformAddedComponentAsset*>(&EditorAssetBinarySerializer::WriteSceneEntityPlatformAddedComponentAsset));
}

void EditorAssetBinarySerializer::WriteSceneEntityPlatformTransformOverrideAsset(::EngineBinaryWriter* writer, ::SceneEntityPlatformTransformOverrideAsset* asset)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
writer->WriteString(asset->get_PlatformId());
writer->WriteByte(asset->get_HasLocalPositionOverride() ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteFloat3(asset->get_LocalPosition());
writer->WriteByte(asset->get_HasLocalScaleOverride() ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteFloat3(asset->get_LocalScale());
writer->WriteByte(asset->get_HasLocalOrientationOverride() ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteFloat4(asset->get_LocalOrientation());
}

void EditorAssetBinarySerializer::WriteSceneSettingsAsset(::EngineBinaryWriter* writer, ::SceneSettingsAsset* sceneSettings)
{
WriteSceneCanvasProfile(writer, sceneSettings->get_CanvasProfile());
}

void EditorAssetBinarySerializer::WriteShaderAsset(::EngineBinaryWriter* writer, ::ShaderAsset* asset)
{
writer->WriteString(asset->Id);
writer->WriteString(asset->Name);
writer->WriteString(asset->TargetName);
writer->WriteArray<ShaderProgramAsset*>(asset->Programs, new Action<EngineBinaryWriter*, ShaderProgramAsset*>(&EditorAssetBinarySerializer::WriteShaderProgramAsset));
writer->WriteArray<ShaderBinaryAsset*>(asset->Binaries, new Action<EngineBinaryWriter*, ShaderBinaryAsset*>(&EditorAssetBinarySerializer::WriteShaderBinaryAsset));
}

void EditorAssetBinarySerializer::WriteShaderBinaryAsset(::EngineBinaryWriter* writer, ::ShaderBinaryAsset* asset)
{
writer->WriteString(asset->ProgramName);
writer->WriteInt32(static_cast<int32_t>(asset->Stage));
writer->WriteString(asset->TargetName);
writer->WriteString(asset->Variant);
writer->WriteByteArray(asset->Bytecode);
}

void EditorAssetBinarySerializer::WriteShaderBindingAsset(::EngineBinaryWriter* writer, ::ShaderBindingAsset* asset)
{
writer->WriteString(asset->Name);
writer->WriteInt32(static_cast<int32_t>(asset->Type));
writer->WriteInt32(asset->Set);
writer->WriteInt32(asset->Slot);
writer->WriteInt32(asset->Size);
writer->WriteArray<ShaderConstantMemberAsset*>(asset->Members, new Action<EngineBinaryWriter*, ShaderConstantMemberAsset*>(&EditorAssetBinarySerializer::WriteShaderConstantMemberAsset));
}

void EditorAssetBinarySerializer::WriteShaderConstantMemberAsset(::EngineBinaryWriter* writer, ::ShaderConstantMemberAsset* asset)
{
writer->WriteString(asset->Name);
writer->WriteString(asset->Type);
writer->WriteInt32(asset->Offset);
writer->WriteInt32(asset->Size);
}

void EditorAssetBinarySerializer::WriteShaderProgramAsset(::EngineBinaryWriter* writer, ::ShaderProgramAsset* asset)
{
writer->WriteString(asset->Name);
writer->WriteInt32(static_cast<int32_t>(asset->Stage));
writer->WriteString(asset->EntryPoint);
writer->WriteArray<ShaderBindingAsset*>(asset->Bindings, new Action<EngineBinaryWriter*, ShaderBindingAsset*>(&EditorAssetBinarySerializer::WriteShaderBindingAsset));
writer->WriteArray<ShaderVertexElementAsset*>(asset->Inputs, new Action<EngineBinaryWriter*, ShaderVertexElementAsset*>(&EditorAssetBinarySerializer::WriteShaderVertexElementAsset));
writer->WriteArray<ShaderVertexElementAsset*>(asset->Outputs, new Action<EngineBinaryWriter*, ShaderVertexElementAsset*>(&EditorAssetBinarySerializer::WriteShaderVertexElementAsset));
writer->WriteArray<ShaderVariantAsset*>(asset->Variants, new Action<EngineBinaryWriter*, ShaderVariantAsset*>(&EditorAssetBinarySerializer::WriteShaderVariantAsset));
}

void EditorAssetBinarySerializer::WriteShaderVariantAsset(::EngineBinaryWriter* writer, ::ShaderVariantAsset* asset)
{
writer->WriteString(asset->Name);
writer->WriteArray<std::string>(asset->Defines, new Action<EngineBinaryWriter*, std::string>(&EditorAssetBinarySerializer::WriteStringValue));
}

void EditorAssetBinarySerializer::WriteShaderVertexElementAsset(::EngineBinaryWriter* writer, ::ShaderVertexElementAsset* asset)
{
writer->WriteString(asset->Semantic);
writer->WriteInt32(asset->Index);
writer->WriteString(asset->Format);
}

void EditorAssetBinarySerializer::WriteStringValue(::EngineBinaryWriter* writer, std::string value)
{
writer->WriteString(value);
}

void EditorAssetBinarySerializer::WriteTextAsset(::EngineBinaryWriter* writer, ::TextAsset* asset)
{
writer->WriteString(asset->Id);
writer->WriteString(asset->Text);
}

void EditorAssetBinarySerializer::WriteTextureAsset(::EngineBinaryWriter* writer, ::TextureAsset* asset)
{
writer->WriteString(asset->Id);
writer->WriteUInt16(asset->Width);
writer->WriteUInt16(asset->Height);
writer->WriteByteArray(asset->Colors);
}

void EditorAssetBinarySerializer::WriteUInt16Value(::EngineBinaryWriter* writer, uint16_t value)
{
writer->WriteUInt16(value);
}

void EditorAssetBinarySerializer::WriteUInt32Value(::EngineBinaryWriter* writer, uint32_t value)
{
writer->WriteUInt32(value);
}


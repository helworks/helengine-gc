#ifdef DrawText
#undef DrawText
#endif
#include "MeshComponentScenePayloadSerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryWriter.hpp"
#include "EngineBinaryReader.hpp"
#include "SceneAssetReference.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

uint8_t MeshComponentScenePayloadSerializer::CurrentVersion = 2;

void MeshComponentScenePayloadSerializer::Read(::EngineBinaryReader* reader, ::SceneAssetReference*& modelReference, Array<::SceneAssetReference*>*& materialReferences, uint8_t& renderOrder3D)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
const uint8_t version = reader->ReadByte();
    if (version == CurrentVersion)
    {
ReadCurrentVersion(reader, modelReference, materialReferences, renderOrder3D);
return;    }
throw new InvalidOperationException(std::string("Unsupported mesh component payload version '") + std::to_string(version) + std::string("'."));
}

void MeshComponentScenePayloadSerializer::Write(::EngineBinaryWriter* writer, ::SceneAssetReference* modelReference, Array<::SceneAssetReference*>* materialReferences, uint8_t renderOrder3D)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (materialReferences == nullptr)
    {
throw new ArgumentNullException("materialReferences");
    }
writer->WriteByte(CurrentVersion);
WriteOptionalReference(writer, modelReference);
writer->WriteInt32(materialReferences->Length);
for (int32_t materialIndex = 0; materialIndex < materialReferences->Length; materialIndex++) {
WriteOptionalReference(writer, (*materialReferences)[materialIndex]);
}
writer->WriteByte(renderOrder3D);
}

void MeshComponentScenePayloadSerializer::ReadCurrentVersion(::EngineBinaryReader* reader, ::SceneAssetReference*& modelReference, Array<::SceneAssetReference*>*& materialReferences, uint8_t& renderOrder3D)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
modelReference = ReadOptionalReference(reader);
const int32_t materialReferenceCount = reader->ReadInt32();
    if (materialReferenceCount < 0)
    {
throw new InvalidOperationException("Mesh component payload material reference count must be non-negative.");
    }
materialReferences = new Array<SceneAssetReference*>(materialReferenceCount);
for (int32_t materialIndex = 0; materialIndex < materialReferenceCount; materialIndex++) {
(*materialReferences)[materialIndex] = ReadOptionalReference(reader);
}
renderOrder3D = reader->ReadByte();
}

::SceneAssetReference* MeshComponentScenePayloadSerializer::ReadOptionalReference(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
    if (reader->ReadByte() == 0)
    {
return nullptr;    }
return ([&]() {
auto __object_000000F8 = new ::SceneAssetReference();
__object_000000F8->set_SourceKind(static_cast<SceneAssetReferenceSourceKind>(reader->ReadInt32()));
__object_000000F8->set_RelativePath(reader->ReadString());
__object_000000F8->set_ProviderId(reader->ReadString());
__object_000000F8->set_AssetId(reader->ReadString());
return __object_000000F8;
})();}

void MeshComponentScenePayloadSerializer::WriteOptionalReference(::EngineBinaryWriter* writer, ::SceneAssetReference* reference)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteByte(reference == nullptr ? static_cast<uint8_t>(0) : static_cast<uint8_t>(1));
    if (reference == nullptr)
    {
return;    }
writer->WriteInt32(static_cast<int32_t>(reference->get_SourceKind()));
writer->WriteString(reference->get_RelativePath());
writer->WriteString(reference->get_ProviderId());
writer->WriteString(reference->get_AssetId());
}


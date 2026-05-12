#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeMeshComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "MeshComponentScenePayloadSerializer.hpp"
#include "MeshComponent.hpp"
#include "Component.hpp"
#include "runtime/array.hpp"
#include "EngineBinaryEndianness.hpp"
#include "SceneAssetReference.hpp"
#include "RuntimeMaterial.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeMeshComponentDeserializer::get_ComponentTypeId()
{
return ComponentType;
}

::Component* RuntimeMeshComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
    if (referenceResolver == nullptr)
    {
throw new ArgumentNullException("referenceResolver");
    }
    if (!String::Equals(record->get_ComponentTypeId(), ComponentType, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("Mesh component deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000255 = ([&]() {
Array<uint8_t>* __coalesce_00000256 = record->get_Payload();
return __coalesce_00000256 != nullptr ? __coalesce_00000256 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000257 = false;
return new ::MemoryStream(__ctor_arg_00000255, __ctor_arg_00000257);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
::SceneAssetReference* modelReference;
Array<::SceneAssetReference*>* materialReferences;
uint8_t renderOrder3D;
MeshComponentScenePayloadSerializer::Read(reader, modelReference, materialReferences, renderOrder3D);
::MeshComponent *meshComponent = ([&]() {
auto __object_00000258 = new ::MeshComponent();
__object_00000258->set_RenderOrder3D(renderOrder3D);
return __object_00000258;
})();
    if (modelReference != nullptr)
    {
meshComponent->set_Model(referenceResolver->ResolveModel(modelReference));
    }
meshComponent->SetMaterials(this->ResolveMaterials(materialReferences, referenceResolver));
return meshComponent;}
}
}

std::string RuntimeMeshComponentDeserializer::ComponentType = "helengine.MeshComponent";

uint8_t RuntimeMeshComponentDeserializer::CurrentVersion = MeshComponentScenePayloadSerializer::CurrentVersion;

Array<::RuntimeMaterial*>* RuntimeMeshComponentDeserializer::ResolveMaterials(Array<::SceneAssetReference*>* references, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (references == nullptr)
    {
throw new ArgumentNullException("references");
    }
else     if (referenceResolver == nullptr)
    {
throw new ArgumentNullException("referenceResolver");
    }
Array<::RuntimeMaterial*> *runtimeMaterials = new Array<RuntimeMaterial*>(references->Length);
for (int32_t materialIndex = 0; materialIndex < references->Length; materialIndex++) {
    if ((*references)[materialIndex] != nullptr)
    {
(*runtimeMaterials)[materialIndex] = referenceResolver->ResolveMaterial((*references)[materialIndex]);
    }
}
return runtimeMaterials;}


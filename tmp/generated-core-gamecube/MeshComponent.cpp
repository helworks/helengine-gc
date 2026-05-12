#ifdef DrawText
#undef DrawText
#endif
#include "MeshComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "RuntimeMaterial.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

::RuntimeMaterial* MeshComponent::get_Material()
{
return this->MaterialsBySlot->Length == 0 ? nullptr : (*this->MaterialsBySlot)[0];}

void MeshComponent::set_Material(::RuntimeMaterial* value)
{
    if (this->MaterialsBySlot->Length == 0)
    {
this->MaterialsBySlot = value == nullptr ? Array<RuntimeMaterial*>::Empty() : new Array<RuntimeMaterial*>({ value });
return;    }
Array<::RuntimeMaterial*> *updatedMaterials = new Array<RuntimeMaterial*>(this->MaterialsBySlot->Length);
Array<RuntimeMaterial*>::Copy(this->MaterialsBySlot, updatedMaterials, this->MaterialsBySlot->Length);
(*updatedMaterials)[0] = value;
this->MaterialsBySlot = updatedMaterials;
}

Array<::RuntimeMaterial*>* MeshComponent::get_Materials()
{
return this->MaterialsBySlot;
}

::RuntimeModel* MeshComponent::get_Model()
{
return this->Model;
}

void MeshComponent::set_Model(::RuntimeModel* value)
{
this->Model = value;
}

uint8_t MeshComponent::get_RenderOrder3D()
{
return this->renderOrder3D;}

void MeshComponent::set_RenderOrder3D(uint8_t value)
{
    if (this->renderOrder3D != value)
    {
    if (Parent != nullptr && Parent->get_IsHierarchyEnabled())
    {
Core::get_Instance()->get_ObjectManager()->RemoveFromRender3D(this);
this->renderOrder3D = value;
Core::get_Instance()->get_ObjectManager()->RegisterForRender3D(this);
    }
else {
this->renderOrder3D = value;
}
    }
}

void MeshComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
Core::get_Instance()->get_ObjectManager()->RegisterForRender3D(this);
    }
}

MeshComponent::MeshComponent() : Model(), MaterialsBySlot(), renderOrder3D()
{
this->MaterialsBySlot = Array<RuntimeMaterial*>::Empty();
}

void MeshComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
Core::get_Instance()->get_ObjectManager()->RegisterForRender3D(this);
    }
else {
Core::get_Instance()->get_ObjectManager()->RemoveFromRender3D(this);
}
}

void MeshComponent::SetMaterials(Array<::RuntimeMaterial*>* runtimeMaterials)
{
    if (runtimeMaterials == nullptr)
    {
throw new ArgumentNullException("runtimeMaterials");
    }
this->MaterialsBySlot = new Array<RuntimeMaterial*>(runtimeMaterials->Length);
Array<RuntimeMaterial*>::Copy(runtimeMaterials, this->MaterialsBySlot, runtimeMaterials->Length);
}

::Entity* MeshComponent::get_Parent()
{
return this->Component::get_Parent();
}

void MeshComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}


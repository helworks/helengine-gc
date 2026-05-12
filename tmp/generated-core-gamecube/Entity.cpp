#ifdef DrawText
#undef DrawText
#endif
#include "Entity.hpp"
#include "runtime/native_exceptions.hpp"
#include "Entity.hpp"
#include "runtime/native_list.hpp"
#include "Component.hpp"
#include "ComponentExecutionPolicy.hpp"
#include "Core.hpp"
#include "ObjectManager.hpp"
#include "float4.hpp"
#include "float3.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

List<::Entity*>* Entity::get_Children()
{
return this->Children;
}

void Entity::set_Children(List<::Entity*>* value)
{
this->Children = value;
}

List<::Component*>* Entity::get_Components()
{
return this->Components;
}

void Entity::set_Components(List<::Component*>* value)
{
this->Components = value;
}

bool Entity::get_Enabled()
{
return this->isEnabled;}

void Entity::set_Enabled(bool value)
{
const bool wasHierarchyEnabled = this->get_IsHierarchyEnabled();
    if (this->isEnabled != value)
    {
this->isEnabled = value;
const bool isHierarchyEnabled = this->get_IsHierarchyEnabled();
    if (wasHierarchyEnabled != isHierarchyEnabled)
    {
this->ParentEnabledChange(isHierarchyEnabled);
    }
    }
}

bool Entity::get_IsHierarchyEnabled()
{
    if (!this->isEnabled)
    {
return false;    }
    if (this->Parent == nullptr)
    {
return true;    }
return this->Parent->get_IsHierarchyEnabled();}

uint16_t Entity::get_LayerMask()
{
return this->LayerMask;
}

void Entity::set_LayerMask(uint16_t value)
{
this->LayerMask = value;
}

::float4 Entity::get_LocalOrientation()
{
return this->orientation;}

void Entity::set_LocalOrientation(::float4 value)
{
this->orientation = value;
}

::float3 Entity::get_LocalPosition()
{
return this->position;}

void Entity::set_LocalPosition(::float3 value)
{
this->position = value;
}

::float3 Entity::get_LocalScale()
{
return this->scale;}

void Entity::set_LocalScale(::float3 value)
{
this->scale = value;
}

::float4 Entity::get_Orientation()
{
::float4 ori = this->orientation;
    if (this->Parent != nullptr)
    {
ori = ori * this->Parent->get_Orientation();
    }
return ori;}

void Entity::set_Orientation(::float4 value)
{
this->orientation = value;
}

::Entity* Entity::get_Parent()
{
return this->Parent;
}

void Entity::set_Parent(::Entity* value)
{
this->Parent = value;
}

::float3 Entity::get_Position()
{
::float3 pos = this->position;
    if (this->Parent != nullptr)
    {
::float3 rotatedLocal = float4::RotateVector(pos, this->Parent->get_Orientation());
pos = rotatedLocal + this->Parent->get_Position();
    }
return pos;}

void Entity::set_Position(::float3 value)
{
this->position = value;
}

::float3 Entity::get_Scale()
{
::float3 sca = this->scale;
    if (this->Parent != nullptr)
    {
sca = sca + this->Parent->get_Scale();
    }
return sca;}

void Entity::set_Scale(::float3 value)
{
this->scale = value;
}

bool Entity::get_Static()
{
return this->isStatic;}

void Entity::set_Static(bool value)
{
    if (this->isStatic != value)
    {
this->ParentStaticChange(value);
    }
this->isStatic = value;
}

bool Entity::get_SuppressUpdateComponentExecutionInEditor()
{
return this->SuppressUpdateComponentExecutionInEditor;
}

void Entity::set_SuppressUpdateComponentExecutionInEditor(bool value)
{
this->SuppressUpdateComponentExecutionInEditor = value;
}

void Entity::AddChild(::Entity* entity)
{
    if (entity->get_Parent() != nullptr)
    {
throw new Exception("Parent is not empty");
    }
const bool wasHierarchyEnabled = entity->get_IsHierarchyEnabled();
entity->set_Parent(this);
this->Children->Add(entity);
const bool isHierarchyEnabled = entity->get_IsHierarchyEnabled();
    if (wasHierarchyEnabled != isHierarchyEnabled)
    {
entity->ParentEnabledChange(isHierarchyEnabled);
    }
}

void Entity::AddComponent(::Component* comp)
{
this->Components->Add(comp);
comp->AttachToEntity(this);
    if (ComponentExecutionPolicy::ShouldRunComponentLifecycle(comp, this))
    {
comp->ComponentAdded(this);
    }
}

void Entity::Dispose()
{
    if (this->Children != nullptr)
    {
while (this->Children->Count() > 0) {
::Entity *child = (*this->Children)[this->Children->Count() - 1];
this->RemoveChild(child);
child->Dispose();
}
    }
    if (this->Components != nullptr)
    {
while (this->Components->Count() > 0) {
this->RemoveComponent((*this->Components)[this->Components->Count() - 1]);
}
    }
    if (this->Parent != nullptr)
    {
this->Parent->RemoveChild(this);
    }
Core::get_Instance()->get_ObjectManager()->RemoveEntity(this);
}

Entity::Entity() : Children(), Components(), LayerMask(), Parent(), SuppressUpdateComponentExecutionInEditor(), isEnabled(), isStatic(), orientation(), position(), scale()
{
this->isEnabled = true;
this->set_Orientation(float4::get_Identity());
this->set_Scale(float3::get_One());
this->set_LayerMask(0b00000001);
Core::get_Instance()->get_ObjectManager()->RegisterEntity(this);
}

void Entity::InitChildren()
{
this->set_Children(new List<::Entity*>());
}

void Entity::InitComponents()
{
this->set_Components(new List<::Component*>());
}

void Entity::RemoveChild(::Entity* entity)
{
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
else     if (this->Children == nullptr)
    {
throw new InvalidOperationException("Children collection has not been initialized.");
    }
else     if (entity->get_Parent() != this)
    {
throw new InvalidOperationException("Entity is not parented to this parent.");
    }
const bool wasHierarchyEnabled = entity->get_IsHierarchyEnabled();
    if (!this->Children->Remove(entity))
    {
throw new InvalidOperationException("Entity could not be removed from the child collection.");
    }
entity->set_Parent(nullptr);
const bool isHierarchyEnabled = entity->get_IsHierarchyEnabled();
    if (wasHierarchyEnabled != isHierarchyEnabled)
    {
entity->ParentEnabledChange(isHierarchyEnabled);
    }
}

void Entity::RemoveComponent(::Component* comp)
{
    if (comp == nullptr)
    {
throw new ArgumentNullException("comp");
    }
else     if (this->Components == nullptr)
    {
throw new InvalidOperationException("Components collection has not been initialized.");
    }
else     if (comp->get_Parent() != this)
    {
throw new InvalidOperationException("Component is not attached to this entity.");
    }
    if (!this->Components->Remove(comp))
    {
throw new InvalidOperationException("Component could not be removed from the component collection.");
    }
const bool shouldRunLifecycle = ComponentExecutionPolicy::ShouldRunComponentLifecycle(comp, this);
    if (this->get_IsHierarchyEnabled() && shouldRunLifecycle)
    {
comp->ParentEnabledChange(false);
    }
    if (shouldRunLifecycle)
    {
comp->ComponentRemoved(this);
    }
comp->DetachFromEntity();
}

void Entity::ParentEnabledChange(bool newEnabled)
{
    if (this->Components != nullptr)
    {
for (int32_t i = 0; i < this->Components->Count(); i++) {
::Component *component = (*this->Components)[i];
    if (!ComponentExecutionPolicy::ShouldRunComponentLifecycle(component, this))
    {
continue;
    }
component->ParentEnabledChange(newEnabled);
}
    }
    if (this->Children != nullptr)
    {
for (int32_t i = 0; i < this->Children->Count(); i++) {
(*this->Children)[i]->ParentEnabledChange((*this->Children)[i]->get_IsHierarchyEnabled());
}
    }
}

void Entity::ParentStaticChange(bool newEnabled)
{
    if (this->Components != nullptr)
    {
for (int32_t i = 0; i < this->Components->Count(); i++) {
(*this->Components)[i]->ParentStaticChange(newEnabled);
}
    }
    if (this->Children != nullptr)
    {
for (int32_t i = 0; i < this->Children->Count(); i++) {
(*this->Children)[i]->ParentStaticChange(newEnabled);
}
    }
}


#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;

class Component
{
public:
    virtual ~Component() = default;

    Component();

    ::Entity* Parent;

    ::Entity* get_Parent();
    void set_Parent(::Entity* value);

    void AttachToEntity(::Entity* entity);

    virtual void ComponentAdded(::Entity* entity);

    virtual void ComponentRemoved(::Entity* entity);

    void DetachFromEntity();

    virtual void ParentEnabledChange(bool newEnabled);

    virtual void ParentStaticChange(bool newEnabled);
};

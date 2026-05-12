#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "Attribute.hpp"

class ScenePersistenceIgnoreAttribute : public Attribute
{
public:
    virtual ~ScenePersistenceIgnoreAttribute() = default;
};

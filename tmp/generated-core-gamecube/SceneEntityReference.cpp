#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityReference.hpp"

SceneEntityReference::SceneEntityReference() : EntityId()
{
}

std::string SceneEntityReference::get_EntityId()
{
return this->EntityId;
}

void SceneEntityReference::set_EntityId(std::string value)
{
this->EntityId = value;
}


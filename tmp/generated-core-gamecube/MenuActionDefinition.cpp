#ifdef DrawText
#undef DrawText
#endif
#include "MenuActionDefinition.hpp"

::MenuActionKind MenuActionDefinition::get_Kind()
{
return this->Kind;
}

std::string MenuActionDefinition::get_TargetId()
{
return this->TargetId;
}

MenuActionDefinition::MenuActionDefinition(::MenuActionKind kind, std::string targetId) : Kind(), TargetId()
{
this->Kind = kind;
this->TargetId = targetId;
}


#ifdef DrawText
#undef DrawText
#endif
#include "MenuItemRuntime.hpp"
#include "runtime/native_exceptions.hpp"

::RoundedRectComponent* MenuItemRuntime::get_Background()
{
return this->Background;
}

::MenuItemComponent* MenuItemRuntime::get_Definition()
{
return this->Definition;
}

::Entity* MenuItemRuntime::get_Entity()
{
return this->Entity;
}

int32_t MenuItemRuntime::get_Index()
{
return this->Index;
}

MenuItemRuntime::MenuItemRuntime(::MenuItemComponent* definition, int32_t index, ::Entity* entity, ::RoundedRectComponent* background) : Background(), Definition(), Entity(), Index(0)
{
this->Definition = (definition != nullptr ? definition : throw new ArgumentNullException("definition"));
this->Index = index;
this->Entity = (entity != nullptr ? entity : throw new ArgumentNullException("entity"));
this->Background = (background != nullptr ? background : throw new ArgumentNullException("background"));
}


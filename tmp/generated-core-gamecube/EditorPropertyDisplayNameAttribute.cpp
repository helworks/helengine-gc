#ifdef DrawText
#undef DrawText
#endif
#include "EditorPropertyDisplayNameAttribute.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string EditorPropertyDisplayNameAttribute::get_DisplayName()
{
return this->DisplayName;
}

EditorPropertyDisplayNameAttribute::EditorPropertyDisplayNameAttribute(std::string displayName) : DisplayName()
{
    if (String::IsNullOrWhiteSpace(displayName))
    {
throw ([&]() {
auto __ctor_arg_00000213 = "Display name must be provided.";
auto __ctor_arg_00000214 = "displayName";
return new ArgumentException(__ctor_arg_00000213, __ctor_arg_00000214);
})();
    }
this->DisplayName = displayName;
}


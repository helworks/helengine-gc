#ifdef DrawText
#undef DrawText
#endif
#include "ShaderVariant.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

List<std::string>* ShaderVariant::get_Defines()
{
return new List<std::string>(this->defines);}

std::string ShaderVariant::get_Name()
{
return this->Name;
}

ShaderVariant::ShaderVariant(std::string name, Array<std::string>* defines) : Name(), defines()
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_000001F3 = "Variant name must be provided.";
auto __ctor_arg_000001F4 = "name";
return new ArgumentException(__ctor_arg_000001F3, __ctor_arg_000001F4);
})();
    }
    if (defines == nullptr)
    {
throw new ArgumentNullException("defines");
    }
this->Name = name;
this->defines = defines;
}


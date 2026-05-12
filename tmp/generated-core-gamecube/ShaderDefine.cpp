#ifdef DrawText
#undef DrawText
#endif
#include "ShaderDefine.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string ShaderDefine::get_Name()
{
return this->Name;
}

std::string ShaderDefine::get_Value()
{
return this->Value;
}

ShaderDefine::ShaderDefine(std::string name, std::string value) : Name(), Value()
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_000001B0 = "Define name must be provided.";
auto __ctor_arg_000001B1 = "name";
return new ArgumentException(__ctor_arg_000001B0, __ctor_arg_000001B1);
})();
    }
    if (String::IsNullOrEmpty(value))
    {
throw new ArgumentNullException("value");
    }
this->Name = name;
this->Value = value;
}


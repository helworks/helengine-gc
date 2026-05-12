#ifdef DrawText
#undef DrawText
#endif
#include "ShaderConstantMember.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string ShaderConstantMember::get_Name()
{
return this->Name;
}

int32_t ShaderConstantMember::get_Offset()
{
return this->Offset;
}

int32_t ShaderConstantMember::get_Size()
{
return this->Size;
}

std::string ShaderConstantMember::get_Type()
{
return this->Type;
}

ShaderConstantMember::ShaderConstantMember(std::string name, std::string type, int32_t offset, int32_t size) : Name(), Offset(0), Size(0), Type()
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_000001A7 = "Member name must be provided.";
auto __ctor_arg_000001A8 = "name";
return new ArgumentException(__ctor_arg_000001A7, __ctor_arg_000001A8);
})();
    }
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_000001A9 = "Member type must be provided.";
auto __ctor_arg_000001AA = "type";
return new ArgumentException(__ctor_arg_000001A9, __ctor_arg_000001AA);
})();
    }
    if (offset < 0)
    {
throw ([&]() {
auto __ctor_arg_000001AB = "offset";
auto __ctor_arg_000001AC = "Offset cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000001AB, __ctor_arg_000001AC);
})();
    }
    if (size < 0)
    {
throw ([&]() {
auto __ctor_arg_000001AD = "size";
auto __ctor_arg_000001AE = "Size cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000001AD, __ctor_arg_000001AE);
})();
    }
this->Name = name;
this->Type = type;
this->Offset = offset;
this->Size = size;
}


#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeStorageProfileId.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeStorageProfileId::get_Value()
{
return this->Value;
}

RuntimeStorageProfileId::RuntimeStorageProfileId(std::string value) : Value()
{
    if (String::IsNullOrWhiteSpace(value))
    {
throw ([&]() {
auto __ctor_arg_0000013F = "Runtime storage-profile id is required.";
auto __ctor_arg_00000140 = "value";
return new ArgumentException(__ctor_arg_0000013F, __ctor_arg_00000140);
})();
    }
this->Value = value;
}

std::string RuntimeStorageProfileId::ToString()
{
return this->Value;}


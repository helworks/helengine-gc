#ifdef DrawText
#undef DrawText
#endif
#include "MaterialLayoutBinding.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string MaterialLayoutBinding::get_Name()
{
return this->Name;
}

::ShaderResourceType MaterialLayoutBinding::get_ResourceType()
{
return this->ResourceType;
}

int32_t MaterialLayoutBinding::get_Set()
{
return this->Set;
}

int32_t MaterialLayoutBinding::get_Size()
{
return this->Size;
}

int32_t MaterialLayoutBinding::get_Slot()
{
return this->Slot;
}

MaterialLayoutBinding::MaterialLayoutBinding(std::string name, ::ShaderResourceType resourceType, int32_t set, int32_t slot, int32_t size) : Name(), ResourceType(), Set(0), Size(0), Slot(0)
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_000000CF = "Binding name must be provided.";
auto __ctor_arg_000000D0 = "name";
return new ArgumentException(__ctor_arg_000000CF, __ctor_arg_000000D0);
})();
    }
    if (set < 0)
    {
throw ([&]() {
auto __ctor_arg_000000D1 = "set";
auto __ctor_arg_000000D2 = "Binding set cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000000D1, __ctor_arg_000000D2);
})();
    }
    if (slot < 0)
    {
throw ([&]() {
auto __ctor_arg_000000D3 = "slot";
auto __ctor_arg_000000D4 = "Binding slot cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000000D3, __ctor_arg_000000D4);
})();
    }
    if (size < 0)
    {
throw ([&]() {
auto __ctor_arg_000000D5 = "size";
auto __ctor_arg_000000D6 = "Binding size cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000000D5, __ctor_arg_000000D6);
})();
    }
this->Name = name;
this->ResourceType = resourceType;
this->Set = set;
this->Slot = slot;
this->Size = size;
}


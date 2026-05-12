#ifdef DrawText
#undef DrawText
#endif
#include "ShaderBinding.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

List<::ShaderConstantMember*>* ShaderBinding::get_Members()
{
return new List<ShaderConstantMember*>(this->members);}

std::string ShaderBinding::get_Name()
{
return this->Name;
}

int32_t ShaderBinding::get_Set()
{
return this->Set;
}

int32_t ShaderBinding::get_Size()
{
return this->Size;
}

int32_t ShaderBinding::get_Slot()
{
return this->Slot;
}

::ShaderResourceType ShaderBinding::get_Type()
{
return this->Type;
}

ShaderBinding::ShaderBinding(std::string name, ::ShaderResourceType type, int32_t set, int32_t slot, int32_t size, Array<::ShaderConstantMember*>* members) : Name(), Set(0), Size(0), Slot(0), Type(), members()
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_00000161 = "Binding name must be provided.";
auto __ctor_arg_00000162 = "name";
return new ArgumentException(__ctor_arg_00000161, __ctor_arg_00000162);
})();
    }
    if (set < 0)
    {
throw ([&]() {
auto __ctor_arg_00000163 = "set";
auto __ctor_arg_00000164 = "Set cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000163, __ctor_arg_00000164);
})();
    }
    if (slot < 0)
    {
throw ([&]() {
auto __ctor_arg_00000165 = "slot";
auto __ctor_arg_00000166 = "Slot cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000165, __ctor_arg_00000166);
})();
    }
    if (size < 0)
    {
throw ([&]() {
auto __ctor_arg_00000167 = "size";
auto __ctor_arg_00000168 = "Size cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000167, __ctor_arg_00000168);
})();
    }
    if (members == nullptr)
    {
throw new ArgumentNullException("members");
    }
this->Name = name;
this->Type = type;
this->Set = set;
this->Slot = slot;
this->Size = size;
this->members = members;
}


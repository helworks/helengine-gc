#ifdef DrawText
#undef DrawText
#endif
#include "ShaderBindingAsset.hpp"
#include "runtime/array.hpp"
#include "ShaderBinding.hpp"
#include "runtime/native_exceptions.hpp"
#include "ShaderBindingAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "ShaderConstantMember.hpp"
#include "ShaderConstantMemberAsset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

ShaderBindingAsset::ShaderBindingAsset() : Members(), Name(), Set(0), Size(0), Slot(0), Type()
{
}

::ShaderBindingAsset* ShaderBindingAsset::FromBinding(::ShaderBinding* binding)
{
    if (binding == nullptr)
    {
throw new ArgumentNullException("binding");
    }
::ShaderBindingAsset *asset = ([&]() {
auto __object_00000169 = new ::ShaderBindingAsset();
__object_00000169->Name = binding->get_Name();
__object_00000169->Type = binding->get_Type();
__object_00000169->Set = binding->get_Set();
__object_00000169->Slot = binding->get_Slot();
__object_00000169->Size = binding->get_Size();
__object_00000169->Members = BuildMemberAssets(binding);
return __object_00000169;
})();
return asset;}

::ShaderBinding* ShaderBindingAsset::ToBinding()
{
this->Validate();
Array<::ShaderConstantMember*> *members = this->BuildMembers();
return new ::ShaderBinding(this->Name, this->Type, this->Set, this->Slot, this->Size, members);}

Array<::ShaderConstantMemberAsset*>* ShaderBindingAsset::BuildMemberAssets(::ShaderBinding* binding)
{
List<::ShaderConstantMember*> *members = binding->get_Members();
Array<::ShaderConstantMemberAsset*> *assets = new Array<ShaderConstantMemberAsset*>(members->get_Count());
for (int32_t i = 0; i < members->get_Count(); i++) {
(*assets)[i] = ShaderConstantMemberAsset::FromMember((*members)[i]);
}
return assets;}

Array<::ShaderConstantMember*>* ShaderBindingAsset::BuildMembers()
{
Array<::ShaderConstantMember*> *members = new Array<ShaderConstantMember*>(this->Members->Length);
for (int32_t i = 0; i < this->Members->Length; i++) {
::ShaderConstantMemberAsset *member = (*this->Members)[i];
    if (member == nullptr)
    {
throw new InvalidOperationException("Binding members contain a null entry.");
    }
(*members)[i] = member->ToMember();
}
return members;}

void ShaderBindingAsset::Validate()
{
    if (String::IsNullOrWhiteSpace(this->Name))
    {
throw new InvalidOperationException("Binding name must be provided.");
    }
else     if (this->Set < 0)
    {
throw new InvalidOperationException("Binding set cannot be negative.");
    }
else     if (this->Slot < 0)
    {
throw new InvalidOperationException("Binding slot cannot be negative.");
    }
else     if (this->Size < 0)
    {
throw new InvalidOperationException("Binding size cannot be negative.");
    }
else     if (this->Members == nullptr)
    {
throw new InvalidOperationException("Binding members must be provided.");
    }
}


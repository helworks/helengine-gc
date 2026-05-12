#ifdef DrawText
#undef DrawText
#endif
#include "ShaderConstantMemberAsset.hpp"
#include "ShaderConstantMember.hpp"
#include "runtime/native_exceptions.hpp"
#include "ShaderConstantMemberAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

ShaderConstantMemberAsset::ShaderConstantMemberAsset() : Name(), Offset(0), Size(0), Type()
{
}

::ShaderConstantMemberAsset* ShaderConstantMemberAsset::FromMember(::ShaderConstantMember* member)
{
    if (member == nullptr)
    {
throw new ArgumentNullException("member");
    }
::ShaderConstantMemberAsset *asset = ([&]() {
auto __object_000001AF = new ::ShaderConstantMemberAsset();
__object_000001AF->Name = member->get_Name();
__object_000001AF->Type = member->get_Type();
__object_000001AF->Offset = member->get_Offset();
__object_000001AF->Size = member->get_Size();
return __object_000001AF;
})();
return asset;}

::ShaderConstantMember* ShaderConstantMemberAsset::ToMember()
{
this->Validate();
return new ::ShaderConstantMember(this->Name, this->Type, this->Offset, this->Size);}

void ShaderConstantMemberAsset::Validate()
{
    if (String::IsNullOrWhiteSpace(this->Name))
    {
throw new InvalidOperationException("Member name must be provided.");
    }
else     if (String::IsNullOrWhiteSpace(this->Type))
    {
throw new InvalidOperationException("Member type must be provided.");
    }
else     if (this->Offset < 0)
    {
throw new InvalidOperationException("Member offset cannot be negative.");
    }
else     if (this->Size < 0)
    {
throw new InvalidOperationException("Member size cannot be negative.");
    }
}


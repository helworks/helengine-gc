#ifdef DrawText
#undef DrawText
#endif
#include "ShaderTargetNames.hpp"
#include "runtime/native_string.hpp"
#include "ShaderCompileTarget.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string ShaderTargetNames::GetTargetName(::ShaderCompileTarget target)
{
switch (target) {
case ShaderCompileTarget::DirectX9: {
return "dx9";}
case ShaderCompileTarget::DirectX11: {
return "dx11";}
case ShaderCompileTarget::DirectX12: {
return "dx12";}
case ShaderCompileTarget::Vulkan: {
return "vulkan";}
case ShaderCompileTarget::Metal: {
return "metal";}
default:  {
throw ([&]() {
auto __ctor_arg_000001EF = "target";
auto __ctor_arg_000001F0 = "Unsupported compile target.";
return new ArgumentOutOfRangeException(__ctor_arg_000001EF, __ctor_arg_000001F0);
})();
}
}

}

::ShaderCompileTarget ShaderTargetNames::ParseTarget(std::string name)
{
::ShaderCompileTarget target;
    if (!TryParseTarget(name, target))
    {
throw ([&]() {
auto __ctor_arg_000001F1 = "Target name was not recognized.";
auto __ctor_arg_000001F2 = "name";
return new ArgumentException(__ctor_arg_000001F1, __ctor_arg_000001F2);
})();
    }
return target;}

bool ShaderTargetNames::TryParseTarget(std::string name, ::ShaderCompileTarget& target)
{
    if (String::IsNullOrWhiteSpace(name))
    {
target = ShaderCompileTarget::DirectX11;
return false;    }
{
const std::string __switchValue19_1 = String::ToLowerInvariant(String::Trim(name));
if (String::Equals(__switchValue19_1, "dx9")) {
target = ShaderCompileTarget::DirectX9;
return true;}
else if (String::Equals(__switchValue19_1, "dx11")) {
target = ShaderCompileTarget::DirectX11;
return true;}
else if (String::Equals(__switchValue19_1, "dx12")) {
target = ShaderCompileTarget::DirectX12;
return true;}
else if (String::Equals(__switchValue19_1, "vulkan")) {
target = ShaderCompileTarget::Vulkan;
return true;}
else if (String::Equals(__switchValue19_1, "metal")) {
target = ShaderCompileTarget::Metal;
return true;}
else {
target = ShaderCompileTarget::DirectX11;
return false;}
}

}


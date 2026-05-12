#ifdef DrawText
#undef DrawText
#endif
#include "ShaderPlatformDefines.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "ShaderDefine.hpp"
#include "ShaderCompileTarget.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

Array<::ShaderDefine*>* ShaderPlatformDefines::BuildDefines(::ShaderCompileTarget target, ::ShaderModel* shaderModel, List<::ShaderDefine*>* additionalDefines)
{
    if (shaderModel == nullptr)
    {
throw new ArgumentNullException("shaderModel");
    }
    if (additionalDefines == nullptr)
    {
throw new ArgumentNullException("additionalDefines");
    }
List<::ShaderDefine*> *defines = new List<::ShaderDefine*>(additionalDefines->get_Count() + 3);
AddTargetDefine(target, defines);
defines->Add(([&]() {
auto __ctor_arg_000001D0 = "HEL_SM_MAJOR";
auto __ctor_arg_000001D1 = std::to_string(shaderModel->get_Major());
return new ::ShaderDefine(__ctor_arg_000001D0, __ctor_arg_000001D1);
})());
defines->Add(([&]() {
auto __ctor_arg_000001D2 = "HEL_SM_MINOR";
auto __ctor_arg_000001D3 = std::to_string(shaderModel->get_Minor());
return new ::ShaderDefine(__ctor_arg_000001D2, __ctor_arg_000001D3);
})());
for (int32_t i = 0; i < additionalDefines->get_Count(); i++) {
defines->Add((*additionalDefines)[i]);
}
return defines->ToArray();}

void ShaderPlatformDefines::AddTargetDefine(::ShaderCompileTarget target, List<::ShaderDefine*>* defines)
{
const std::string defineName = GetTargetDefineName(target);
defines->Add(new ::ShaderDefine(defineName, "1"));
}

std::string ShaderPlatformDefines::GetTargetDefineName(::ShaderCompileTarget target)
{
switch (target) {
case ShaderCompileTarget::DirectX9: {
return "HEL_API_DX9";}
case ShaderCompileTarget::DirectX11: {
return "HEL_API_DX11";}
case ShaderCompileTarget::DirectX12: {
return "HEL_API_DX12";}
case ShaderCompileTarget::Vulkan: {
return "HEL_API_VULKAN";}
case ShaderCompileTarget::Metal: {
return "HEL_API_METAL";}
default:  {
throw ([&]() {
auto __ctor_arg_000001D4 = "target";
auto __ctor_arg_000001D5 = "Unsupported compile target.";
return new ArgumentOutOfRangeException(__ctor_arg_000001D4, __ctor_arg_000001D5);
})();
}
}

}


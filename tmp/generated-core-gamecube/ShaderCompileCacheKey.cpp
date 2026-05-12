#ifdef DrawText
#undef DrawText
#endif
#include "ShaderCompileCacheKey.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string ShaderCompileCacheKey::get_BindingPolicySignature()
{
return this->BindingPolicySignature;
}

std::string ShaderCompileCacheKey::get_DefinesSignature()
{
return this->DefinesSignature;
}

std::string ShaderCompileCacheKey::get_EntryPoint()
{
return this->EntryPoint;
}

std::string ShaderCompileCacheKey::get_ProgramName()
{
return this->ProgramName;
}

::ShaderModel* ShaderCompileCacheKey::get_ShaderModel()
{
return this->ShaderModel;
}

std::string ShaderCompileCacheKey::get_SourceHash()
{
return this->SourceHash;
}

::ShaderStage ShaderCompileCacheKey::get_Stage()
{
return this->Stage;
}

::ShaderCompileTarget ShaderCompileCacheKey::get_Target()
{
return this->Target;
}

std::string ShaderCompileCacheKey::get_Variant()
{
return this->Variant;
}

ShaderCompileCacheKey::ShaderCompileCacheKey(std::string sourceHash, std::string programName, std::string entryPoint, ::ShaderStage stage, ::ShaderCompileTarget target, ::ShaderModel* shaderModel, std::string variant, std::string definesSignature, std::string bindingPolicySignature) : BindingPolicySignature(), DefinesSignature(), EntryPoint(), ProgramName(), ShaderModel(), SourceHash(), Stage(), Target(), Variant(), key()
{
    if (String::IsNullOrWhiteSpace(sourceHash))
    {
throw ([&]() {
auto __ctor_arg_00000178 = "Source hash must be provided.";
auto __ctor_arg_00000179 = "sourceHash";
return new ArgumentException(__ctor_arg_00000178, __ctor_arg_00000179);
})();
    }
    if (String::IsNullOrWhiteSpace(programName))
    {
throw ([&]() {
auto __ctor_arg_0000017A = "Program name must be provided.";
auto __ctor_arg_0000017B = "programName";
return new ArgumentException(__ctor_arg_0000017A, __ctor_arg_0000017B);
})();
    }
    if (String::IsNullOrWhiteSpace(entryPoint))
    {
throw ([&]() {
auto __ctor_arg_0000017C = "Entry point must be provided.";
auto __ctor_arg_0000017D = "entryPoint";
return new ArgumentException(__ctor_arg_0000017C, __ctor_arg_0000017D);
})();
    }
    if (shaderModel == nullptr)
    {
throw new ArgumentNullException("shaderModel");
    }
    if (String::IsNullOrWhiteSpace(variant))
    {
throw ([&]() {
auto __ctor_arg_0000017E = "Variant name must be provided.";
auto __ctor_arg_0000017F = "variant";
return new ArgumentException(__ctor_arg_0000017E, __ctor_arg_0000017F);
})();
    }
    if (String::IsNullOrEmpty(definesSignature))
    {
throw new ArgumentNullException("definesSignature");
    }
    if (String::IsNullOrEmpty(bindingPolicySignature))
    {
throw new ArgumentNullException("bindingPolicySignature");
    }
this->SourceHash = sourceHash;
this->ProgramName = programName;
this->EntryPoint = entryPoint;
this->Stage = stage;
this->Target = target;
this->ShaderModel = shaderModel;
this->Variant = variant;
this->DefinesSignature = definesSignature;
this->BindingPolicySignature = bindingPolicySignature;
this->key = this->BuildKey();
}

std::string ShaderCompileCacheKey::ToString()
{
return this->key;}

std::string ShaderCompileCacheKey::BuildKey()
{
return String::Join("|", this->SourceHash, this->ProgramName, this->EntryPoint, std::to_string(static_cast<int32_t>(this->Stage)), std::to_string(static_cast<int32_t>(this->Target)), this->ShaderModel->ToString(), this->Variant, this->DefinesSignature, this->BindingPolicySignature);}


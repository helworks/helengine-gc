#ifdef DrawText
#undef DrawText
#endif
#include "ShaderCompileOptions.hpp"
#include "runtime/native_exceptions.hpp"

::ShaderBindingPolicy* ShaderCompileOptions::get_BindingPolicy()
{
return this->BindingPolicy;
}

bool ShaderCompileOptions::get_GenerateDebugInfo()
{
return this->GenerateDebugInfo;
}

bool ShaderCompileOptions::get_Optimize()
{
return this->Optimize;
}

bool ShaderCompileOptions::get_TreatWarningsAsErrors()
{
return this->TreatWarningsAsErrors;
}

ShaderCompileOptions::ShaderCompileOptions(::ShaderBindingPolicy* bindingPolicy, bool generateDebugInfo, bool optimize, bool treatWarningsAsErrors) : BindingPolicy(), GenerateDebugInfo(), Optimize(), TreatWarningsAsErrors()
{
    if (bindingPolicy == nullptr)
    {
throw new ArgumentNullException("bindingPolicy");
    }
this->BindingPolicy = bindingPolicy;
this->GenerateDebugInfo = generateDebugInfo;
this->Optimize = optimize;
this->TreatWarningsAsErrors = treatWarningsAsErrors;
}


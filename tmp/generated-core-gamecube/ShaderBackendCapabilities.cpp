#ifdef DrawText
#undef DrawText
#endif
#include "ShaderBackendCapabilities.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::ShaderModel* ShaderBackendCapabilities::get_MaximumShaderModel()
{
return this->MaximumShaderModel;
}

::ShaderModel* ShaderBackendCapabilities::get_MinimumShaderModel()
{
return this->MinimumShaderModel;
}

List<::ShaderStage>* ShaderBackendCapabilities::get_SupportedStages()
{
return this->SupportedStages;
}

bool ShaderBackendCapabilities::get_SupportsRayTracing()
{
return this->SupportsRayTracing;
}

ShaderBackendCapabilities::ShaderBackendCapabilities(::ShaderModel* minimumShaderModel, ::ShaderModel* maximumShaderModel, List<::ShaderStage>* supportedStages, bool supportsRayTracing) : MaximumShaderModel(), MinimumShaderModel(), SupportedStages(), SupportsRayTracing()
{
    if (minimumShaderModel == nullptr)
    {
throw new ArgumentNullException("minimumShaderModel");
    }
    if (maximumShaderModel == nullptr)
    {
throw new ArgumentNullException("maximumShaderModel");
    }
    if (supportedStages == nullptr)
    {
throw new ArgumentNullException("supportedStages");
    }
this->MinimumShaderModel = minimumShaderModel;
this->MaximumShaderModel = maximumShaderModel;
this->SupportedStages = supportedStages;
this->SupportsRayTracing = supportsRayTracing;
}


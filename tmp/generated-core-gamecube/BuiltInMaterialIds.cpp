#ifdef DrawText
#undef DrawText
#endif
#include "BuiltInMaterialIds.hpp"
#include "runtime/native_string.hpp"

std::string BuiltInMaterialIds::StandardForwardPixelProgramName = "ForwardStandardShader.ps";

std::string BuiltInMaterialIds::StandardForwardShaderAssetId = "ForwardStandardShader";

std::string BuiltInMaterialIds::StandardForwardVertexProgramName = "ForwardStandardShader.vs";

std::string BuiltInMaterialIds::StandardMaterialShaderAssetId = "engine:material:standard";

std::string BuiltInMaterialIds::StandardRuntimeMaterialAssetId = "Engine.Materials.Standard.material";

bool BuiltInMaterialIds::UsesStandardMeshTransform(std::string materialId)
{
return String::Equals(materialId, StandardRuntimeMaterialAssetId, StringComparison::Ordinal);}

bool BuiltInMaterialIds::UsesStandardMeshTransform(std::string materialId, std::string shaderAssetId, std::string vertexProgram, std::string pixelProgram)
{
    if (UsesStandardMeshTransform(materialId))
    {
return true;    }
return String::Equals(shaderAssetId, StandardForwardShaderAssetId, StringComparison::Ordinal) && String::Equals(vertexProgram, StandardForwardVertexProgramName, StringComparison::Ordinal) && String::Equals(pixelProgram, StandardForwardPixelProgramName, StringComparison::Ordinal);}


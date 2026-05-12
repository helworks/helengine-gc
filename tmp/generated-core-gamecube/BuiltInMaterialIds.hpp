#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class BuiltInMaterialIds
{
public:
    virtual ~BuiltInMaterialIds() = default;

    static std::string StandardForwardPixelProgramName;

    static std::string StandardForwardShaderAssetId;

    static std::string StandardForwardVertexProgramName;

    static std::string StandardMaterialShaderAssetId;

    static std::string StandardRuntimeMaterialAssetId;

    static bool UsesStandardMeshTransform(std::string materialId);

    static bool UsesStandardMeshTransform(std::string materialId, std::string shaderAssetId, std::string vertexProgram, std::string pixelProgram);
};

#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;

#include "Asset.hpp"
#include "Ps2MaterialAlphaMode.hpp"
#include "Ps2MaterialLightingMode.hpp"
#include "Ps2RenderClass.hpp"
#include "runtime/native_string.hpp"

class Ps2MaterialAsset : public Asset
{
public:
    virtual ~Ps2MaterialAsset() = default;

    Ps2MaterialAsset();

    ::Ps2MaterialAlphaMode AlphaMode;

    uint8_t BaseColorA;

    uint8_t BaseColorB;

    uint8_t BaseColorG;

    uint8_t BaseColorR;

    bool CastShadows;

    bool DoubleSided;

    float EmissiveStrength;

    bool ExpensiveModeAllowed;

    ::Ps2MaterialLightingMode LightingMode;

    ::Ps2RenderClass RenderClass;

    std::string RendererFamilyId;

    float Roughness;

    float SpecularStrength;

    std::string TextureRelativePath;

    bool UseVertexColor;

    std::string get_Id();

    void set_Id(std::string value);
};

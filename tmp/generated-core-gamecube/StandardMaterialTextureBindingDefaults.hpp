#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeMaterial;
class MaterialLayout;
class RuntimeTexture;
class MaterialPropertyBlock;
class Core;

#include "runtime/native_exceptions.hpp"
#include "RuntimeMaterial.hpp"
#include "MaterialLayout.hpp"
#include "RuntimeTexture.hpp"
#include "MaterialPropertyBlock.hpp"
#include "Core.hpp"
#include "Core.hpp"
#include "runtime/native_string.hpp"
#include "RuntimeMaterial.hpp"

class StandardMaterialTextureBindingDefaults
{
public:
    virtual ~StandardMaterialTextureBindingDefaults() = default;

    static std::string DiffuseTextureBindingName;

    static void Apply(::RuntimeMaterial* material);
};

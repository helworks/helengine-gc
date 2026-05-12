#ifdef DrawText
#undef DrawText
#endif
#include "StandardMaterialTextureBindingDefaults.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeMaterial.hpp"
#include "MaterialLayout.hpp"
#include "RuntimeTexture.hpp"
#include "MaterialPropertyBlock.hpp"
#include "Core.hpp"
#include "TextureUtils.hpp"
#include "runtime/native_exceptions.hpp"

std::string StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName = "DiffuseTexture";

void StandardMaterialTextureBindingDefaults::Apply(::RuntimeMaterial* material)
{
    if (material == nullptr)
    {
throw new ArgumentNullException("material");
    }
const int32_t diffuseTextureBindingIndex = material->get_Layout()->FindTextureBindingIndex(DiffuseTextureBindingName);
    if (diffuseTextureBindingIndex < 0)
    {
return;    }
::RuntimeTexture *diffuseTexture = material->get_Properties()->GetTexture(diffuseTextureBindingIndex);
    if (diffuseTexture != nullptr)
    {
return;    }
::Core *core = Core::get_Instance();
    if (core == nullptr || core->get_RenderManager2D() == nullptr)
    {
return;    }
material->get_Properties()->SetTexture(diffuseTextureBindingIndex, TextureUtils::get_PixelTexture());
}


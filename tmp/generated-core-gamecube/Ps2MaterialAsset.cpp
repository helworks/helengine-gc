#ifdef DrawText
#undef DrawText
#endif
#include "Ps2MaterialAsset.hpp"

Ps2MaterialAsset::Ps2MaterialAsset() : AlphaMode(), BaseColorA(), BaseColorB(), BaseColorG(), BaseColorR(), CastShadows(), DoubleSided(), EmissiveStrength(), ExpensiveModeAllowed(), LightingMode(), RenderClass(), RendererFamilyId(), Roughness(), SpecularStrength(), TextureRelativePath(), UseVertexColor()
{
}

std::string Ps2MaterialAsset::get_Id()
{
return this->Asset::get_Id();
}

void Ps2MaterialAsset::set_Id(std::string value)
{
this->Asset::set_Id(value);
}


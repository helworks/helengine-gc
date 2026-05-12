#ifdef DrawText
#undef DrawText
#endif
#include "MaterialAsset.hpp"
#include "MaterialRenderState.hpp"
#include "runtime/array.hpp"

MaterialAsset::MaterialAsset() : CastsShadows(), ConstantBuffers(), DiffuseTextureAssetId(), EmissiveTextureAssetId(), NormalTextureAssetId(), PixelProgram(), ReceivesShadows(), RenderState(), ShaderAssetId(), Variant(), VertexProgram()
{
this->RenderState = new ::MaterialRenderState();
this->ConstantBuffers = Array<MaterialConstantBufferAsset*>::Empty();
this->CastsShadows = true;
this->ReceivesShadows = true;
}

std::string MaterialAsset::get_Id()
{
return this->Asset::get_Id();
}

void MaterialAsset::set_Id(std::string value)
{
this->Asset::set_Id(value);
}


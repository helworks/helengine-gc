#ifdef DrawText
#undef DrawText
#endif
#include "TextureUtils.hpp"
#include "TextureAsset.hpp"
#include "RuntimeTexture.hpp"
#include "Core.hpp"

::RuntimeTexture* TextureUtils::get_BlackPixelTexture()
{
    if (blackPixelTexture == nullptr)
    {
blackPixelTexture = BuildSolidPixelTexture(0, 0, 0, 255);
    }
return blackPixelTexture;}

::RuntimeTexture* TextureUtils::get_PixelTexture()
{
    if (pixelTexture == nullptr)
    {
pixelTexture = BuildSolidPixelTexture(255, 255, 255, 255);
    }
return pixelTexture;}

::RuntimeTexture* TextureUtils::blackPixelTexture;

::RuntimeTexture* TextureUtils::pixelTexture;

::RuntimeTexture* TextureUtils::BuildSolidPixelTexture(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
::TextureAsset *rawTex = new ::TextureAsset();
rawTex->Colors = new Array<uint8_t>({ red, green, blue, alpha });
rawTex->Width = 1;
rawTex->Height = 1;
return Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(rawTex);}


#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class TextureAsset;
class RuntimeTexture;

#include "TextureAsset.hpp"
#include "RuntimeTexture.hpp"
#include "RuntimeTexture.hpp"

class TextureUtils
{
public:
    virtual ~TextureUtils() = default;

    static ::RuntimeTexture* get_BlackPixelTexture();

    static ::RuntimeTexture* get_PixelTexture();
private:
    static ::RuntimeTexture* blackPixelTexture;

    static ::RuntimeTexture* pixelTexture;

    static ::RuntimeTexture* BuildSolidPixelTexture(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
};

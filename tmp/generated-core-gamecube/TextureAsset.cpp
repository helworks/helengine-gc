#ifdef DrawText
#undef DrawText
#endif
#include "TextureAsset.hpp"
#include "runtime/array.hpp"

TextureAsset::TextureAsset() : Colors(), Height(), Width()
{
}

std::string TextureAsset::get_Id()
{
return this->Asset::get_Id();
}

void TextureAsset::set_Id(std::string value)
{
this->Asset::set_Id(value);
}


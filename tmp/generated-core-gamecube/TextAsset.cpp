#ifdef DrawText
#undef DrawText
#endif
#include "TextAsset.hpp"

TextAsset::TextAsset() : Text()
{
}

std::string TextAsset::get_Id()
{
return this->Asset::get_Id();
}

void TextAsset::set_Id(std::string value)
{
this->Asset::set_Id(value);
}


#ifdef DrawText
#undef DrawText
#endif
#include "FontInfo.hpp"

int32_t FontInfo::get_LineSpacing()
{
return this->LineSpacing;
}

void FontInfo::set_LineSpacing(int32_t value)
{
this->LineSpacing = value;
}

std::string FontInfo::get_Name()
{
return this->Name;
}

void FontInfo::set_Name(std::string value)
{
this->Name = value;
}

float FontInfo::get_SpaceWidth()
{
return this->SpaceWidth;
}

void FontInfo::set_SpaceWidth(float value)
{
this->SpaceWidth = value;
}

FontInfo::FontInfo(std::string name, int32_t lineSpacing, float spaceWidth) : LineSpacing(0), Name(), SpaceWidth()
{
this->set_Name(name);
this->set_LineSpacing(lineSpacing);
this->set_SpaceWidth(spaceWidth);
}


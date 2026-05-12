#ifdef DrawText
#undef DrawText
#endif
#include "TextContent.hpp"
#include "runtime/native_string.hpp"

TextContent::TextContent() : Text(String::Empty)
{
}

std::string TextContent::get_Text()
{
return this->Text;
}

void TextContent::set_Text(std::string value)
{
this->Text = value;
}


#ifdef DrawText
#undef DrawText
#endif
#include "TextLayoutUtils.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/text/string-builder.hpp"
#include "FontChar.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/text/string-builder.hpp"

std::string TextLayoutUtils::WrapText(std::string text, ::FontAsset* font, int32_t maxWidth)
{
    if (font == nullptr)
    {
throw new ArgumentNullException("font");
    }
    if (String::IsNullOrEmpty(text) || maxWidth <= 0)
    {
return text;    }
StringBuilder *wrappedText = new StringBuilder(static_cast<int32_t>(text.size()) + 16);
int32_t lineStartIndex = 0;
int32_t lastWrapIndex = -1;
double lineWidth = 0.0;
double widthAtLastWrap = 0.0;
int32_t wrappedLength = 0;
for (int32_t index = 0; index < static_cast<int32_t>(text.size()); index++) {
const char character = text[index];
    if (character == '\r')
    {
continue;
    }
    if (character == '\n')
    {
wrappedText->Append('\n');
wrappedLength++;
lineStartIndex = wrappedLength;
lastWrapIndex = -1;
widthAtLastWrap = 0.0;
lineWidth = 0.0;
continue;
    }
const double characterWidth = GetCharacterAdvanceWidth(font, character);
    if (lineWidth > 0.0 && lineWidth + characterWidth > maxWidth)
    {
    if (lastWrapIndex >= lineStartIndex)
    {
const int32_t suffixStartIndex = lastWrapIndex + 1;
const std::string currentText = wrappedText->ToString();
const std::string suffix = String::Substring(currentText, suffixStartIndex);
wrappedText = new StringBuilder(String::Substring(currentText, 0, lastWrapIndex));
wrappedText->Append('\n');
wrappedText->Append(suffix);
wrappedLength = lastWrapIndex + 1 + static_cast<int32_t>(suffix.size());
lineStartIndex = wrappedLength - static_cast<int32_t>(suffix.size());
lineWidth -= widthAtLastWrap;
    }
else {
wrappedText->Append('\n');
wrappedLength++;
lineStartIndex = wrappedLength;
lineWidth = 0.0;
}
lastWrapIndex = -1;
widthAtLastWrap = 0.0;
    if (character == ' ')
    {
continue;
    }
index--;
continue;
    }
wrappedText->Append(character);
wrappedLength++;
lineWidth += characterWidth;
    if (character == ' ')
    {
lastWrapIndex = wrappedLength - 1;
widthAtLastWrap = lineWidth;
    }
}
return wrappedText->ToString();}

double TextLayoutUtils::GetCharacterAdvanceWidth(::FontAsset* font, char character)
{
    if (character == ' ')
    {
return font->get_FontInfo()->get_SpaceWidth();    }
::FontChar glyph;
    if (!font->get_Characters()->TryGetValue(character, glyph))
    {
return 0.0;    }
const double pixelWidth = glyph.SourceRect.Z * font->get_AtlasWidth();
    if (glyph.AdvanceWidth > 0.0f)
    {
return glyph.AdvanceWidth;    }
return pixelWidth;}


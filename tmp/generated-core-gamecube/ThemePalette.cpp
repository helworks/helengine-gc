#ifdef DrawText
#undef DrawText
#endif
#include "ThemePalette.hpp"
#include "runtime/native_exceptions.hpp"

::ThemeColors* ThemePalette::get_Colors()
{
return this->Colors;
}

ThemePalette::ThemePalette(::ThemeColors* colors) : Colors()
{
this->Colors = (colors != nullptr ? colors : throw new ArgumentNullException("colors"));
}


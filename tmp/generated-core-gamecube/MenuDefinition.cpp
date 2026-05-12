#ifdef DrawText
#undef DrawText
#endif
#include "MenuDefinition.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

::byte4 MenuDefinition::get_AccentColor()
{
return this->AccentColor;
}

::byte4 MenuDefinition::get_AccentSecondaryColor()
{
return this->AccentSecondaryColor;
}

::byte4 MenuDefinition::get_BackgroundColor()
{
return this->BackgroundColor;
}

std::string MenuDefinition::get_BodyFontPath()
{
return this->BodyFontPath;
}

std::string MenuDefinition::get_InitialPanelId()
{
return this->InitialPanelId;
}

::byte4 MenuDefinition::get_MutedTextColor()
{
return this->MutedTextColor;
}

Array<::MenuPanelDefinition*>* MenuDefinition::get_Panels()
{
return this->Panels;
}

std::string MenuDefinition::get_Subtitle()
{
return this->Subtitle;
}

::byte4 MenuDefinition::get_SurfaceBorderColor()
{
return this->SurfaceBorderColor;
}

::byte4 MenuDefinition::get_SurfaceColor()
{
return this->SurfaceColor;
}

::byte4 MenuDefinition::get_TextColor()
{
return this->TextColor;
}

std::string MenuDefinition::get_Title()
{
return this->Title;
}

std::string MenuDefinition::get_TitleFontPath()
{
return this->TitleFontPath;
}

MenuDefinition::MenuDefinition(std::string title, std::string subtitle, std::string initialPanelId, std::string titleFontPath, std::string bodyFontPath, ::byte4 backgroundColor, ::byte4 surfaceColor, ::byte4 surfaceBorderColor, ::byte4 accentColor, ::byte4 accentSecondaryColor, ::byte4 textColor, ::byte4 mutedTextColor, Array<::MenuPanelDefinition*>* panels) : AccentColor(), AccentSecondaryColor(), BackgroundColor(), BodyFontPath(), InitialPanelId(), MutedTextColor(), Panels(), Subtitle(), SurfaceBorderColor(), SurfaceColor(), TextColor(), Title(), TitleFontPath()
{
    if (String::IsNullOrEmpty(title))
    {
throw new ArgumentNullException("title");
    }
    if (String::IsNullOrEmpty(subtitle))
    {
throw new ArgumentNullException("subtitle");
    }
    if (String::IsNullOrWhiteSpace(initialPanelId))
    {
throw ([&]() {
auto __ctor_arg_000000E4 = "Initial panel id must be provided.";
auto __ctor_arg_000000E5 = "initialPanelId";
return new ArgumentException(__ctor_arg_000000E4, __ctor_arg_000000E5);
})();
    }
    if (String::IsNullOrWhiteSpace(titleFontPath))
    {
throw ([&]() {
auto __ctor_arg_000000E6 = "Title font path must be provided.";
auto __ctor_arg_000000E7 = "titleFontPath";
return new ArgumentException(__ctor_arg_000000E6, __ctor_arg_000000E7);
})();
    }
    if (String::IsNullOrWhiteSpace(bodyFontPath))
    {
throw ([&]() {
auto __ctor_arg_000000E8 = "Body font path must be provided.";
auto __ctor_arg_000000E9 = "bodyFontPath";
return new ArgumentException(__ctor_arg_000000E8, __ctor_arg_000000E9);
})();
    }
    if (panels == nullptr)
    {
throw new ArgumentNullException("panels");
    }
    if (panels->Length == 0)
    {
throw new InvalidOperationException("Menu definitions must contain at least one panel.");
    }
this->Title = title;
this->Subtitle = subtitle;
this->InitialPanelId = initialPanelId;
this->TitleFontPath = titleFontPath;
this->BodyFontPath = bodyFontPath;
this->BackgroundColor = backgroundColor;
this->SurfaceColor = surfaceColor;
this->SurfaceBorderColor = surfaceBorderColor;
this->AccentColor = accentColor;
this->AccentSecondaryColor = accentSecondaryColor;
this->TextColor = textColor;
this->MutedTextColor = mutedTextColor;
this->Panels = panels;
}


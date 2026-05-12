#ifdef DrawText
#undef DrawText
#endif
#include "ThemeManager.hpp"
#include "ThemeColors.hpp"
#include "ThemePalette.hpp"
#include "byte4.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"

::ThemeColors* ThemeManager::get_Colors()
{
return Current->get_Colors();
}

::ThemePalette* ThemeManager::Current = CreateNeon90s();

::ThemePalette* ThemeManager::get_Current()
{
return ThemeManager::Current;
}

void ThemeManager::set_Current(::ThemePalette* value)
{
ThemeManager::Current = value;
}

::Event ThemeManager::ThemeChanged;

::ThemePalette* ThemeManager::CreateDarkTheme()
{
::ThemeColors *colors = ([&]() {
auto __object_000001FD = new ::ThemeColors();
__object_000001FD->set_BackgroundPrimary(::byte4(20, 20, 20, 255));
__object_000001FD->set_SurfacePrimary(::byte4(35, 35, 35, 255));
__object_000001FD->set_SurfaceInput(::byte4(10, 10, 10, 255));
__object_000001FD->set_AccentPrimary(::byte4(0, 123, 255, 255));
__object_000001FD->set_AccentSecondary(::byte4(108, 117, 125, 255));
__object_000001FD->set_AccentTertiary(::byte4(52, 58, 64, 255));
__object_000001FD->set_AccentQuaternary(::byte4(173, 181, 189, 255));
__object_000001FD->set_StateWarning(::byte4(255, 193, 7, 255));
__object_000001FD->set_StateDanger(::byte4(220, 53, 69, 255));
__object_000001FD->set_StateSuccess(::byte4(40, 167, 69, 255));
__object_000001FD->set_InputForegroundPrimary(::byte4(0, 255, 0, 255));
__object_000001FD->set_InputForegroundSecondary(::byte4(255, 255, 0, 255));
__object_000001FD->set_TextPrimary(::byte4(255, 255, 255, 255));
__object_000001FD->set_TextSecondary(::byte4(173, 181, 189, 255));
__object_000001FD->set_TextOnAccent(::byte4(255, 255, 255, 255));
return __object_000001FD;
})();
return new ::ThemePalette(colors);}

::ThemePalette* ThemeManager::CreateLightTheme()
{
::ThemeColors *colors = ([&]() {
auto __object_000001FE = new ::ThemeColors();
__object_000001FE->set_BackgroundPrimary(::byte4(248, 249, 250, 255));
__object_000001FE->set_SurfacePrimary(::byte4(255, 255, 255, 255));
__object_000001FE->set_SurfaceInput(::byte4(233, 236, 239, 255));
__object_000001FE->set_AccentPrimary(::byte4(0, 123, 255, 255));
__object_000001FE->set_AccentSecondary(::byte4(108, 117, 125, 255));
__object_000001FE->set_AccentTertiary(::byte4(52, 58, 64, 255));
__object_000001FE->set_AccentQuaternary(::byte4(73, 80, 87, 255));
__object_000001FE->set_StateWarning(::byte4(255, 193, 7, 255));
__object_000001FE->set_StateDanger(::byte4(220, 53, 69, 255));
__object_000001FE->set_StateSuccess(::byte4(40, 167, 69, 255));
__object_000001FE->set_InputForegroundPrimary(::byte4(33, 37, 41, 255));
__object_000001FE->set_InputForegroundSecondary(::byte4(52, 58, 64, 255));
__object_000001FE->set_TextPrimary(::byte4(33, 37, 41, 255));
__object_000001FE->set_TextSecondary(::byte4(73, 80, 87, 255));
__object_000001FE->set_TextOnAccent(::byte4(255, 255, 255, 255));
return __object_000001FE;
})();
return new ::ThemePalette(colors);}

::ThemePalette* ThemeManager::CreateNeon90s()
{
::ThemeColors *colors = ([&]() {
auto __object_000001FF = new ::ThemeColors();
__object_000001FF->set_BackgroundPrimary(::byte4(25, 15, 35, 255));
__object_000001FF->set_SurfacePrimary(::byte4(40, 25, 50, 255));
__object_000001FF->set_SurfaceInput(::byte4(15, 15, 15, 255));
__object_000001FF->set_AccentPrimary(::byte4(194, 49, 175, 255));
__object_000001FF->set_AccentSecondary(::byte4(141, 49, 194, 255));
__object_000001FF->set_AccentTertiary(::byte4(68, 49, 194, 255));
__object_000001FF->set_AccentQuaternary(::byte4(204, 204, 204, 255));
__object_000001FF->set_StateWarning(::byte4(255, 178, 102, 255));
__object_000001FF->set_StateDanger(::byte4(255, 80, 80, 255));
__object_000001FF->set_StateSuccess(::byte4(102, 255, 153, 255));
__object_000001FF->set_InputForegroundPrimary(::byte4(0, 255, 0, 255));
__object_000001FF->set_InputForegroundSecondary(::byte4(255, 255, 0, 255));
__object_000001FF->set_TextPrimary(::byte4(25, 15, 35, 255));
__object_000001FF->set_TextSecondary(::byte4(40, 25, 50, 255));
__object_000001FF->set_TextOnAccent(::byte4(25, 15, 35, 255));
return __object_000001FF;
})();
return new ::ThemePalette(colors);}

void ThemeManager::SetTheme(::ThemePalette* palette)
{
ThemeManager::set_Current((palette != nullptr ? palette : throw new ArgumentNullException("palette")));
ThemeChanged.Invoke(nullptr, nullptr);
}


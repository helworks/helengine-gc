#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeSceneCatalogEntry::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

std::string RuntimeSceneCatalogEntry::get_SceneId()
{
return this->SceneId;
}

RuntimeSceneCatalogEntry::RuntimeSceneCatalogEntry(std::string sceneId, std::string cookedRelativePath) : CookedRelativePath(), SceneId()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_00000135 = "Scene id is required.";
auto __ctor_arg_00000136 = "sceneId";
return new ArgumentException(__ctor_arg_00000135, __ctor_arg_00000136);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_00000137 = "Cooked relative path is required.";
auto __ctor_arg_00000138 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_00000137, __ctor_arg_00000138);
})();
    }
this->SceneId = sceneId;
this->CookedRelativePath = this->NormalizeCookedRelativePath(cookedRelativePath);
}

std::string RuntimeSceneCatalogEntry::NormalizeCookedRelativePath(std::string cookedRelativePath)
{
Array<char> *normalizedCharacters = new Array<char>(static_cast<int32_t>(cookedRelativePath.size()));
for (int32_t index = 0; index < static_cast<int32_t>(cookedRelativePath.size()); index++) {
const char character = cookedRelativePath[index];
(*normalizedCharacters)[index] = character == '\\' ? '/' : character;
}
return std::string(normalizedCharacters->Data, static_cast<size_t>(normalizedCharacters->Length));}


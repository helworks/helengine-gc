#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeContentManagerConfiguration.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "ContentManager.hpp"
#include "RuntimeContentProcessorIds.hpp"
#include "AssetContentProcessor.hpp"
#include "MaterialAsset.hpp"
#include "ModelAsset.hpp"
#include "TextureAsset.hpp"
#include "TextAsset.hpp"
#include "ShaderAsset.hpp"
#include "SceneAsset.hpp"
#include "BinaryContentProcessor.hpp"
#include "FontAsset.hpp"
#include "FontAssetBinarySerializer.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

void RuntimeContentManagerConfiguration::ConfigureSharedAssetContentManager(::ContentManager* contentManager)
{
    if (contentManager == nullptr)
    {
throw new ArgumentNullException("contentManager");
    }
RegisterProcessorIfMissing<MaterialAsset*>(contentManager, RuntimeContentProcessorIds::MaterialAsset, new ::AssetContentProcessor_1<::MaterialAsset*>(), new Array<std::string>({ MaterialAssetExtension }));
RegisterProcessorIfMissing<ModelAsset*>(contentManager, RuntimeContentProcessorIds::ModelAsset, new ::AssetContentProcessor_1<::ModelAsset*>(), nullptr);
RegisterProcessorIfMissing<TextureAsset*>(contentManager, RuntimeContentProcessorIds::TextureAsset, new ::AssetContentProcessor_1<::TextureAsset*>(), nullptr);
RegisterProcessorIfMissing<TextAsset*>(contentManager, RuntimeContentProcessorIds::TextAsset, new ::AssetContentProcessor_1<::TextAsset*>(), nullptr);
RegisterProcessorIfMissing<ShaderAsset*>(contentManager, RuntimeContentProcessorIds::ShaderAsset, new ::AssetContentProcessor_1<::ShaderAsset*>(), new Array<std::string>({ ShaderPackageExtension }));
RegisterProcessorIfMissing<SceneAsset*>(contentManager, RuntimeContentProcessorIds::SceneAsset, new ::AssetContentProcessor_1<::SceneAsset*>(), new Array<std::string>({ SceneAsset::FileExtension }));
RegisterProcessorIfMissing<FontAsset*>(contentManager, RuntimeContentProcessorIds::FontAsset, new ::BinaryContentProcessor_1<::FontAsset*>(new Func<Stream*, FontAsset*>(FontAssetBinarySerializer::Deserialize)), new Array<std::string>({ FontAssetExtension }));
}

std::string RuntimeContentManagerConfiguration::FontAssetExtension = ".hefont";

std::string RuntimeContentManagerConfiguration::MaterialAssetExtension = ".helmat";

std::string RuntimeContentManagerConfiguration::ShaderPackageExtension = ".shader.asset";

template <typename T>
void RuntimeContentManagerConfiguration::RegisterProcessorIfMissing(::ContentManager* contentManager, std::string processorId, ::IContentProcessor_1<T>* processor, Array<std::string>* extensions)
{
    if (contentManager == nullptr)
    {
throw new ArgumentNullException("contentManager");
    }
    if (String::IsNullOrWhiteSpace(processorId))
    {
throw ([&]() {
auto __ctor_arg_00000125 = "Processor id must be provided.";
auto __ctor_arg_00000126 = "processorId";
return new ArgumentException(__ctor_arg_00000125, __ctor_arg_00000126);
})();
    }
    if (processor == nullptr)
    {
throw new ArgumentNullException("processor");
    }
    if (contentManager->IsProcessorRegistered(processorId))
    {
return;    }
contentManager->RegisterProcessor<T>(processorId, processor, extensions);
}


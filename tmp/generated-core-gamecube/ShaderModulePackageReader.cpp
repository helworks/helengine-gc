#ifdef DrawText
#undef DrawText
#endif
#include "ShaderModulePackageReader.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/path.hpp"
#include "ShaderAsset.hpp"
#include "ShaderCompileTarget.hpp"
#include "ShaderTargetNames.hpp"
#include "ShaderModuleDefinition.hpp"
#include "ShaderModulePackage.hpp"
#include "ContentManager.hpp"
#include "AssetContentProcessor.hpp"
#include "system/io/file.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"

::ShaderModulePackage* ShaderModulePackageReader::Read(std::string packagePath)
{
    if (String::IsNullOrWhiteSpace(packagePath))
    {
throw ([&]() {
auto __ctor_arg_000001CC = "Package path must be provided.";
auto __ctor_arg_000001CD = "packagePath";
return new ArgumentException(__ctor_arg_000001CC, __ctor_arg_000001CD);
})();
    }
    if (!File::Exists(packagePath))
    {
throw new FileNotFoundException("Shader package was not found.", packagePath);
    }
const std::string directory = Path::GetDirectoryName(packagePath);
    if (String::IsNullOrWhiteSpace(directory))
    {
throw new InvalidOperationException("Package directory could not be resolved.");
    }
::ShaderAsset *asset = this->ReadAsset(packagePath);
::ShaderCompileTarget target = ShaderTargetNames::ParseTarget(asset->TargetName);
::ShaderModuleDefinition *definition = asset->BuildDefinition();
return new ::ShaderModulePackage(target, directory, definition);}

ShaderModulePackageReader::ShaderModulePackageReader(std::string rootDirectory) : PackageContentManager()
{
    if (String::IsNullOrWhiteSpace(rootDirectory))
    {
throw ([&]() {
auto __ctor_arg_000001CE = "Root directory must be provided.";
auto __ctor_arg_000001CF = "rootDirectory";
return new ArgumentException(__ctor_arg_000001CE, __ctor_arg_000001CF);
})();
    }
this->PackageContentManager = new ::ContentManager(rootDirectory);
this->PackageContentManager->RegisterProcessor<ShaderAsset*>("shader-package", new ::AssetContentProcessor_1<::ShaderAsset*>(), new Array<std::string>({ ".shader.asset" }));
}

::ShaderAsset* ShaderModulePackageReader::ReadAsset(std::string packagePath)
{
return this->PackageContentManager->Load<ShaderAsset*>(packagePath, "shader-package");}


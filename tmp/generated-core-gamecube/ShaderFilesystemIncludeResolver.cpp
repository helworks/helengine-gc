#ifdef DrawText
#undef DrawText
#endif
#include "ShaderFilesystemIncludeResolver.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "TextContent.hpp"
#include "ContentManager.hpp"
#include "ShaderIncludeResult.hpp"
#include "system/io/path.hpp"
#include "system/io/file.hpp"
#include "system/io/directory.hpp"
#include "system/io/file.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"

::ShaderIncludeResult* ShaderFilesystemIncludeResolver::Resolve(std::string requestingFile, std::string includePath)
{
    if (String::IsNullOrWhiteSpace(requestingFile))
    {
throw ([&]() {
auto __ctor_arg_00000275 = "Requesting file must be provided.";
auto __ctor_arg_00000276 = "requestingFile";
return new ArgumentException(__ctor_arg_00000275, __ctor_arg_00000276);
})();
    }
    if (String::IsNullOrWhiteSpace(includePath))
    {
throw ([&]() {
auto __ctor_arg_00000277 = "Include path must be provided.";
auto __ctor_arg_00000278 = "includePath";
return new ArgumentException(__ctor_arg_00000277, __ctor_arg_00000278);
})();
    }
const std::string resolvedPath = this->ResolvePath(requestingFile, includePath);
::TextContent *sourceContent = this->IncludeContentManager->Load<TextContent*>(resolvedPath, nullptr);
return new ::ShaderIncludeResult(resolvedPath, sourceContent->get_Text());}

ShaderFilesystemIncludeResolver::ShaderFilesystemIncludeResolver(std::string rootDirectory) : IncludeContentManager(), rootDirectory()
{
    if (String::IsNullOrWhiteSpace(rootDirectory))
    {
throw ([&]() {
auto __ctor_arg_00000279 = "Root directory must be provided.";
auto __ctor_arg_0000027A = "rootDirectory";
return new ArgumentException(__ctor_arg_00000279, __ctor_arg_0000027A);
})();
    }
    if (!Directory::Exists(rootDirectory))
    {
throw new DirectoryNotFoundException("Include root directory does not exist.");
    }
this->rootDirectory = rootDirectory;
this->IncludeContentManager = new ::ContentManager(rootDirectory);
}

std::string ShaderFilesystemIncludeResolver::ResolvePath(std::string requestingFile, std::string includePath)
{
    if (Path::IsPathRooted(includePath))
    {
    if (!File::Exists(includePath))
    {
throw new FileNotFoundException("Include file does not exist.", includePath);
    }
return includePath;    }
const std::string requestingDirectory = Path::GetDirectoryName(requestingFile);
    if (String::IsNullOrWhiteSpace(requestingDirectory))
    {
throw new InvalidOperationException("Requesting file path does not include a directory.");
    }
const std::string candidate = Path::Combine(requestingDirectory, includePath);
    if (File::Exists(candidate))
    {
return candidate;    }
const std::string rootedCandidate = Path::Combine(this->rootDirectory, includePath);
    if (File::Exists(rootedCandidate))
    {
return rootedCandidate;    }
throw new FileNotFoundException("Include file does not exist.", includePath);
}


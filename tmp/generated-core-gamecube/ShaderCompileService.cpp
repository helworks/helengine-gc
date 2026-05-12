#ifdef DrawText
#undef DrawText
#endif
#include "ShaderCompileService.hpp"
#include "runtime/native_exceptions.hpp"
#include "IShaderBackend.hpp"
#include "ShaderCompileCacheKey.hpp"
#include "ShaderCompileResult.hpp"
#include "IShaderCompileCache.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"
#include "ContentManager.hpp"
#include "TextContent.hpp"
#include "ShaderSourceInfo.hpp"
#include "ShaderCompileRequest.hpp"
#include "runtime/native_dictionary.hpp"
#include "ShaderSourceHasher.hpp"
#include "system/text/string-builder.hpp"
#include "ShaderCompileTarget.hpp"
#include "ShaderDefine.hpp"
#include "system/io/file.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"
#include "system/text/string-builder.hpp"

::ShaderCompileResult* ShaderCompileService::Compile(::ShaderCompileRequest* request)
{
    if (request == nullptr)
    {
throw new ArgumentNullException("request");
    }
::IShaderBackend *backend = this->GetBackend(request->get_Target());
::ShaderCompileCacheKey *cacheKey = this->CreateCacheKey(request);
::ShaderCompileResult *cached;
    if (this->cache->TryGet(cacheKey, cached))
    {
return cached;    }
::ShaderCompileResult *result = backend->Compile(request, this->includeResolver);
this->cache->Store(cacheKey, result);
return result;}

::ShaderCompileResult* ShaderCompileService::CompileFromFile(std::string path, std::string programName, std::string entryPoint, ::ShaderStage stage, ::ShaderCompileTarget target, ::ShaderModel* shaderModel, std::string variant, List<::ShaderDefine*>* defines, ::ShaderCompileOptions* options)
{
    if (String::IsNullOrWhiteSpace(path))
    {
throw ([&]() {
auto __ctor_arg_0000018C = "Shader path must be provided.";
auto __ctor_arg_0000018D = "path";
return new ArgumentException(__ctor_arg_0000018C, __ctor_arg_0000018D);
})();
    }
    if (!File::Exists(path))
    {
throw new FileNotFoundException("Shader source file does not exist.", path);
    }
const std::string directory = Path::GetDirectoryName(Path::GetFullPath(path));
    if (String::IsNullOrWhiteSpace(directory))
    {
throw new InvalidOperationException("Shader source directory could not be resolved.");
    }
::ContentManager *contentManager = new ::ContentManager(directory);
::TextContent *sourceContent = contentManager->Load<TextContent*>(path, nullptr);
const std::string source = sourceContent->get_Text();
::ShaderSourceInfo *sourceInfo = new ::ShaderSourceInfo(path, source);
::ShaderCompileRequest *request = new ::ShaderCompileRequest(sourceInfo, programName, entryPoint, stage, target, shaderModel, variant, defines, options);
return this->Compile(request);}

void ShaderCompileService::RegisterBackend(::IShaderBackend* backend)
{
    if (backend == nullptr)
    {
throw new ArgumentNullException("backend");
    }
(*this->backends)[backend->get_Target()] = backend;
}

ShaderCompileService::ShaderCompileService(::IShaderIncludeResolver* includeResolver, ::IShaderCompileCache* cache, ::ShaderSourceHasher* sourceHasher) : backends(), cache(), includeResolver(), sourceHasher()
{
    if (includeResolver == nullptr)
    {
throw new ArgumentNullException("includeResolver");
    }
    if (cache == nullptr)
    {
throw new ArgumentNullException("cache");
    }
    if (sourceHasher == nullptr)
    {
throw new ArgumentNullException("sourceHasher");
    }
this->includeResolver = includeResolver;
this->cache = cache;
this->sourceHasher = sourceHasher;
this->backends = new Dictionary<::ShaderCompileTarget, ::IShaderBackend*>();
}

std::string ShaderCompileService::BuildBindingPolicySignature(::ShaderBindingPolicy* policy)
{
return String::Concat(std::to_string(policy->get_DefaultSpace()), ":b", std::to_string(policy->get_ConstantBufferShift()), ":t", std::to_string(policy->get_TextureShift()), ":s", std::to_string(policy->get_SamplerShift()), ":u", std::to_string(policy->get_StorageShift()));}

std::string ShaderCompileService::BuildDefinesSignature(List<::ShaderDefine*>* defines)
{
    if (defines->get_Count() == 0)
    {
return String::Empty;    }
StringBuilder *builder = new StringBuilder();
for (int32_t i = 0; i < defines->get_Count(); i++) {
::ShaderDefine *define = (*defines)[i];
builder->Append(define->get_Name());
builder->Append('=');
builder->Append(define->get_Value());
builder->Append(';');
}
return builder->ToString();}

::ShaderCompileCacheKey* ShaderCompileService::CreateCacheKey(::ShaderCompileRequest* request)
{
const std::string sourceHash = this->sourceHasher->ComputeHash(request->get_Source()->get_Source());
const std::string definesSignature = this->BuildDefinesSignature(request->get_Defines());
const std::string bindingSignature = this->BuildBindingPolicySignature(request->get_Options()->get_BindingPolicy());
return new ::ShaderCompileCacheKey(sourceHash, request->get_ProgramName(), request->get_EntryPoint(), request->get_Stage(), request->get_Target(), request->get_ShaderModel(), request->get_Variant(), definesSignature, bindingSignature);}

::IShaderBackend* ShaderCompileService::GetBackend(::ShaderCompileTarget target)
{
::IShaderBackend *backend;
    if (this->backends->TryGetValue(target, backend))
    {
return backend;    }
throw new InvalidOperationException("No shader backend is registered for the requested target.");
}


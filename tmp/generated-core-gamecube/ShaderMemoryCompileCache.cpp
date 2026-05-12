#ifdef DrawText
#undef DrawText
#endif
#include "ShaderMemoryCompileCache.hpp"
#include "runtime/native_exceptions.hpp"
#include "ShaderCompileResult.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/string_comparer.hpp"

ShaderMemoryCompileCache::ShaderMemoryCompileCache() : cache()
{
this->cache = new Dictionary<std::string, ::ShaderCompileResult*>(StringComparer::Ordinal);
}

void ShaderMemoryCompileCache::Store(::ShaderCompileCacheKey* key, ::ShaderCompileResult* result)
{
    if (key == nullptr)
    {
throw new ArgumentNullException("key");
    }
    if (result == nullptr)
    {
throw new ArgumentNullException("result");
    }
(*this->cache)[key->ToString()] = result;
}

bool ShaderMemoryCompileCache::TryGet(::ShaderCompileCacheKey* key, ::ShaderCompileResult*& result)
{
    if (key == nullptr)
    {
throw new ArgumentNullException("key");
    }
return this->cache->TryGetValue(key->ToString(), result);}


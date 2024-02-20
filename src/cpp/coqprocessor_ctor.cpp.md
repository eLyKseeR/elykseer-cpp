declared in [CoqProcessor](coqprocessor.hpp.md)

```cpp

static const int default_cache_size = 3;
static const int default_cache_depth = 12;

CoqProcessor::CoqProcessor(const CoqConfiguration &c)
  : CoqProcessor(c, default_cache_size)
{}

CoqProcessor::CoqProcessor(const CoqConfiguration &c, int cache_size)
  : _config(c)
{
    _cache.reset(new CoqAssemblyCache(_config, cache_size, default_cache_depth));
    // TODO _cache.register_key_store
    // TODO _cache.register_fblock_store
}

CoqProcessor::CoqProcessor(const CoqConfiguration &c, const std::shared_ptr<CoqAssemblyCache> &cache)
  : _config(c), _cache(cache)
{}

CoqProcessor::~CoqProcessor() {
    if (_cache) {
        _cache->close();
        _cache.reset();
    }
}
```

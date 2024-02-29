declared in [CoqProcessor](coqprocessor.hpp.md)

```cpp

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

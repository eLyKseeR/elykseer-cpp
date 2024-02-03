declared in [CoqStore](coqstore.hpp.md)


```coq
    Definition init (c : configuration) : R := {| config := c; entries := [] |}.
```

```cpp
template <typename V, typename K>
CoqStore<V,K>::CoqStore(const CoqConfiguration &c)
  : _config(c)
{}

template <typename V, typename K>
CoqStore<V,K>::~CoqStore() = default;
```

declared in [CoqStore](coqstore.hpp.md)


```coq
    Definition init (c : configuration) : R := {| config := c; entries := [] |}.
```

```cpp
template <typename V, typename K>
CoqStore<V,K>::CoqStore()
{}

template <typename V, typename K>
CoqStore<V,K>::~CoqStore() = default;
```

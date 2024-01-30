declared in [CoqStore](coqstore.hpp.md)

```coq
Fixpoint rec_find {V : Type} (k : string) (es : list (string * V)) : option V :=
    match es with
    | [] => None
    | (k', v')::r => if k' =? k then Some v'
                     else rec_find k r
    end.
```

```cpp
template <typename V, typename K>
int CoqStore<V,K>::size() const
{
    return _entries.size();
}
```


```cpp
template <typename V, typename K>
std::optional<const V> CoqStore<V,K>::find(const K &k) const
{
    if (auto const it = _entries.find(k); it != _entries.cend()) {
        return it->second;
    } else {
        return {};
    }
}
```

```cpp
template <typename V, typename K>
std::optional<std::pair<const K, const V>> CoqStore<V,K>::at(int idx) const
{
    if (idx >= _entries.size()) {
        return {};
    }
    int i = 0;
    for (auto _entry : _entries) {
        if (i++ == idx) {
            return _entry;
        }
    }
    return {};
}

```

```cpp
template <typename V, typename K>
bool CoqStore<V,K>::add(const K &k, const V &v)
{
    _entries[k] = v;
    return true;
}
```

## Template class instances

instantiate template classes with the used types.

```cpp
template class CoqStore<CoqAssembly::BlockInformation, std::string>;
template class CoqStore<CoqAssembly::KeyInformation, std::string>;
```

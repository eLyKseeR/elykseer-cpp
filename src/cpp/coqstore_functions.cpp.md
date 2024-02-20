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

```cpp
template <typename V, typename K>
std::vector<std::pair<K,V>> CoqStore<V,K>::list() const
{
    std::vector<std::pair<K,V>> _res{};
    for (auto const &p : _entries) {
        _res.push_back(p);
    }
    return _res;
}
```

```cpp
template <typename V, typename K>
void CoqStore<V,K>::iterate(std::function<void(const std::pair<K,V> &)> f) const
{
    for (auto const &p : _entries) {
        f(p);
    }
}
```

```cpp
template <typename V, typename K>
void CoqStore<V,K>::reset()
{
    _entries = {};
}
```


## Template class instances

instantiate template classes with the used types.

```cpp
template class CoqStore<CoqAssembly::BlockInformation, std::string>;
template class CoqStore<CoqAssembly::KeyInformation, std::string>;
```

## Output in CSV format encrypted

The entities in the store are output to an encrypted file. The encryption uses a key which is looked up in GnuPG for a specific recipient. Pass in the keys identifier.

```cpp
bool CoqFBlockStore::encrypted_output(const std::filesystem::path &fp, const std::string & gpg_recipient) const
{
    lxr::Gpg gpg;
    if (! gpg.has_public_key(gpg_recipient)) {
        std::clog << "Error: cannot find PGP recipient's encryption key !" << std::endl;
        return false;
    }
    auto &_out = gpg.ostream();
    iterate([&_out](const std::pair<std::string, lxr::CoqAssembly::BlockInformation> &p) {
        _out << "\\"" << p.first << "\\", " << p.second.blockid << ", \\"" << p.second.bchecksum << "\\", " << p.second.blocksize << ", " << p.second.filepos << ", \\"" << p.second.blockaid << "\\", " << p.second.blockapos << std::endl;
    });
    auto _enc = gpg.encrypt_to_key(gpg_recipient);
    if (_enc) {
        std::ofstream _outf; _outf.open(fp.string());
        _outf << _enc.value();
        _outf.close();
        return true;
    } else {
        std::clog << "Error: cannot encrypt FBlockStore !" << std::endl;
    }
    return false;
}
```

```cpp
bool CoqKeyStore::encrypted_output(const std::filesystem::path &fp, const std::string & gpg_recipient) const
{
    lxr::Gpg gpg;
    if (! gpg.has_public_key(gpg_recipient)) {
        std::clog << "Error: cannot find PGP recipient's encryption key !" << std::endl;
        return false;
    }
    auto &_out = gpg.ostream();
    iterate([&_out](const std::pair<std::string, lxr::CoqAssembly::KeyInformation> &p) {
        _out << "\\"" << p.first << "\\", \\"" << p.second._ivec.toHex() << "\\", \\"" << p.second._pkey.toHex() << "\\", \\"" << p.second._localid << "\\", " << p.second._localnchunks << std::endl;
    });
    auto _enc = gpg.encrypt_to_key(gpg_recipient);
    if (_enc) {
        std::ofstream _outf; _outf.open(fp.string());
        _outf << _enc.value();
        _outf.close();
        return true;
    } else {
        std::clog << "Error: cannot encrypt KeyStore !" << std::endl;
    }
    return false;
}
```

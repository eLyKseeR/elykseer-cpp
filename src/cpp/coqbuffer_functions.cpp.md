```coq
```

```cpp

uint32_t CoqBuffer::len() const
{
    if (_pimpl) {
        return _pimpl->len();
    } else {
        return 0;
    }
}

std::optional<const Key256> CoqBuffer::calc_checksum() const
{
    if (_pimpl) {
        return _pimpl->calc_checksum(0, _pimpl->len());
    } else {
        return {};
    }
}

std::optional<const Key256> CoqBuffer::calc_checksum(int offset, int dlen) const
{
    if (_pimpl) {
        return _pimpl->calc_checksum(offset, dlen);
    } else {
        return {};
    }
}

int CoqBuffer::copy_sz_pos(int pos0, int sz, CoqBuffer &target, int pos1) const
{
    if (_pimpl) {
        return _pimpl->copy_sz_pos(pos0, sz, target, pos1);
    } else {
        return -1;
    }
}

int CoqBuffer::fileout_sz_pos(int pos, int sz, FILE *fstr) const
{
    if (_pimpl && fstr) {
        return _pimpl->fileout_sz_pos(pos, sz, fstr);
    } else {
        return -1;
    }
}

int CoqBuffer::filein_sz_pos(int pos, int sz, FILE *fstr)
{
    if (_pimpl && fstr) {
        return _pimpl->filein_sz_pos(pos, sz, fstr);
    } else {
        return -1;
    }
}

int CoqBuffer::to_buffer(int n, char *b) const
{
    if (_pimpl && b) {
        return _pimpl->to_buffer(n, b);
    } else {
        return -1;
    }
}

```

## buffer encryption

```cpp
std::shared_ptr<CoqBufferEncrypted> CoqBufferPlain::encrypt(const Key128 &iv, const Key256 &key)
{
    if (_pimpl) {
        _pimpl->encrypt_buffer(iv, key);
        return std::make_shared<CoqBufferEncrypted>(this, std::move(_pimpl));
    } else {
        return nullptr;
    }
}
```

```cpp
std::shared_ptr<CoqBufferPlain> CoqBufferEncrypted::decrypt(const Key128 &iv, const Key256 &key)
{
    if (_pimpl) {
        _pimpl->decrypt_buffer(iv, key);
        return std::make_shared<CoqBufferPlain>(this, std::move(_pimpl));
    } else {
        return nullptr;
    }
}
```

declared in [CoqBuffer](coqbuffer.hpp.md)

```coq
```

```cpp
struct CoqBuffer::pimpl {
public:
    pimpl(int n) {
        if (n > 0) {
            _buffer = (char*)calloc(n, 1);
            _len = n;
        }
    };
    ~pimpl() {
        if (_buffer) { free(_buffer); }
    }
    uint32_t len() const;
    std::optional<const Key256> calc_checksum() const;
    int copy_sz_pos(int pos0, int sz, CoqBuffer &target, int pos1) const;
    int fileout_sz_pos(int pos, int sz, FILE *fstr) const;
    int filein_sz_pos(int pos, int sz, FILE *fstr);
    int to_buffer(int n, char *b) const;
    char at(uint32_t idx0) const { return _buffer[std::min(_len, idx0)]; }
    void at(uint32_t idx0, char v) { _buffer[std::min(_len, idx0)] = v; }

private:
    uint32_t _len{0};
    char *_buffer{nullptr};
};

```

```cpp

uint32_t CoqBuffer::pimpl::len() const
{
    return _len;
}

std::optional<const Key256> CoqBuffer::pimpl::calc_checksum() const
{
    if (_buffer) {
        return Sha256::hash(_buffer, _len);
    } else {
        return {};
    }
}

int CoqBuffer::pimpl::copy_sz_pos(int pos0, int sz, CoqBuffer &target, int pos1) const
{
    if (sz < 1 || pos0 < 0 || pos1 < 0) { return 0; }
    if (sz + pos0 > _len) { return -1; }
    if (sz + pos1 > target.len()) { return -2; }
    if (target._pimpl && target._pimpl->_buffer) {
        std::memcpy(target._pimpl->_buffer+pos1, _buffer+pos0, sz);
        return sz;
    }
    return 0;
}

int CoqBuffer::pimpl::fileout_sz_pos(int pos, int sz, FILE *fstr) const
{
    if (sz < 1 || pos < 0) { return 0; }
    if (sz + pos > _len) { return -1; }
    fwrite(_buffer+pos, 1, sz, fstr);
    return sz;
}

int CoqBuffer::pimpl::filein_sz_pos(int pos, int sz, FILE *fstr)
{
    if (sz < 1 || pos < 0) { return 0; }
    if (sz + pos > _len) { return -1; }
    return fread(_buffer+pos, 1, sz, fstr);
}

int CoqBuffer::pimpl::to_buffer(int n, char *b) const
{
    if (n > _len) { return -1; }
    std::memcpy(b, _buffer, n);
    return n;
}

```


```cpp

CoqBuffer::CoqBuffer(int n)
  : _pimpl(new pimpl(n))
{
}

CoqBuffer::~CoqBuffer() = default;

```

```cpp
char CoqBuffer::at(uint32_t idx) const {
    return _pimpl->at(idx);
}

void CoqBuffer::at(uint32_t idx, char v) {
    _pimpl->at(idx, v);
}
```

```cpp

CoqBufferPlain::CoqBufferPlain(int n)
  : CoqBuffer(n)
{
}

EncryptionState CoqBufferPlain::state() const { return EncryptionState::Plain; }

```

```cpp

CoqBufferEncrypted::CoqBufferEncrypted(int n)
  : CoqBuffer(n)
{
}

EncryptionState CoqBufferEncrypted::state() const { return EncryptionState::Encrypted; }

```

```cpp

CoqBufferEncrypted&& CoqBufferPlain::encrypt(const std::string &iv, const std::string &key) const
{
    CoqBufferEncrypted *res = new CoqBufferEncrypted(len());

    return std::move(*res);
}

```

```cpp

CoqBufferPlain&& CoqBufferEncrypted::decrypt(const std::string &iv, const std::string &key) const
{
    CoqBufferPlain *res = new CoqBufferPlain(len());

    return std::move(*res);
}

```

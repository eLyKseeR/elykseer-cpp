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
    pimpl(int n, const char *m) : pimpl(n) {
        std::memcpy(_buffer, m, n);
    };
    ~pimpl() {
        if (_buffer) { free(_buffer); }
    }
    uint32_t len() const;
    std::optional<const Key256> calc_checksum(int offset, int dlen) const;
    int copy_sz_pos(int pos0, int sz, CoqBuffer &target, int pos1) const;
    int fileout_sz_pos(int pos, int sz, FILE *fstr) const;
    int filein_sz_pos(int pos, int sz, FILE *fstr);
    int to_buffer(int n, char *b) const;
    char at(uint32_t idx0) const { return _buffer[std::min(_len, idx0)]; }
    void at(uint32_t idx0, char v) { _buffer[std::min(_len, idx0)] = v; }
    void encrypt_buffer(const Key128 &iv, const Key256 &pk);
    void decrypt_buffer(const Key128 &iv, const Key256 &pk);

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

std::optional<const Key256> CoqBuffer::pimpl::calc_checksum(int offset, int dlen) const
{
    if (_buffer && dlen + offset <= _len) {
        return Sha256::hash(_buffer + offset, dlen);
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

void CoqBuffer::pimpl::encrypt_buffer(const Key128 &iv, const Key256 &pk)
{
    constexpr uint32_t datasz{4096};
    AesEncrypt aesenc(pk, iv);
    sizebounded<unsigned char, datasz> buf;
    uint32_t n_encrypted{0};
    int blen, plen;
    while (n_encrypted <= _len) {
        blen = std::min(_len - n_encrypted, datasz);
        // copy blen@n_encrypted data into buf
        std::memcpy((char*)buf.ptr(), _buffer+n_encrypted, blen);
        if (blen < datasz) {
            plen = aesenc.finish(blen, buf);
        } else {
            plen = aesenc.process(blen, buf);
        }
        // copy back plen bytes from buf to buffer@n_encrypted
        std::memcpy(_buffer+n_encrypted, buf.ptr(), plen);
        n_encrypted += blen;
        if (blen == 0) break;
    }
}

void CoqBuffer::pimpl::decrypt_buffer(const Key128 &iv, const Key256 &pk)
{
    constexpr uint32_t datasz{4096};
    AesDecrypt aesdec(pk, iv);
    sizebounded<unsigned char, datasz> buf;
    uint32_t n_encrypted{0};
    int blen, plen;
    while (n_encrypted <= _len) {
        blen = std::min(_len - n_encrypted, datasz);
        // copy blen@n_encrypted data into buf
        std::memcpy((char*)buf.ptr(), _buffer+n_encrypted, blen);
        if (blen < datasz) {
            plen = aesdec.finish(blen, buf);
        } else {
            plen = aesdec.process(blen, buf);
        }
        // copy back plen bytes from buf to buffer@n_encrypted
        std::memcpy(_buffer+n_encrypted, buf.ptr(), plen);
        n_encrypted += blen;
        if (blen == 0) break;
    }
}

```


```cpp

CoqBuffer::CoqBuffer(int n)
  : _pimpl(new pimpl(n))
{}

CoqBuffer::CoqBuffer(int n, const char *m)
  : _pimpl(new pimpl(n,m))
{}

CoqBuffer::CoqBuffer(std::unique_ptr<struct pimpl> &&p)
  : _pimpl(std::move(p))
{}

CoqBuffer::~CoqBuffer()
{
    std::clog << "DTOR CoqBuffer::~CoqBuffer" << std::endl;
    if (_pimpl) {
        _pimpl.reset();
    }
}

```

```cpp
char CoqBuffer::at(uint32_t idx) const {
    return _pimpl->at(idx);
}

void CoqBuffer::at(uint32_t idx, char v) {
    _pimpl->at(idx, v);
}
```

## CoqBufferPlain
```cpp

CoqBufferPlain::CoqBufferPlain(int n)
  : CoqBuffer(n)
{}

CoqBufferPlain::CoqBufferPlain(int n, const char *m)
  : CoqBuffer(n, m)
{}

CoqBufferPlain::CoqBufferPlain(CoqBufferEncrypted *cb, std::unique_ptr<struct pimpl> &&p)
  : CoqBuffer(std::move(p))
{}

CoqBufferPlain::~CoqBufferPlain()
{
    std::clog << "DTOR CoqBufferPlain::~CoqBufferPlain" << std::endl;
}

EncryptionState CoqBufferPlain::state() const { return EncryptionState::Plain; }

```

## CoqBufferEncrypted
```cpp

CoqBufferEncrypted::CoqBufferEncrypted(int n)
  : CoqBuffer(n)
{}

CoqBufferEncrypted::CoqBufferEncrypted(CoqBufferPlain *cb, std::unique_ptr<struct pimpl> &&p)
  : CoqBuffer(std::move(p))
{}

CoqBufferEncrypted::~CoqBufferEncrypted()
{
    std::clog << "DTOR CoqBufferEncrypted::~CoqBufferEncrypted" << std::endl;
}

EncryptionState CoqBufferEncrypted::state() const { return EncryptionState::Encrypted; }

```

module;
/*
    eLyKseeR or LXR - cryptographic data archiving software
    https://github.com/eLyKseeR/elykseer-cpp
    Copyright (C) 2018-2026 Alexander Diemand

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <iostream>
#include <memory>
#include <cstring>
#include <optional>
#include <stdint.h>
#include <string>

#include "sizebounded/sizebounded.hpp"
#include "sizebounded/sizebounded.ipp"

import lxr_sha3;
import lxr_aes_gcm;
import lxr_key96;
import lxr_key128;
import lxr_key256;


module lxr_coqbuffer;


namespace lxr {

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
    bool encrypt_buffer(const Key128 &iv, const Key256 &pk, const std::string &aad);
    bool decrypt_buffer(const Key128 &iv, const Key256 &pk, const std::string &aad);

private:
    pimpl(pimpl const &) = delete;
    pimpl & operator=(pimpl const &) = delete;

    uint32_t _len{0};
    char *_buffer{nullptr};
};

uint32_t CoqBuffer::pimpl::len() const
{
    return _len;
}

std::optional<const Key256> CoqBuffer::pimpl::calc_checksum(int offset, int dlen) const
{
    if (_buffer && dlen + offset <= _len) {
        return Sha3_256::hash(_buffer + offset, dlen);
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

// AES-256-GCM, compatible with elykseer-ml (Cstdio.encrypt/decrypt):
// nonce = the first 96 bits of the ivec, aad = the aid, the tag is in the last tag_len bytes

static Key96 nonce_from_ivec(const Key128 &iv)
{
    Key96 nonce{true};
    nonce.fromBytes(iv.bytes());
    return nonce;
}

bool CoqBuffer::pimpl::encrypt_buffer(const Key128 &iv, const Key256 &pk, const std::string &aad)
{
    if (! _buffer || _len < CoqBuffer::tag_len) { return false; }
    AesGcmEncrypt aesenc(pk, nonce_from_ivec(iv));
    if (! aesenc.aad((unsigned char const*)aad.data(), aad.size())) { return false; }
    sizebounded<unsigned char, AesGcm::datasz> buf;
    const uint32_t dlen = _len - CoqBuffer::tag_len;
    uint32_t n_encrypted{0};
    while (n_encrypted < dlen) {
        const int blen = std::min(dlen - n_encrypted, AesGcm::datasz);
        std::memcpy((char*)buf.ptr(), _buffer+n_encrypted, blen);
        if (aesenc.process(blen, buf) != blen) { return false; }
        std::memcpy(_buffer+n_encrypted, buf.ptr(), blen);
        n_encrypted += blen;
    }
    if (aesenc.finish(0, buf) < 0) { return false; }
    return aesenc.tag((unsigned char*)_buffer + dlen);
}

bool CoqBuffer::pimpl::decrypt_buffer(const Key128 &iv, const Key256 &pk, const std::string &aad)
{
    if (! _buffer || _len < CoqBuffer::tag_len) { return false; }
    AesGcmDecrypt aesdec(pk, nonce_from_ivec(iv));
    const uint32_t dlen = _len - CoqBuffer::tag_len;
    if (! aesdec.aad((unsigned char const*)aad.data(), aad.size())) { return false; }
    if (! aesdec.tag((unsigned char const*)_buffer + dlen)) { return false; }
    sizebounded<unsigned char, AesGcm::datasz> buf;
    uint32_t n_decrypted{0};
    bool ok{true};
    while (ok && n_decrypted < dlen) {
        const int blen = std::min(dlen - n_decrypted, AesGcm::datasz);
        std::memcpy((char*)buf.ptr(), _buffer+n_decrypted, blen);
        if (aesdec.process(blen, buf) != blen) { ok = false; break; }
        std::memcpy(_buffer+n_decrypted, buf.ptr(), blen);
        n_decrypted += blen;
    }
    if (ok && aesdec.finish(0, buf) >= 0) { return true; }
    // the plaintext is not authenticated: discard it
    std::memset(_buffer, 0, _len);
    return false;
}

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
    // std::clog << "DTOR CoqBuffer::~CoqBuffer" << std::endl;
    if (_pimpl) {
        _pimpl.reset();
    }
}

char CoqBuffer::at(uint32_t idx) const {
    return _pimpl->at(idx);
}

void CoqBuffer::at(uint32_t idx, char v) {
    _pimpl->at(idx, v);
}


// CoqBufferPlain

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
    // std::clog << "DTOR CoqBufferPlain::~CoqBufferPlain" << std::endl;
}

EncryptionState CoqBufferPlain::state() const { return EncryptionState::Plain; }

// CoqBufferEncrypted

CoqBufferEncrypted::CoqBufferEncrypted(int n)
  : CoqBuffer(n)
{}

CoqBufferEncrypted::CoqBufferEncrypted(CoqBufferPlain *cb, std::unique_ptr<struct pimpl> &&p)
  : CoqBuffer(std::move(p))
{}

CoqBufferEncrypted::~CoqBufferEncrypted()
{
    // std::clog << "DTOR CoqBufferEncrypted::~CoqBufferEncrypted" << std::endl;
}

EncryptionState CoqBufferEncrypted::state() const { return EncryptionState::Encrypted; }

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

// buffer encryption

std::shared_ptr<CoqBufferEncrypted> CoqBufferPlain::encrypt(const Key128 &iv, const Key256 &key, const std::string &aad)
{
    if (_pimpl && _pimpl->encrypt_buffer(iv, key, aad)) {
        return std::make_shared<CoqBufferEncrypted>(this, std::move(_pimpl));
    } else {
        return nullptr;
    }
}

std::shared_ptr<CoqBufferPlain> CoqBufferEncrypted::decrypt(const Key128 &iv, const Key256 &key, const std::string &aad)
{
    if (_pimpl && _pimpl->decrypt_buffer(iv, key, aad)) {
        return std::make_shared<CoqBufferPlain>(this, std::move(_pimpl));
    } else {
        return nullptr;
    }
}

} // namespace
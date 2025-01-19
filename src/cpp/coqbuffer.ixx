module;
/*
    eLyKseeR or LXR - cryptographic data archiving software
    https://github.com/eLyKseeR/elykseer-cpp
    Copyright (C) 2018-2025 Alexander Diemand

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

#include "lxr/key128.hpp"
#include "lxr/key256.hpp"

#include <memory>
#include <optional>


export module lxr_coqbuffer;


export namespace lxr {

/*

```coq
Module Export Buffer.

Inductive EncryptionState := Plain | Encrypted.

Axiom cstdio_buffer : Type.

Module Type BUF.
  Axiom buffer_t : Type.
  Axiom buffer_create : N -> buffer_t.
  Axiom buffer_len : buffer_t -> N.
  Axiom calc_checksum : buffer_t -> string.
  Axiom copy_sz_pos : buffer_t -> N -> N -> buffer_t -> N -> N.
  Axiom from_buffer : cstdio_buffer -> buffer_t.
  Axiom to_buffer : buffer_t -> cstdio_buffer.

  Parameter state : EncryptionState.
End BUF.

Module Export BufferEncrypted : BUF.
  Axiom buffer_t : Type.
  Axiom buffer_create : N -> buffer_t.
  Axiom buffer_len : buffer_t -> N.
  Axiom calc_checksum : buffer_t -> string.
  Axiom copy_sz_pos : buffer_t -> N -> N -> buffer_t -> N -> N.
  Axiom from_buffer : cstdio_buffer -> buffer_t.
  Axiom to_buffer : buffer_t -> cstdio_buffer.

  Definition state := Encrypted.
End BufferEncrypted.

Module Export BufferPlain : BUF.
  Axiom buffer_t : Type.
  Axiom buffer_create : N -> buffer_t.
  Axiom buffer_len : buffer_t -> N.
  Axiom calc_checksum : buffer_t -> string.
  Axiom copy_sz_pos : buffer_t -> N -> N -> buffer_t -> N -> N.
  Axiom from_buffer : cstdio_buffer -> buffer_t.
  Axiom to_buffer : buffer_t -> cstdio_buffer.

  Definition state := Plain.
End BufferPlain.

Axiom cpp_encrypt_buffer : BufferPlain.buffer_t -> string -> string -> BufferEncrypted.buffer_t.
Definition encrypt (bin : BufferPlain.buffer_t) (iv : string) (pw : string) : BufferEncrypted.buffer_t :=
  cpp_encrypt_buffer bin iv pw.
Axiom cpp_decrypt_buffer : BufferEncrypted.buffer_t -> string -> string -> BufferPlain.buffer_t.
Definition decrypt (bin : BufferEncrypted.buffer_t) (iv : string) (pw : string) : BufferPlain.buffer_t :=
  cpp_decrypt_buffer bin iv pw.

Axiom cpp_ranbuf128 : unit -> cstdio_buffer.
Program Definition ranbuf128 (_ : unit) : BufferPlain.buffer_t :=
  let rb := cpp_ranbuf128 tt in
  BufferPlain.from_buffer rb.

End Buffer.
```

*/

enum class EncryptionState
{
  Plain,
  Encrypted
};


class CoqBuffer
{
  protected:
    struct pimpl;
    explicit CoqBuffer(int n);
    explicit CoqBuffer(int n, const char *b);
    explicit CoqBuffer(std::unique_ptr<struct CoqBuffer::pimpl> &&);
    ~CoqBuffer();
  public:
    virtual EncryptionState state() const = 0;
    uint32_t len() const;
    std::optional<const Key256> calc_checksum() const;
    std::optional<const Key256> calc_checksum(int offset, int dlen) const;
    int copy_sz_pos(int pos0, int sz, CoqBuffer & target, int pos1) const;
    int fileout_sz_pos(int pos, int sz, FILE *fstr) const;
    int filein_sz_pos(int pos, int sz, FILE *fstr);
    int to_buffer(int n, char *b) const;
    char at(uint32_t idx) const;
    void at(uint32_t idx, char v);
  protected:
    std::unique_ptr<struct pimpl> _pimpl;
};


class CoqBufferEncrypted;

class CoqBufferPlain : public CoqBuffer
{
  public:
    explicit CoqBufferPlain(int n);
    explicit CoqBufferPlain(int n, const char *b);
    explicit CoqBufferPlain(CoqBufferEncrypted *, std::unique_ptr<struct pimpl> &&p);
    ~CoqBufferPlain();
    virtual EncryptionState state() const override final;
    std::shared_ptr<CoqBufferEncrypted> encrypt(const Key128 &iv, const Key256 &key);
};


class CoqBufferEncrypted : public CoqBuffer
{
  public:
    explicit CoqBufferEncrypted(int n);
    explicit CoqBufferEncrypted(CoqBufferPlain *, std::unique_ptr<struct pimpl> &&p);
    ~CoqBufferEncrypted();
    virtual EncryptionState state() const override final;
    std::shared_ptr<CoqBufferPlain> decrypt(const Key128 &iv, const Key256 &key);
  protected:
    explicit CoqBufferEncrypted(int n, const char *b);
};

} // namespace
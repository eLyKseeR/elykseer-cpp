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

#include <stdio.h>
#include <memory>
#include <optional>
#include <string>

import lxr_key256;
import lxr_key128;


export module lxr_coqbuffer;


export namespace lxr {

/*

from elykseer-ml @ e6db580 (0.10.1), theories/Cstdio.v (excerpt):

```coq
Module Export Cstdio.

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

(* AES-256-GCM authentication tag, stored in the last tag_len bytes of an assembly (physical
   position, i.e. the end of the last chunk) *)
Definition tag_len : N := 16.

(* AES-256-GCM: the nonce is derived from the ivec, the aad is authenticated but not encrypted *)
Axiom cpp_encrypt_buffer : BufferPlain.buffer_t -> N -> string -> string -> string -> option (N * BufferEncrypted.buffer_t).

Definition encrypt (bin : BufferPlain.buffer_t) (aad : string) (iv : string) (pw : string) : option (N * BufferEncrypted.buffer_t) :=
  let blen := BufferPlain.buffer_len bin in
  (* the data length is the buffer length minus the tag *)
  cpp_encrypt_buffer bin (blen - tag_len) aad iv pw.
(* returns None if the authentication tag does not verify *)
Axiom cpp_decrypt_buffer : BufferEncrypted.buffer_t -> string -> string -> string -> option (N * BufferPlain.buffer_t).

Definition decrypt (bin : BufferEncrypted.buffer_t) (aad : string) (iv : string) (pw : string) : option (N * BufferPlain.buffer_t) :=
  cpp_decrypt_buffer bin aad iv pw.

Axiom cpp_ranbuf128 : unit -> cstdio_buffer.
Program Definition ranbuf128 (_ : unit) : BufferPlain.buffer_t :=
  let rb := cpp_ranbuf128 tt in
  BufferPlain.from_buffer rb.

End Cstdio.
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
    CoqBuffer(CoqBuffer const &) = delete;
    CoqBuffer & operator=(CoqBuffer const &) = delete;
  public:
    // AES-256-GCM authentication tag in the last tag_len bytes of the buffer
    static constexpr uint32_t tag_len = 16;
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
    // AES-256-GCM over the first len() - tag_len bytes, the tag is written to the last tag_len bytes;
    // nonce = first 96 bits of iv, aad = additional authenticated data (the aid);
    // returns nullptr on failure
    std::shared_ptr<CoqBufferEncrypted> encrypt(const Key128 &iv, const Key256 &key, const std::string &aad);
};


class CoqBufferEncrypted : public CoqBuffer
{
  public:
    explicit CoqBufferEncrypted(int n);
    explicit CoqBufferEncrypted(CoqBufferPlain *, std::unique_ptr<struct pimpl> &&p);
    ~CoqBufferEncrypted();
    virtual EncryptionState state() const override final;
    // returns nullptr if the key, iv or aad do not match, or the data was modified
    std::shared_ptr<CoqBufferPlain> decrypt(const Key128 &iv, const Key256 &key, const std::string &aad);
  protected:
    explicit CoqBufferEncrypted(int n, const char *b);
};

} // namespace
```cpp

/*
    eLyKseeR or LXR - cryptographic data archiving software
    https://github.com/eLyKseeR/elykseer-cpp
    Copyright (C) 2023 Alexander Diemand

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

#pragma once

#include "lxr/key256.hpp"

#include <memory>

````

namespace [lxr](namespace.list) {

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

# enum class EncryptionState

{

  Plain,

  Encrypted

};


# class CoqBuffer

{

>protected:

>explicit [CoqBuffer](coqbuffer_ctor.cpp.md)(int n);

>explicit [CoqBuffer](coqbuffer_ctor.cpp.md)(int n, const char *b);

>[~CoqBuffer](coqbuffer_ctor.cpp.md)();

>public:

>virtual EncryptionState [state](coqbuffer_functions.cpp.md)() const = 0;

>uint32_t [len](coqbuffer_functions.cpp.md)() const;

>std::optional&lt;const Key256&gt; [calc_checksum](coqbuffer_functions.cpp.md)() const;

>int [copy_sz_pos](coqbuffer_functions.cpp.md)(int pos0, int sz, CoqBuffer & target, int pos1) const;

>int [fileout_sz_pos](coqbuffer_functions.cpp.md)(int pos, int sz, FILE *fstr) const;

>int [filein_sz_pos](coqbuffer_functions.cpp.md)(int pos, int sz, FILE *fstr);

>int [to_buffer](coqbuffer_functions.cpp.md)(int n, char *b) const;

>char [at](coqbuffer_functions.cpp.md)(uint32_t idx) const;

>void [at](coqbuffer_functions.cpp.md)(uint32_t idx, char v);

>protected:

>EncryptionState _state;

>private:

>struct pimpl;

>std::unique_ptr&lt;struct pimpl&gt; _pimpl;

>CoqBuffer(CoqBuffer const &) = delete;

>CoqBuffer & operator=(CoqBuffer const &) = delete;

};


class CoqBufferEncrypted;

# class CoqBufferPlain : public CoqBuffer

{

>public:

>explicit [CoqBufferPlain](coqbuffer_ctor.cpp.md)(int n);

>explicit [CoqBufferPlain](coqbuffer_ctor.cpp.md)(int n, const char *b);

>virtual EncryptionState [state](coqbuffer_ctor.cpp.md)() const override final;

>CoqBufferEncrypted&& [encrypt](coqbuffer_functions.cpp.md)(const std::string &iv, const std::string &key) const;

};


# class CoqBufferEncrypted : public CoqBuffer

{

>public:

>explicit [CoqBufferEncrypted](coqbuffer_ctor.cpp.md)(int n);

>explicit [CoqBufferEncrypted](coqbuffer_ctor.cpp.md)(int n, const char *b);

>virtual EncryptionState [state](coqbuffer_ctor.cpp.md)() const override final;

>CoqBufferPlain&& [decrypt](coqbuffer_functions.cpp.md)(const std::string &iv, const std::string &key) const;

};


```cpp
} // namespace
```

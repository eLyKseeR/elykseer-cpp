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
```

`#pragma once`

`#include <filesystem>`

`#include <memory>`

`#include <optional>`

`#include <string>`

`#include` "[lxr/coqbuffer.hpp](coqbuffer.hpp.md)"

`#include` "[lxr/coqconfiguration.hpp](coqconfiguration.hpp.md)"

`#include "lxr/key256.hpp"`

`#include "lxr/key128.hpp"`


namespace [lxr](namespace.list) {

/*

```coq
Module Export Assembly.

Definition aid_t := string.

Definition RecordAssemblyInformation := Set.
Record assemblyinformation : RecordAssemblyInformation :=
    mkassembly
        { nchunks : Nchunks.Private.t
        ; aid : aid_t
        ; apos : N }.

Definition RecordKeyInformation := Set.
Record keyinformation : RecordKeyInformation :=
    mkkeyinformation
        { ivec : string
        ; pkey : string
        ; localid : string
        ; localnchunks : positive
        }.

Definition RecordBlockInformation := Set.
Record blockinformation : RecordBlockInformation :=
    mkblockinformation
        { blockid : positive
        ; bchecksum : string
        ; blocksize : N
        ; filepos : N
        ; blockaid : aid_t
        ; blockapos : N
        }.

Module Type ASS.
    (* Parameter H : RecordAssemblyInformation. *)
    Parameter B : Type.
    Axiom create : configuration -> (assemblyinformation * B).
    Axiom buffer_len : B -> N.
    Axiom calc_checksum : B -> string.
End ASS.

Module AssemblyPlainWritable : ASS.
    (* Definition H : RecordAssemblyInformation := assemblyinformation. *)
    Definition B := BufferPlain.buffer_t.
    Definition buffer_len : B -> N := BufferPlain.buffer_len.
    Definition calc_checksum : B -> string := fun _ => "<>".
    Definition create (c : configuration) : assemblyinformation * B :=
        let chunks := config_nchunks c in
        let b := BufferPlain.buffer_create (chunksize_N * Nchunks.to_N chunks) in
        let rb := Buffer.ranbuf128 tt in
        let nb := BufferPlain.copy_sz_pos rb 0 16 b 0 in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) nb, b).
End AssemblyPlainWritable.

Module AssemblyEncrypted : ASS.
    (* Definition H : RecordAssemblyInformation := assemblyinformation. *)
    Definition B := BufferEncrypted.buffer_t.
    Definition buffer_len : B -> N := BufferEncrypted.buffer_len.
    Definition calc_checksum : B -> string := BufferEncrypted.calc_checksum.
    Definition create (c : configuration) : assemblyinformation * B :=
        let chunks := config_nchunks c in
        let b := BufferEncrypted.buffer_create (chunksize_N * Nchunks.to_N chunks) in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) 0, b).
End AssemblyEncrypted.

Module AssemblyPlainFull : ASS.
    (* Definition H : RecordAssemblyInformation := assemblyinformation. *)
    Definition B := BufferPlain.buffer_t.
    Definition buffer_len : B -> N := BufferPlain.buffer_len.
    Definition calc_checksum : B -> string := BufferPlain.calc_checksum.
    Definition create (c : configuration) : assemblyinformation * B :=
        let chunks := config_nchunks c in
        let sz := chunksize_N * Nchunks.to_N chunks in
        let b := BufferPlain.buffer_create sz in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) sz, b).
End AssemblyPlainFull.

Section Code_Writeable.

Axiom id_assembly_full_ainfo_from_writable : assemblyinformation -> assemblyinformation.
Axiom id_assembly_full_buffer_from_writable : AssemblyPlainWritable.B -> AssemblyPlainFull.B.
Program Definition finish (a : assemblyinformation) (b : AssemblyPlainWritable.B) : (assemblyinformation * AssemblyPlainFull.B) :=

Axiom assembly_add_content : BufferPlain.buffer_t -> N -> N -> AssemblyPlainWritable.B -> N.
Program Definition backup (a : assemblyinformation) (b : AssemblyPlainWritable.B) (fpos : N) (content : BufferPlain.buffer_t) : (assemblyinformation * blockinformation) :=

End Code_Writeable.

Section Code_Readable.

Axiom id_buffer_t_from_full : AssemblyPlainFull.B -> BufferPlain.buffer_t.
(* Axiom id_buffer_t_from_writable : AssemblyPlainWritable.B -> BufferPlain.buffer_t. *)
Axiom id_assembly_enc_buffer_t_from_buf : BufferEncrypted.buffer_t -> AssemblyEncrypted.B.
Program Definition encrypt (a : assemblyinformation) (b : AssemblyPlainFull.B) (ki : keyinformation) : option (assemblyinformation * AssemblyEncrypted.B) :=

Axiom assembly_get_content : AssemblyPlainFull.B -> N -> N -> BufferPlain.buffer_t -> N.
Program Definition restore (b : AssemblyPlainFull.B) (bi : blockinformation) : option BufferPlain.buffer_t :=

End Code_Readable.

Section Code_Encrypted.

Axiom id_buffer_t_from_enc : AssemblyEncrypted.B -> BufferEncrypted.buffer_t.
Axiom id_assembly_plain_buffer_t_from_buf : BufferPlain.buffer_t -> AssemblyPlainFull.B.
Program Definition decrypt (a : assemblyinformation) (b : AssemblyEncrypted.B) (ki : keyinformation) : option (assemblyinformation * AssemblyPlainFull.B) :=

Axiom chunk_identifier : configuration -> aid_t -> positive -> string.
Axiom chunk_identifier_path : configuration -> aid_t -> positive -> string.
Axiom ext_load_chunk_from_path : string -> option BufferEncrypted.buffer_t.
Axiom id_enc_from_buffer_t : BufferEncrypted.buffer_t -> AssemblyEncrypted.B.
Program Definition recall (c : configuration) (a : assemblyinformation) : option (assemblyinformation * AssemblyEncrypted.B) :=

Axiom ext_store_chunk_to_path : string -> N -> N -> BufferEncrypted.buffer_t -> N.
Program Definition extract (c : configuration) (a : assemblyinformation) (b : AssemblyEncrypted.B) : N :=

End Code_Encrypted.
End Assembly.

```

*/


# class CoqAssembly

{

>public:

>typedef std::string aid_t;

>static constexpr uint32_t chunkwidth = 256;

>static constexpr uint32_t chunklength = 1024;

>static constexpr uint32_t chunksize = chunkwidth * chunklength;

>static const uint32_t assemblysize (int nchunks) { return chunksize * nchunks; }


## struct AssemblyInformation

{

>AssemblyInformation();

>AssemblyInformation(const [CoqConfiguration](coqconfiguration.hpp) & c);

>AssemblyInformation(const [CoqConfiguration](coqconfiguration.hpp) & c, const std::string & aid);

>uint16_t _nchunks{16};

>uint32_t _apos{0};

>aid_t _aid{""};

};

## struct BlockInformation

{

>int blockid;

>Key256 bchecksum{true};

>uint32_t blocksize;

>uint64_t filepos;

>aid_t blockaid;

>uint32_t blockapos;

};

## struct KeyInformation

{

>KeyInformation() {};

>KeyInformation(const [CoqConfiguration](coqconfiguration.hpp) & c);

>Key128 _ivec{false};

>Key256 _pkey{false};

>std::string _localid;

>int _localnchunks;

};

>protected:

>explicit [CoqAssembly](coqassembly_ctor.cpp.md)(const [CoqConfiguration](coqconfiguration.hpp) & c);

>explicit [CoqAssembly](coqassembly_ctor.cpp.md)(CoqAssembly*);

>[CoqAssembly](coqassembly_ctor.cpp.md)();

>virtual [~CoqAssembly](coqassembly_ctor.cpp.md)();

>virtual void [mk_assembly_id](coqassembly_functions.cpp.md)() final;

>virtual int [buffer_len](coqassembly_functions.cpp.md)() const = 0;

>virtual std::optional&lt;const Key256&gt; [calc_checksum](coqassembly_functions.cpp.md)() const = 0;

>virtual std::optional&lt;const std::filesystem::path&gt; [chunk_path](coqassembly_functions.cpp.md)(const uint16_t cnum, const aid_t aid) const final;

>public:

>virtual int [apos](coqassembly_functions.cpp.md)() const final;

>virtual int [afree](coqassembly_functions.cpp.md)() const final;

>virtual std::string [aid](coqassembly_functions.cpp.md)() const final;

>inline const uint32_t idx2apos (const uint32_t idx, const uint32_t nchunks) const {
    uint32_t cnum = idx % nchunks;
    uint32_t cidx = idx / nchunks;
    return cnum * CoqAssembly::chunksize + cidx;
}


>protected:

>const [CoqConfiguration](coqconfiguration.hpp) _config;

>struct AssemblyInformation _assemblyinformation;


>private:

>CoqAssembly(CoqAssembly const &) = delete;

>CoqAssembly & operator=(CoqAssembly const &) = delete;

};


class CoqAssemblyPlainFull;

# class CoqAssemblyPlainWritable : public CoqAssembly

{

>public:

>explicit [CoqAssemblyPlainWritable](coqassembly_ctor.cpp.md)(const [CoqConfiguration](coqconfiguration.hpp) & c);

>virtual [~CoqAssemblyPlainWritable](coqassembly_ctor.cpp.md)();

>virtual int [buffer_len](coqassembly_functions.cpp.md)() const override final;

>virtual std::optional&lt;const Key256&gt; [calc_checksum](coqassembly_functions.cpp.md)() const override final;

>virtual std::shared_ptr&lt;CoqAssemblyPlainFull&gt; [finish](coqassembly_functions.cpp.md)() final;

>virtual CoqAssembly::BlockInformation [backup](coqassembly_functions.cpp.md)(const CoqBufferPlain &b) final;

>virtual CoqAssembly::BlockInformation [backup](coqassembly_functions.cpp.md)(const CoqBufferPlain &b, const uint32_t offset, const uint32_t dlen) final;

>private:

>std::shared_ptr&lt;[CoqBufferPlain](coqbuffer.hpp.md#class-coqbufferplain--public-coqbuffer)&gt; _buffer{nullptr};

};


class CoqAssemblyEncrypted;

# class CoqAssemblyPlainFull : public CoqAssembly

{

>public:

>explicit [CoqAssemblyPlainFull](coqassembly_ctor.cpp.md)(const [CoqConfiguration](coqconfiguration.hpp) & c);

>explicit [CoqAssemblyPlainFull](coqassembly_ctor.cpp.md)(CoqAssemblyEncrypted *, std::shared_ptr&lt;CoqBufferPlain&gt; &b);

>explicit [CoqAssemblyPlainFull](coqassembly_ctor.cpp.md)(CoqAssemblyPlainWritable *, std::shared_ptr&lt;CoqBufferPlain&gt; &b);

>virtual [~CoqAssemblyPlainFull](coqassembly_ctor.cpp.md)();

>virtual int [buffer_len](coqassembly_functions.cpp.md)() const override final;

>virtual std::optional&lt;const Key256&gt; [calc_checksum](coqassembly_functions.cpp.md)() const override final;

>virtual std::shared_ptr&lt;CoqAssemblyEncrypted&gt; [encrypt](coqassembly_functions.cpp.md)(const CoqAssembly::KeyInformation & ki) final;

>virtual std::shared_ptr&lt;CoqBufferPlain&gt; [restore](coqassembly_functions.cpp.md)(const CoqAssembly::BlockInformation & bi) const final;

>private:

>std::shared_ptr&lt;[CoqBufferPlain](coqbuffer.hpp.md#class-coqbufferplain--public-coqbuffer)&gt; _buffer{nullptr};

};


# class CoqAssemblyEncrypted : public CoqAssembly

{

>public:

>explicit [CoqAssemblyEncrypted](coqassembly_ctor.cpp.md)(const [CoqConfiguration](coqconfiguration.hpp) & c);

>explicit [CoqAssemblyEncrypted](coqassembly_ctor.cpp.md)(CoqAssemblyPlainFull*, std::shared_ptr&lt;CoqBufferEncrypted&gt; &b);

>virtual [~CoqAssemblyEncrypted](coqassembly_ctor.cpp.md)();

>virtual int [buffer_len](coqassembly_functions.cpp.md)() const override final;

>virtual std::optional&lt;const Key256&gt; [calc_checksum](coqassembly_functions.cpp.md)() const override final;

>virtual std::shared_ptr&lt;CoqAssemblyPlainFull&gt; [decrypt](coqassembly_functions.cpp.md)(const CoqAssembly::KeyInformation & ki) final;

>virtual uint32_t [extract](coqassembly_functions.cpp.md)() final;

>static std::shared_ptr&lt;CoqAssemblyEncrypted&gt; [recall](coqassembly_functions.cpp.md)(const [CoqConfiguration](coqconfiguration.hpp) & c, const CoqAssembly::aid_t & aid);

>private:

>std::shared_ptr&lt;[CoqBufferEncrypted](coqbuffer.hpp.md#class-coqbufferencrypted--public-coqbuffer)&gt; _buffer{nullptr};

};

```cpp
} // namespace
```

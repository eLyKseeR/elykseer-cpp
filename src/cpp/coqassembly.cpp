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
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <cstdint>

import lxr_key128;
import lxr_key256;
import lxr_sha3;

import lxr_coqbuffer;
import lxr_coqconfiguration;
import lxr_nchunks;


module lxr_coqassembly;


namespace lxr {

CoqAssembly::KeyInformation::KeyInformation(const CoqConfiguration & c)
  : _localnchunks(c.nchunks())
{}

CoqAssembly::AssemblyInformation::AssemblyInformation()
{}

CoqAssembly::AssemblyInformation::AssemblyInformation(const CoqConfiguration & c)
  : _nchunks(c.nchunks())
{}

CoqAssembly::AssemblyInformation::AssemblyInformation(const CoqConfiguration & c, const std::string & aid)
  : _nchunks(c.nchunks()), _aid(aid)
{}

CoqAssembly::CoqAssembly(const CoqConfiguration &c)
  : _config(c)
{
  // std::clog << "CoqAssembly::CoqAssembly(const CoqConfiguration &c)" << std::endl;
}

CoqAssembly::CoqAssembly(CoqAssembly *ca)
  : _config(ca->_config), _assemblyinformation(ca->_assemblyinformation)
{
  // std::clog << "CoqAssembly::CoqAssembly(CoqAssembly *ca)" << std::endl;
}

CoqAssembly::CoqAssembly()
{
  // std::clog << "CoqAssembly::CoqAssembly()" << std::endl;
}

CoqAssembly::~CoqAssembly() = default;

// create a distinct assembly id (256 bits)

/*
```coq
Definition mkaid (c : configuration) : aid_t :=
    Utilities.rnd256 (my_id c).
```
extracted to (theories/MakeML.v):
```ocaml
Extract Constant rnd256 =>
   "
   function
   x -> Elykseer_crypto.Key256.mk () |> Elykseer_crypto.Key256.to_hex |>
     String.cat x |>
     Elykseer_crypto.Sha3_256.string
   ".
```
*/

CoqAssembly::aid_t CoqAssembly::mk_aid(const std::string & my_id, const std::string & rnd_hex)
{
  return Sha3_256::hash(my_id + rnd_hex).toHex();
}

void CoqAssembly::mk_assembly_id()
{
  const Key256 rnd{false};
  _assemblyinformation._aid = mk_aid(_config.my_id, rnd.toHex());
}

// CoqAssemblyPlainWritable

/*
```coq
    Definition create (c : configuration) : assemblyinformation * B :=
        let chunks := config_nchunks c in
        let b := BufferPlain.buffer_create (chunksize_N * Nchunks.to_N chunks) in
        let rb := Cstdio.ranbuf128 tt in
        (* header bytes: n_chunks - tag_len, at most the 128 random bytes available;
           the tail of tag_len bytes holds the GCM tag *)
        let nb := BufferPlain.copy_sz_pos rb 0 (N.min (Nchunks.to_N chunks - Cstdio.tag_len) 128) b 0 in
        (mkassembly chunks (mkaid c) nb, b).
```
*/

CoqAssemblyPlainWritable::CoqAssemblyPlainWritable(const CoqConfiguration &c)
  : CoqAssemblyPlainWritable(c, "")
{
  mk_assembly_id();
}

CoqAssemblyPlainWritable::CoqAssemblyPlainWritable(const CoqConfiguration &c, const aid_t &aid)
  : CoqAssembly(c)
{
  const Nchunks& nchunks = c.nchunks();
  _buffer.reset(new CoqBufferPlain(assemblysize(nchunks)));
  // the random header is copied to the physical positions [0, h), like BufferPlain.copy_sz_pos
  const uint32_t h = header_len(nchunks);
  for (uint32_t idx = 0; idx < h; idx += 32) {
    const Key256 _rand256{false};
    const char *r = (const char*)_rand256.bytes();
    for (uint32_t i = 0; i < 32 && idx + i < h; i++) {
        _buffer->at(idx + i, r[i]);
    }
  }
  _assemblyinformation._apos = h;
  _assemblyinformation._nchunks = nchunks;
  _assemblyinformation._aid = aid;
}

CoqAssemblyPlainWritable::~CoqAssemblyPlainWritable() = default;

/*
```coq
    Definition create (c : configuration) : assemblyinformation * B :=
        let chunks := config_nchunks c in
        let sz := chunksize_N * Nchunks.to_N chunks in
        let b := BufferPlain.buffer_create sz in
        (mkassembly chunks (mkaid c) sz, b).
```
*/

// CoqAssemblyPlainFull

CoqAssemblyPlainFull::CoqAssemblyPlainFull(const CoqConfiguration &c)
  : CoqAssembly(c)
{
  // std::clog << "CoqAssemblyPlainFull::CoqAssemblyPlainFull(const CoqConfiguration &c)" << std::endl;
  _buffer.reset(new CoqBufferPlain(assemblysize(c.nchunks())));
}

CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyEncrypted *ca, std::shared_ptr<CoqBufferPlain> &b)
  : CoqAssembly(ca)
{
  // std::clog << "CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyEncrypted *ca, std::shared_ptr<CoqBufferPlain> &b)" << std::endl;
  _buffer = b;
}

CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyPlainWritable *ca, std::shared_ptr<CoqBufferPlain> &b)
  : CoqAssembly(ca)
{
  // std::clog << "CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyPlainWritable *ca, std::shared_ptr<CoqBufferPlain> &b)" << std::endl;
  _buffer = b;
}

CoqAssemblyPlainFull::~CoqAssemblyPlainFull() = default;


/*
```coq
    Definition create (c : configuration) : assemblyinformation * B :=
        let chunks := config_nchunks c in
        let b := BufferEncrypted.buffer_create (chunksize_N * Nchunks.to_N chunks) in
        (mkassembly chunks (mkaid c) 0, b).
```
*/

// CoqAssemblyEncrypted

CoqAssemblyEncrypted::CoqAssemblyEncrypted(const CoqConfiguration &c)
  : CoqAssembly(c)
{
  // std::clog << "CoqAssemblyEncrypted::CoqAssemblyEncrypted(const CoqConfiguration &c)" << std::endl;
  _buffer.reset(new CoqBufferEncrypted(assemblysize(c.nchunks())));
}

CoqAssemblyEncrypted::CoqAssemblyEncrypted(CoqAssemblyPlainFull *ca, std::shared_ptr<CoqBufferEncrypted> &b)
  : CoqAssembly(ca)
{
  // std::clog << "CoqAssemblyEncrypted::CoqAssemblyEncrypted(CoqAssemblyPlainFull *ca, std::shared_ptr<CoqBufferEncrypted> &b)" << std::endl;
  _buffer = std::move(std::reinterpret_pointer_cast<CoqBufferEncrypted>(b));
}

CoqAssemblyEncrypted::~CoqAssemblyEncrypted() = default;

// compute file path of a chunk

/*
```ocaml
Extract Constant chunk_identifier =>
   "fun config aid cid ->
      let s = (Configuration.my_id config) ^
              (string_of_int (Conversion.p2i cid)) ^
              aid in
      Elykseer_crypto.Sha3_256.string s
   ".

Extract Constant chunk_identifier_path =>
   "fun config aid cid -> let cident = chunk_identifier config aid cid in
      let subd = Helper.mk_cid_subdir cident in
      (Configuration.path_chunks config ^ ""/"" ^ subd ^ ""/"" ^ cident ^ "".lxr"")
   ".
```
*/

std::string CoqAssembly::chunk_identifier(const std::string & my_id, const aid_t & aid, const uint16_t cid)
{
    return Sha3_256::hash(my_id + std::to_string(cid) + aid).toHex();
}

std::optional<const std::filesystem::path> CoqAssembly::chunk_path(const uint16_t cnum, const aid_t aid) const {
    const auto h = chunk_identifier(_config.my_id, aid, cnum);
    const int len = h.length();
    if (len != 64) {
        std::clog << "chunk_path |sha3_256| != 64 chars" << std::endl;
        return {};
    }
    // <chunkpath>/<c63c64>/<c61c62>/<chunk id>.lxr
    std::filesystem::path fp = _config.path_chunks;
    fp /= h.substr(len-2, 2);
    fp /= h.substr(len-4, 2);
    std::error_code ec;
    std::filesystem::create_directories(fp, ec);
    if (ec) {
        std::clog << "cannot create chunk directory " << fp.string() << ": " << ec.message() << std::endl;
        return {};
    }
    (fp /= h) . replace_extension("lxr");
    return fp;
}

int CoqAssemblyPlainFull::buffer_len() const
{
    if (_buffer) {
        return _buffer->len();
    }
    return 0;
}

int CoqAssemblyPlainWritable::buffer_len() const
{
    if (_buffer) {
        return _buffer->len();
    }
    return 0;
}

int CoqAssemblyEncrypted::buffer_len() const
{
    if (_buffer) {
        return _buffer->len();
    }
    return 0;
}

int CoqAssembly::apos() const
{
    return _assemblyinformation._apos;
}

/*
```coq
        (* data is striped over the chunks: logical position i is stored in chunk (i mod n) at
           offset (i / n); the GCM tag occupies the last tag_len bytes of the last chunk, hence
           the last tag_len rows (tag_len * n logical bytes) are not available for data *)
        let nch := Nchunks.to_N e0.(econfig AB).(Configuration.config_nchunks) in
        let afree := (Assembly.assemblysize e0.(econfig AB).(Configuration.config_nchunks)) - e0.(cur_assembly AB).(apos) - Cstdio.tag_len * nch in
```
*/
int CoqAssembly::afree() const
{
    const int free = (int)assemblysize(_assemblyinformation._nchunks) - (int)_assemblyinformation._apos - (int)(tag_len * _assemblyinformation._nchunks.u());
    return std::max(0, free);
}

std::string CoqAssembly::aid() const
{
    return _assemblyinformation._aid;
}

std::optional<const Key256> CoqAssemblyPlainWritable::calc_checksum() const
{
    return {};
}

std::optional<const Key256> CoqAssemblyPlainFull::calc_checksum() const
{
    if (_buffer) {
        return _buffer->calc_checksum();
    } else {
        return {};
    }
}

std::optional<const Key256> CoqAssemblyEncrypted::calc_checksum() const
{
    if (_buffer) {
        return _buffer->calc_checksum();
    } else {
        return {};
    }
}

/*
```coq
Program Definition decrypt (a : assemblyinformation) (b : AssemblyEncrypted.B) (ki : keyinformation) : option (assemblyinformation * AssemblyPlainFull.B) :=
    let a' := set_apos a 0 in
    (* fails if the key, the ivec, or the aid do not match, or the data was tampered with *)
    match Cstdio.decrypt (id_buffer_t_from_enc b) (aid a) (ivec ki) (pkey ki) with
    | None => None
    | Some (_bsz, bdec) =>
        let b' := id_assembly_plain_buffer_t_from_buf bdec in
        Some (a', b')
    end.
```
*/

std::shared_ptr<CoqAssemblyPlainFull> CoqAssemblyEncrypted::decrypt(const CoqAssembly::KeyInformation & ki)
{
    if (! _buffer) { return nullptr; }
    auto newbuf = _buffer->decrypt(ki._ivec, ki._pkey, _assemblyinformation._aid);
    _buffer.reset();
    if (! newbuf) { return nullptr; }
    return std::make_shared<CoqAssemblyPlainFull>(this, newbuf);
}

std::shared_ptr<CoqAssemblyPlainFull> CoqAssemblyPlainWritable::finish()
{
    auto res = std::make_shared<CoqAssemblyPlainFull>(this, _buffer);
    _buffer.reset();
    return res;
}

/*
```coq
Program Definition encrypt (a : assemblyinformation) (b : AssemblyPlainFull.B) (ki : keyinformation) : option (assemblyinformation * AssemblyEncrypted.B) :=
    let a' := set_apos a (assemblysize (nchunks a)) in
    match Cstdio.encrypt (id_buffer_t_from_full b) (aid a) (ivec ki) (pkey ki) with
    | None => None
    | Some (_bsz, benc) =>
        let b' := id_assembly_enc_buffer_t_from_buf benc in
        Some (a', b')
    end.
```
*/

std::shared_ptr<CoqAssemblyEncrypted> CoqAssemblyPlainFull::encrypt(const CoqAssembly::KeyInformation & ki)
{
    if (! _buffer) { return nullptr; }
    auto newbuf = _buffer->encrypt(ki._ivec, ki._pkey, _assemblyinformation._aid);
    _buffer.reset();
    if (! newbuf) { return nullptr; }
    return std::make_shared<CoqAssemblyEncrypted>(this, newbuf);
}

/*
```coq
Program Definition backup (a : assemblyinformation) (b : AssemblyPlainWritable.B) (fpos : N) (content : BufferPlain.buffer_t) : (assemblyinformation * blockinformation) :=
    let apos_n := a.(apos) in
    let bsz := BufferPlain.buffer_len content in
    let chksum := calc_checksum content in
    let nwritten := assembly_add_content content bsz apos_n b in
    let bi := {| blockid   := 1
            ; bchecksum := chksum
            ; blocksize := nwritten
            ; filepos   := fpos
            ; blockaid  := aid a
            ; blockapos := apos_n |} in
    let a' := set_apos a (apos_n + nwritten) in
    (a', bi).
```
*/
CoqAssembly::BlockInformation CoqAssemblyPlainWritable::backup(const CoqBufferPlain &b)
{
    return backup(b, 0, b.len());
}

CoqAssembly::BlockInformation CoqAssemblyPlainWritable::backup(const CoqBufferPlain &b, const uint32_t offset, const uint32_t dlen)
{
    uint32_t apos_n = _assemblyinformation._apos;
    const Nchunks nchunks = _config.nchunks();
    if (dlen + offset > b.len()) { return {}; }
    if (apos_n + dlen > assemblysize(nchunks)) { return {}; }
    auto const chksum = b.calc_checksum(offset, dlen);
    if (chksum) {
        // TODO parallelise !
        for (uint32_t i = 0; i < dlen; i++) {
            _buffer->at(idx2apos(i + apos_n, nchunks), b.at(i + offset));
        }
        _assemblyinformation._apos += dlen;
        return { 1, chksum.value(), dlen, 0,
                    _assemblyinformation._aid, apos_n };
    }
    return {};
}

/*
```coq
Program Definition extract (c : configuration) (a : assemblyinformation) (b : AssemblyEncrypted.B) : N :=
    let aid := aid a in
    let buf := id_buffer_t_from_enc b in
    List.fold_left
        ( fun nwritten cid =>
            let cpath := chunk_identifier_path c aid cid in
            let apos := chunksize_N * ((Conversion.pos2N cid) - 1) in
            nwritten + store_chunk_to_path cpath chunksize_N apos buf
        )
        (Utilities.make_list (nchunks a))
        0.
```
*/
uint32_t CoqAssemblyEncrypted::extract()
{
    if (! _buffer) { return 0; }
    const aid_t aid = _assemblyinformation._aid;
    uint32_t nwritten{0};
    assert(_assemblyinformation._nchunks.i() <= 256);
    for (int cid : _assemblyinformation._nchunks.sequence()) {
        if (const auto cfpathopt = chunk_path(cid, aid); cfpathopt) {
            const std::filesystem::path cfpath = cfpathopt.value();
            if (! std::filesystem::exists(cfpath)) {
                if (FILE * fstr = fopen(cfpath.string().c_str(), "wb"); fstr) {
#ifdef DEBUG
                    std::clog << "    extract chunk " << cid << " to path " << cfpath << std::endl;
#endif
                    if (int n = _buffer->fileout_sz_pos(CoqAssembly::chunksize * (cid - 1), CoqAssembly::chunksize, fstr); n == CoqAssembly::chunksize) {
                        nwritten += n;
                    } else {
                        std::clog << "only wrote " << n << " bytes to chunk with path " << cfpath.string() << std::endl;
                    }
                    fclose(fstr);
                } else {
                    std::clog << "cannot write to chunk with path " << cfpath.string() << std::endl;
                }
            } else {
                std::clog << "chunk file already exists with path " << cfpath.string() << std::endl;
            }
        }
    }
    _buffer.reset();
    return nwritten;
}

/*
```coq
Program Definition recall (c : configuration) (a : assemblyinformation) : option (assemblyinformation * AssemblyEncrypted.B) :=
    let cidlist := Utilities.make_list (nchunks a) in
    let b := BufferEncrypted.buffer_create (Conversion.pos2N (nchunks a) * chunksize_N) in
    let aid := aid a in
    let blen := BufferEncrypted.buffer_len b in
    let nread := List.fold_left
                    (fun nread cid =>
                        let cpath := chunk_identifier_path c aid cid in
                        match load_chunk_from_path cpath with
                        | None => nread
                        | Some cb =>
                            let apos := chunksize_N * ((Conversion.pos2N cid) - 1) in
                            if N.leb (apos + chunksize_N) blen
                            then nread + BufferEncrypted.copy_sz_pos cb 0 chunksize_N b apos
                            else nread
                        end    
                    )
                    cidlist
                    0 in
    let a' := set_apos a nread in
    let b' := id_enc_from_buffer_t b in
    if N.eqb nread blen
    then Some (a', b')
    else None.
```
*/
std::shared_ptr<CoqAssemblyEncrypted> CoqAssemblyEncrypted::recall(const CoqConfiguration & c, const CoqAssembly::aid_t & aid)
{
    std::shared_ptr<CoqAssemblyEncrypted> newassembly{new CoqAssemblyEncrypted(c)};
    newassembly->_assemblyinformation._aid = aid;
    const Nchunks& nchunks = c.nchunks();
    newassembly->_assemblyinformation._nchunks = nchunks;
    uint32_t nread{0};
    for (int cid : nchunks.sequence()) {
        if (const auto cfpathopt = newassembly->chunk_path(cid, aid); cfpathopt) {
            const std::filesystem::path cfpath = cfpathopt.value();
            if (std::filesystem::exists(cfpath)) {
                if (FILE * fstr = fopen(cfpath.string().c_str(), "rb"); fstr) {
#ifdef DEBUG
                    std::clog << "    recall chunk " << cid << " from path " << cfpath << std::endl;
#endif
                    if (int n = newassembly->_buffer->filein_sz_pos(CoqAssembly::chunksize * (cid - 1), CoqAssembly::chunksize, fstr); n == CoqAssembly::chunksize) {
                        nread += n;
                    } else {
                        std::clog << "only read " << n << " bytes from chunk with path " << cfpath.string() << std::endl;
                    }
                    fclose(fstr);
                } else {
                    std::clog << "cannot read from chunk with path " << cfpath.string() << std::endl;
                }
            } else {
                std::clog << "chunk file does not exists at path " << cfpath.string() << std::endl;
            }
        }
    }
    if (nread != assemblysize(nchunks)) {
        std::clog << "failed to recall assembly " << aid << ": read " << nread << " of " << assemblysize(nchunks) << " bytes" << std::endl;
        return nullptr;
    }
    newassembly->_assemblyinformation._apos = nread;
    return newassembly;
}

/*
```coq
Axiom assembly_get_content : AssemblyPlainFull.B -> N -> N -> BufferPlain.buffer_t -> N.
Program Definition restore (b : AssemblyPlainFull.B) (bi : blockinformation) : option BufferPlain.buffer_t :=
    let bsz := bi.(blocksize) in
    let b' := BufferPlain.buffer_create bsz in
    let nw := assembly_get_content b bsz bi.(blockapos) b' in
    if N.eqb nw bsz then
        let bcksum := calc_checksum b' in
        if String.eqb bcksum bi.(bchecksum) then
            Some b'
        else
            None
    else None.
```
*/
std::shared_ptr<CoqBufferPlain> CoqAssemblyPlainFull::restore(const CoqAssembly::BlockInformation & bi) const
{
    const Nchunks& nchunks = _config.nchunks();
    if (! _buffer || bi.blocksize == 0 || (uint64_t)bi.blockapos + bi.blocksize > assemblysize(nchunks)) { return nullptr; }
    std::shared_ptr<CoqBufferPlain> b{new CoqBufferPlain(bi.blocksize)};
    for (uint32_t idx = 0; idx < bi.blocksize; idx++) {
        b->at(idx, _buffer->at(idx2apos(idx + bi.blockapos, nchunks)));
    }
    bool ok{false};
    try {
        auto const chksum = b->calc_checksum();
        ok = chksum && chksum.value().toHex() == bi.bchecksum.toHex();
    } catch (...) {
        // no checksum given
    }
    if (! ok) {
        std::clog << "block checksum differs in assembly " << bi.blockaid << " at " << bi.blockapos << std::endl;
        return nullptr;
    }
    return b;
}

} // namespace
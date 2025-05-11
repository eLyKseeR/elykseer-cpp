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

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

import lxr_key128;
import lxr_key256;
import lxr_random;
import lxr_sha3;

import lxr_os;
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
   function
   x -> Elykseer_crypto.Random.with_rng (fun rng -> Elykseer_crypto.Random.random32 rng) |> string_of_int |>
     String.cat x |>
     String.cat (Unix.gethostname ()) |> String.cat (Unix.gettimeofday () |> string_of_float) |>
     Elykseer_crypto.Sha3_256.string
*/

void CoqAssembly::mk_assembly_id()
{
  auto now = std::chrono::system_clock::now();
  auto tstamp = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << lxr::Random::rng().random() << _config.my_id << OS::hostname() << tstamp;
  const std::string v{ss.str()};
  const auto h = Sha3_256::hash(v).toHex();
  // std::clog << "  mk_assembly_id: " << v << " => " << h << std::endl;
  _assemblyinformation._aid = h;
}

// CoqAssemblyPlainWritable

/*
```coq
    Definition create (c : configuration) : H * B :=
        let chunks := config_nchunks c in
        let b := BufferPlain.buffer_create (chunksize_N * Nchunks.to_N chunks) in
        let rb := Buffer.ranbuf128 tt in
        let nb := BufferPlain.copy_sz_pos rb 0 16 b 0 in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) nb, b).
```
*/

CoqAssemblyPlainWritable::CoqAssemblyPlainWritable(const CoqConfiguration &c)
  : CoqAssembly(c)
{
  // std::clog << "CoqAssemblyPlainWritable::CoqAssemblyPlainWritable(const CoqConfiguration &c)" << std::endl;
  const Nchunks& nchunks = c.nchunks();
  _buffer.reset(new CoqBufferPlain(assemblysize(nchunks)));
  uint32_t _rand32;
  uint32_t idx{0};
  while (idx < nchunks.u()) {
    _rand32 = Random::rng().random();
    const unsigned char *r = (const unsigned char[4])_rand32;
    for (int i = 0; i < 4; i++) {
      _buffer->at(idx, r[i]);
    }
    idx += 4;
  }
  _assemblyinformation._apos = nchunks.u();
  _assemblyinformation._nchunks = nchunks;
  mk_assembly_id();
}

CoqAssemblyPlainWritable::~CoqAssemblyPlainWritable() = default;

/*
```coq
    Definition create (c : configuration) :=
        let chunks := config_nchunks c in
        let sz := chunksize_N * Nchunks.to_N chunks in
        let b := BufferPlain.buffer_create sz in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) sz, b).
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
    Definition create (c : configuration) : H * B :=
        let chunks := config_nchunks c in
        let b := BufferEncrypted.buffer_create (chunksize_N * Nchunks.to_N chunks) in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) 0, b).
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

std::optional<const std::filesystem::path> CoqAssembly::chunk_path(const uint16_t cnum, const aid_t aid) const {
    std::stringstream ss;
    ss << _config.my_id << aid << cnum;
    const auto h = Sha3_256::hash(ss.str()).toHex();
    const int len = h.length();
    if (len != 64) {
        std::clog << "chunk_path |sha3_256| != 64 chars" << std::endl;
        return {};
    }
    std::filesystem::path fp = _config.path_chunks;
    if (! std::filesystem::exists(fp)) {
        std::filesystem::create_directory(fp);
    }
    char subdir[3];
    subdir[0]=h[len-2];
    subdir[1]=h[len-1];
    subdir[2]='\0';
    fp /= std::string(subdir,2);
    if (! std::filesystem::exists(fp)) {
        std::filesystem::create_directory(fp);
    }
    subdir[0]=h[len-4];
    subdir[1]=h[len-3];
    subdir[2]='\0';
    fp /= std::string(subdir,2);
    if (! std::filesystem::exists(fp)) {
        std::filesystem::create_directory(fp);
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

int CoqAssembly::afree() const
{
    return assemblysize(_assemblyinformation._nchunks) - _assemblyinformation._apos;
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
Program Definition decrypt (a : AssemblyEncrypted.H) (b : AssemblyEncrypted.B) (ki : keyinformation) : option (AssemblyPlainWritable.H * AssemblyPlainWritable.B) :=
    let a' := set apos (const 0) a in
    let bdec := Buffer.decrypt (id_buffer_t_from_enc b) (ivec ki) (pkey ki) in
    let b' := id_assembly_plain_buffer_t_from_buf bdec in
    Some (a', b').
```
*/

std::shared_ptr<CoqAssemblyPlainFull> CoqAssemblyEncrypted::decrypt(const CoqAssembly::KeyInformation & ki)
{
    auto newbuf = _buffer->decrypt(ki._ivec, ki._pkey);
    auto res = std::make_shared<CoqAssemblyPlainFull>(this, newbuf);
    _buffer.reset();
    return res;
}

std::shared_ptr<CoqAssemblyPlainFull> CoqAssemblyPlainWritable::finish()
{
    auto res = std::make_shared<CoqAssemblyPlainFull>(this, _buffer);
    _buffer.reset();
    return res;
}

/*
```coq
Program Definition encrypt (a : AssemblyPlainFull.H) (b : AssemblyPlainFull.B) (ki : keyinformation) : option (AssemblyEncrypted.H * AssemblyEncrypted.B) :=
    let a' := set apos (const (assemblysize (nchunks a))) a in
    let benc  := Buffer.encrypt (id_buffer_t_from_full b) (ivec ki) (pkey ki) in
    let b' := id_assembly_enc_buffer_t_from_buf benc in
    Some (a', b').
```
*/

std::shared_ptr<CoqAssemblyEncrypted> CoqAssemblyPlainFull::encrypt(const CoqAssembly::KeyInformation & ki)
{
    auto newbuf = _buffer->encrypt(ki._ivec, ki._pkey);
    auto res = std::make_shared<CoqAssemblyEncrypted>(this, newbuf);
    _buffer.reset();
    return res;
}

/*
```coq
Program Definition backup (a : AssemblyPlainWritable.H) (b : AssemblyPlainWritable.B) (fpos : N) (content : BufferPlain.buffer_t) : (AssemblyPlainWritable.H * blockinformation) :=
    let apos_n := apos a in
    let bsz := BufferPlain.buffer_len content in
    let chksum := calc_checksum content in
    let nwritten := assembly_add_content content bsz apos_n b in
    let bi := {| blockid   := 1
               ; bchecksum := chksum
               ; blocksize := nwritten
               ; filepos   := fpos
               ; blockaid  := aid a
               ; blockapos := apos_n |} in
    let a' := set apos (fun ap => ap + nwritten) a in
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
    if (dlen + offset > b.len()) { return {}; }
    auto const chksum = b.calc_checksum(offset, dlen);
    if (chksum) {
        // TODO parallelise !
        for (int i = 0; i < dlen; i++) {
            uint32_t apos = idx2apos(i + apos_n);
            _buffer->at(apos, b.at(i + offset));
        }
        _assemblyinformation._apos += dlen;
        return { 1, chksum.value(), dlen, 0,
                    _assemblyinformation._aid, apos_n };
    }
    return {};
}

/*
```coq
Program Definition extract (c : configuration) (a : AssemblyEncrypted.H) (b : AssemblyEncrypted.B) : N :=
    let aid := aid a in
    let buf := id_buffer_t_from_enc b in
    List.fold_left
        ( fun nwritten cid =>
            let cpath := chunk_identifier_path c aid cid in
            let apos := chunksize_N * ((Conversion.pos2N cid) - 1) in
            nwritten + ext_store_chunk_to_path cpath chunksize_N apos buf
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
                if (std::ofstream fstr(cfpath, std::ios::binary); fstr.good()) {
#ifdef DEBUG
                    std::clog << "    extract chunk " << cid << " to path " << cfpath << std::endl;
#endif
                    if (int n = _buffer->fileout_sz_pos(CoqAssembly::chunksize * (cid - 1), CoqAssembly::chunksize, fstr); n == CoqAssembly::chunksize) {
                        nwritten += n;
                    } else {
                        std::clog << "only wrote " << n << " bytes to chunk with path " << cfpath.string() << std::endl;
                    }
                    fstr.close();
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
Program Definition recall (c : configuration) (a : AssemblyEncrypted.H) : option (AssemblyEncrypted.H * AssemblyEncrypted.B) :=
    let cidlist := Utilities.make_list (nchunks a) in
    let b := BufferEncrypted.buffer_create (Conversion.pos2N (nchunks a) * chunksize_N) in
    let aid := aid a in
    let blen := BufferEncrypted.buffer_len b in
    let nread := List.fold_left
                    (fun nread cid =>
                        let cpath := chunk_identifier_path c aid cid in
                        match ext_load_chunk_from_path cpath with
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
    let a' := set apos (const nread) a in
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
                if (std::ifstream fstr(cfpath, std::ios::binary); fstr.good()) {
#ifdef DEBUG
                    std::clog << "    recall chunk " << cid << " from path " << cfpath << std::endl;
#endif
                    if (int n = newassembly->_buffer->filein_sz_pos(CoqAssembly::chunksize * (cid - 1), CoqAssembly::chunksize, fstr); n == CoqAssembly::chunksize) {
                        nread += n;
                    } else {
                        std::clog << "only read " << n << " bytes from chunk with path " << cfpath.string() << std::endl;
                    }
                    fstr.close();
                } else {
                    std::clog << "cannot read from chunk with path " << cfpath.string() << std::endl;
                }
            } else {
                std::clog << "chunk file does not exists at path " << cfpath.string() << std::endl;
            }
        }
    }
    return std::move(newassembly);
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
    std::shared_ptr<CoqBufferPlain> b{new CoqBufferPlain(bi.blocksize)};
    for (uint32_t idx = 0; idx < bi.blocksize; idx++) {
        uint32_t apos = idx2apos(idx + bi.blockapos);
        b->at(idx, _buffer->at(apos)); 
    }
    return b;
}

} // namespace
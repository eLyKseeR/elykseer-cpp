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
#include <optional>

import lxr_coqassembly;
import lxr_coqbuffer;
import lxr_coqconfiguration;


module lxr_coqenvironment;


namespace lxr {

/*
from elykseer-ml @ e6db580 (0.10.1), theories/Environment.v:
```coq
    Definition initial_environment (c : configuration) : E :=
        let (a,b) := AssemblyPlainWritable.create c in
        {| cur_assembly := a
         ; cur_buffer := b
         ; econfig := c
        |}.
```
*/

CoqEnvironment::CoqEnvironment(const CoqConfiguration &c)
  : _config(c)
{}

CoqEnvironment::~CoqEnvironment() = default;

CoqEnvironmentWritable::CoqEnvironmentWritable(const CoqConfiguration &c)
  : CoqEnvironment(c)
{}

CoqEnvironmentReadable::CoqEnvironmentReadable(const CoqConfiguration &c)
  : CoqEnvironment(c)
{}

/*
```coq
Definition recreate_assembly (e : environment AB) : environment AB :=
    let (a,b) := AssemblyPlainWritable.create (e.(econfig AB)) in
    {| cur_assembly := a; cur_buffer := b
    ;  econfig := e.(econfig AB)
    |}.
```
*/

void CoqEnvironmentWritable::recreate_assembly()
{
    _assembly.reset(new CoqAssemblyPlainWritable(_config));
}

void CoqEnvironmentReadable::recreate_assembly()
{}

/*
```coq
    Definition finalise_assembly (e0 : environment AB) : option (aid_t * keyinformation) :=
        let a0 := e0.(cur_assembly AB) in
        let apos := Assembly.apos a0 in
        conditionalTrace e0.(econfig AB).(trace) (N.ltb 0 apos) (* apos > 0 *)
        (Tracer.info) (Some ("finalising assembly " ++ a0.(aid) ++ " with apos = " ++ i2s (n2i apos))%string)
        (fun _ =>
            let (a,b) := Assembly.finish a0 e0.(cur_buffer AB) in
            let ki := {| pkey := cpp_mk_key256 tt
                       ; ivec := cpp_mk_key128 tt
                       ; localnchunks := Nchunks.to_positive (e0.(econfig AB).(Configuration.config_nchunks)) |} in
            optionalTrace e0.(econfig AB).(trace) (Assembly.encrypt a b ki)
            (Tracer.warning) (Some ("failed to encrypt assembly: " ++ a.(aid))%string)
            (fun _ => None)
            (Tracer.info) (Some ("encrypted assembly: " ++ a.(aid))%string)
            (fun '(a',b') =>
                let n := Assembly.extract e0.(econfig AB) a' b' in
                if N.ltb 0 n
                then Some (a.(aid), ki)
                else None
            )
        )
        (Tracer.info) (Some ("not finalising empty assembly " ++ a0.(aid) ++ " with apos = "  ++ i2s (n2i apos))%string)
        (fun _ => None).
```
   differences: an assembly is empty if it holds no data after the random header (apos <= h),
   and the key is only returned if all n chunks were written.
*/
std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>>  CoqEnvironmentWritable::finalise_assembly()
{
    if (! _assembly) { return {}; }
    if (_assembly->apos() <= (int)CoqAssembly::header_len(_config.nchunks())) {
        _assembly.reset();
        return {};
    }
    const CoqAssembly::aid_t aid = _assembly->aid();
    std::shared_ptr<CoqAssemblyPlainFull> finished_assembly = _assembly->finish();
    _assembly.reset();
    CoqAssembly::KeyInformation ki(_config);
    std::shared_ptr<CoqAssemblyEncrypted> encrypted_assembly = finished_assembly->encrypt(ki);
    if (! encrypted_assembly) {
        std::clog << "failed to encrypt assembly: " << aid << std::endl;
        ++n_failed;
        return {};
    }
    const uint32_t nwritten = encrypted_assembly->extract();
    if (nwritten != CoqAssembly::assemblysize(_config.nchunks())) {
        std::clog << "failed to extract assembly: " << aid << " wrote " << nwritten << " bytes" << std::endl;
        ++n_failed;
        return {};
    }
#ifdef DEBUG
    std::clog << "CoqEnvironmentWritable::finalise_assembly() " << aid << std::endl;
#endif
    return std::pair(aid, ki);
}

std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>> CoqEnvironmentReadable::finalise_assembly()
{
    return {};
}

/*
```coq
Definition finalise_and_recreate_assembly (e0 : environment AB)
 : option (environment AB * (aid_t * keyinformation)) :=
    match finalise_assembly e0 with
    | None => None
    | Some ki =>
        Some (EnvironmentWritable.recreate_assembly e0, ki)
    end.
```
*/
std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>> CoqEnvironmentWritable::finalise_and_recreate_assembly()
{
    auto res = finalise_assembly();
    recreate_assembly();
    return res;
}

std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>> CoqEnvironmentReadable::finalise_and_recreate_assembly()
{
    return {};
}


// Restore assembly from chunks

/*
```coq
Definition restore_assembly (e0 : environment AB) (aid : aid_t) (ki : keyinformation) : option (environment AB) :=
    match Assembly.recall e0.(econfig AB) {| nchunks := e0.(econfig AB).(Configuration.config_nchunks); aid := aid; apos := 0 |} with
    | None => None
    | Some (a1, b1) =>
        match Assembly.decrypt a1 b1 ki with
        | None => None
        | Some (a2, b2) =>
            Some {| cur_assembly := a2; cur_buffer := b2; econfig := e0.(econfig AB) |}
        end
    end.
```
*/
bool CoqEnvironmentReadable::restore_assembly(const CoqAssembly::aid_t & aid, const CoqAssembly::KeyInformation & ki)
{
    if (! CoqAssembly::supported_format(ki._version)) {
        std::clog << "cannot restore assembly " << aid << ": unsupported archive format '" << ki._version << "'" << std::endl;
        return false;
    }
    std::shared_ptr<CoqAssemblyEncrypted> new_assembly = CoqAssemblyEncrypted::recall(_config, aid);
    if (new_assembly) {
        auto decrypt_assembly = new_assembly->decrypt(ki);
        if (decrypt_assembly) {
            _assembly = std::move(decrypt_assembly);
            return true;
        }
    }
    return false;
}

bool CoqEnvironmentWritable::restore_assembly(const CoqAssembly::aid_t & aid, const CoqAssembly::KeyInformation & ki)
{
    return false;
}


// Backup file content

/*
```coq
    Program Definition backup (e0 : environment AB) (fp : string) (fpos : N) (content : BufferPlain.buffer_t) : (environment AB * (blockinformation * option (aid_t * keyinformation))) :=
        (* data is striped over the chunks: logical position i is stored in chunk (i mod n) at
           offset (i / n); the GCM tag occupies the last tag_len bytes of the last chunk, hence
           the last tag_len rows (tag_len * n logical bytes) are not available for data *)
        let nch := Nchunks.to_N e0.(econfig AB).(Configuration.config_nchunks) in
        let afree := (Assembly.assemblysize e0.(econfig AB).(Configuration.config_nchunks)) - e0.(cur_assembly AB).(apos) - Cstdio.tag_len * nch in
        let blen := BufferPlain.buffer_len content in
        let (ki, e1) := if afree <? blen then
                            match finalise_and_recreate_assembly e0 with
                            | None => (None, e0)
                            | Some (e0', ki') => (Some ki', e0')
                            end
                        else (None, e0) in
        let (a', bi) := Assembly.backup e1.(cur_assembly AB) e1.(cur_buffer AB) fpos content in
        ({| cur_assembly := a'
          ; cur_buffer := e1.(cur_buffer AB)
          ; econfig := e1.(econfig AB)
         |}, (bi, ki)).
```
   a block that does not fit is placed as a whole into a new assembly
*/

// TODO argument fname should be fhash
std::pair<CoqEnvironment::rel_fname_fblocks, CoqEnvironment::rel_aid_keys> CoqEnvironmentWritable::backup(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen)
{
    if (! _assembly) { return {}; }
    if (dlen > b.len()) { return {}; }

    rel_fname_fblocks _fblocks{};
    rel_aid_keys _kis{};
    if ((int)dlen > _assembly->afree()) {
        auto oki = finalise_and_recreate_assembly();
        if (oki) {
            _kis.push_back(oki.value());
        }
    }
    if ((int)dlen > _assembly->afree()) {
        std::clog << "block of " << dlen << " bytes does not fit into an assembly" << std::endl;
        return std::pair(_fblocks, _kis);
    }
    auto bi = _assembly->backup(b, 0, dlen);
    bi.filepos = fpos;
    _fblocks.push_back(bipair_t(fname, bi));
    return std::pair(_fblocks, _kis);
}

std::pair<CoqEnvironment::rel_fname_fblocks, CoqEnvironment::rel_aid_keys> CoqEnvironmentReadable::backup(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen)
{
    return {};
}

} // namespace
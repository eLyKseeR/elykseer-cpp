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
#include <iostream>
#include <memory>
#include <optional>

import lxr_coqassembly;
import lxr_coqbuffer;
import lxr_coqconfiguration;


module lxr_coqenvironment;


namespace lxr {

/*
```coq
Definition initial_environment (c : configuration) : environment :=
    let (a,b) := AssemblyPlainWritable.create c in
    {| cur_assembly := a
    ;  cur_buffer := b
    ;  config := c
    ;  fblocks := nil
    ;  keys := nil
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
        conditionalTrace e0.(econfig AB).(trace) (N.ltb 16 apos) (* apos > 16 *)
        (Tracer.info) (Some ("finalising assembly " ++ a0.(aid) ++ " with apos = " ++ i2s (n2i apos))%string)
        (fun _ =>
            let (a,b) := Assembly.finish a0 e0.(cur_buffer AB) in
            let ki := {| pkey := cpp_mk_key256 tt
                       ; ivec := cpp_mk_key128 tt
                       ; localnchunks := e0.(econfig AB).(Configuration.config_nchunks)
                       ; localid := e0.(econfig AB).(Configuration.my_id) |} in
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
*/
std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>>  CoqEnvironmentWritable::finalise_assembly()
{
    if (! _assembly) { return {}; }
    if (_assembly->apos() <= 16) {
        _assembly.reset();
        return {};
    }
    std::shared_ptr<CoqAssemblyPlainFull> finished_assembly = _assembly->finish();
    _assembly.reset();
    CoqAssembly::KeyInformation ki(_config);
    std::shared_ptr<CoqAssemblyEncrypted> encrypted_assembly = finished_assembly->encrypt(ki);
    if (encrypted_assembly) {
        encrypted_assembly->extract();
    }
#ifdef DEBUG
    std::clog << "CoqEnvironmentWritable::finalise_assembly()" << std::endl;
#endif
    return std::pair(encrypted_assembly->aid(), ki);
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
Program Definition backup (e0 : environment AB) (fp : string) (fpos : N) (content : BufferPlain.buffer_t)
 : (environment AB * (blockinformation * option (aid_t * keyinformation))) :=
    let afree := N.sub (Assembly.assemblysize e0.(econfig AB).(Configuration.config_nchunks)) e0.(cur_assembly AB).(apos) in
    let blen := BufferPlain.buffer_len content in
    let (ki, e1) := if N.ltb afree blen then
                        match finalise_and_recreate_assembly e0 with
                        | None => (None, e0)
                        | Some (e0', ki) => (Some ki, e0')
                        end
                        else (None, e0) in
    let (a', bi) := Assembly.backup e1.(cur_assembly AB) e1.(cur_buffer AB) fpos content in
    ({| cur_assembly := a'
        ; cur_buffer := e1.(cur_buffer AB)
        ; econfig := e1.(econfig AB)
        |}, (bi, ki)).
```
*/

// TODO argument fname should be fhash
std::pair<CoqEnvironment::rel_fname_fblocks, CoqEnvironment::rel_aid_keys> CoqEnvironmentWritable::backup(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen)
{
    if (! _assembly) { return {}; }
    uint32_t blen = b.len();
    if (dlen > blen) { return {}; }

    rel_fname_fblocks _fblocks{};
    uint32_t nwritten{0};
    rel_aid_keys _kis{};
    while (nwritten < dlen) {
        bool recreate{false};
        int sz = dlen - nwritten;
        int afree = _assembly->afree();
        if (sz > afree) {
            sz = afree;
            recreate = true;
        }
        //std::clog << "CoqEnvironmentWritable::backup dlen = " << dlen << " fpos = " << fpos << " nwritten = " << nwritten << " sz = " << sz << " recreate? " << recreate << std::endl;
        auto bi = _assembly->backup(b, nwritten, sz);
        bi.filepos = fpos;
        fpos += sz;
        nwritten += sz; // sz > 0 -> we make progress
        _fblocks.push_back({fname, bi});   // TODO pass in fhash
        if (recreate) {
            auto oki = finalise_and_recreate_assembly();
            if (oki) {
                _kis.push_back(oki.value());
            }
        }
    }
    return std::pair(_fblocks, _kis);
}

std::pair<CoqEnvironment::rel_fname_fblocks, CoqEnvironment::rel_aid_keys> CoqEnvironmentReadable::backup(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen)
{
    return {};
}

} // namespace
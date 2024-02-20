
```coq
Definition recreate_assembly (e : environment) : environment :=
    let (a,b) := AssemblyPlainWritable.create e.(config) in
    set cur_assembly (const a) (set cur_buffer (const b) e).
```
```cpp

void CoqEnvironmentWritable::recreate_assembly()
{
    _assembly.reset(new CoqAssemblyPlainWritable(_config));
}

void CoqEnvironmentReadable::recreate_assembly()
{}
```

```coq
Definition finalise_assembly (e0 : environment) : environment :=
    let a0 := cur_assembly e0 in
    let apos := apos a0 in
    if N.ltb 0 apos then
        let (a,b) := Assembly.finish a0 (cur_buffer e0) in
        let ki := {| pkey := cpp_mk_key256 tt
                   ; ivec := cpp_mk_key128 tt
                   ; localnchunks := e0.(config).(Configuration.config_nchunks)
                   ; localid := e0.(config).(Configuration.my_id) |} in
        let e1 := env_add_aid_key (aid a) e0 ki in
        match Assembly.encrypt a b ki with
        | None => e0
        | Some (a',b') =>
            let n := Assembly.extract (config e1) a' b' in
            if N.eqb n (Assembly.assemblysize e0.(config).(Configuration.config_nchunks))
            then e1
            else e0
        end
    else e0.
```
```cpp
void CoqEnvironmentWritable::finalise_assembly()
{
    if (! _assembly) { return; }
    std::shared_ptr<CoqAssemblyPlainFull> finished_assembly = _assembly->finish();
    _assembly.reset();
    CoqAssembly::KeyInformation ki(_config);
    std::shared_ptr<CoqAssemblyEncrypted> encrypted_assembly = finished_assembly->encrypt(ki);
    if (encrypted_assembly) {
        // std::clog << "have encrypted assembly" << std::endl;
        _keys.push_back({encrypted_assembly->aid(), std::move(ki)});
        encrypted_assembly->extract();
    }
}

void CoqEnvironmentReadable::finalise_assembly()
{}
```

```coq
Definition finalise_and_recreate_assembly (e0 : environment) : environment :=
    let e1 := finalise_assembly e0 in
    recreate_assembly e1.
```
```cpp
void CoqEnvironmentWritable::finalise_and_recreate_assembly()
{
    finalise_assembly();
    recreate_assembly();
}

void CoqEnvironmentReadable::finalise_and_recreate_assembly()
{}

```

## Extract relations

extract blockinformation of backuped file content as a relation
from filename to _CoqAssembly::BlockInformation_ aka. as a pair.

the environments memory of this relation is cleared.

```cpp
/*
CoqEnvironment::rel_fname_fblocks CoqEnvironment::extract_fblocks()
{
    rel_fname_fblocks res = _fblocks;
    _fblocks.clear();

    // sort by fname, fpos
    std::sort(res.begin(), res.end(), [](const bipair_t &a, const bipair_t &b) { return a.first < b.first && a.second.filepos < b.second.filepos; });

    // reset blockid numbers
    std::string lastfn{";"};
    int bnum{0};
    std::for_each(res.begin(), res.end(), [&lastfn,&bnum](bipair_t &v) { if (v.first != lastfn) { bnum=0; }; v.second.blockid = ++bnum; });

    return res;
}
*/
```

```cpp
CoqEnvironment::rel_aid_keys CoqEnvironment::extract_keys()
{
    rel_aid_keys res = std::move(_keys);
    _keys.clear(); // TODO already gone; on all platforms?
    return res;
}
```

## Restore assembly from chunks

```coq
Definition restore_assembly (e0 : environment) (aid : Assembly.aid_t) : option environment :=
    match key_for_aid e0 aid with
    | None => None
    | Some k =>
        match Assembly.recall e0.(config) {| nchunks := e0.(config).(Configuration.config_nchunks); aid := aid; apos := 0 |} with
        | None => None
        | Some (a1, b1) =>
            match Assembly.decrypt a1 b1 k with
            | None => None
            | Some (a2, b2) =>
                Some {| cur_assembly := a2; cur_buffer := b2; config := e0.(config); fblocks := e0.(fblocks); keys := e0.(keys) |}
            end
        end
    end.
```
```cpp
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
```

```cpp
bool CoqEnvironmentWritable::restore_assembly(const CoqAssembly::aid_t & aid, const CoqAssembly::KeyInformation & ki)
{
    return false;
}
```

## Backup file content

```coq
    Program Definition backup (e0 : environment AB) (fp : string) (fpos : N) (content : BufferPlain.buffer_t) : environment AB :=
        let afree := N.sub (Assembly.assemblysize e0.(config AB).(Configuration.config_nchunks)) e0.(cur_assembly AB).(apos) in
        let blen := BufferPlain.buffer_len content in
        let e1 := if N.ltb afree blen then
                    finalise_and_recreate_assembly e0
                else e0 in
        let (a', bi) := Assembly.backup e1.(cur_assembly AB) e1.(cur_buffer AB) fpos content in
        {| cur_assembly := a'
        ;  cur_buffer := e1.(cur_buffer AB)
        ;  config := e1.(config AB)
        ;  fblocks := (fp,bi) :: e1.(fblocks AB)
        ;  keys := e1.(keys AB)
        |}.
```

```cpp
CoqEnvironment::rel_fname_fblocks CoqEnvironmentWritable::backup(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen)
{
    if (! _assembly) { return {}; }
    uint32_t blen = b.len();
    if (dlen > blen) { return {}; }

    rel_fname_fblocks _fblocks{};
    uint32_t nwritten{0};
    while (nwritten < dlen) {
        bool recreate{false};
        int sz = dlen - nwritten;
        int afree = _assembly->afree();
        if (sz > afree) {
            sz = afree;
            recreate = true;
        }
        auto bi = _assembly->backup(b, nwritten, sz);
        nwritten += sz; // sz > 0 -> we make progress
        _fblocks.push_back({fname, bi});
        if (recreate) { finalise_and_recreate_assembly(); }
    }
    return _fblocks;
}
```

```cpp
CoqEnvironment::rel_fname_fblocks CoqEnvironmentReadable::backup(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen)
{
    return {};
}
```

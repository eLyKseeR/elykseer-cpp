
```coq
Definition recreate_assembly (e : environment) : environment :=
    let (a,b) := AssemblyPlainWritable.create e.(config) in
    set cur_assembly (const a) (set cur_buffer (const b) e).
```
```cpp

void CoqEnvironmentWriteable::recreate_assembly()
{
    _assembly.reset(new CoqAssemblyPlainWritable(_config));
}
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
void CoqEnvironmentWriteable::finalise_assembly()
{
    if (! _assembly) { return; }
    std::shared_ptr<CoqAssemblyPlainFull> finished_assembly = _assembly->finish();
    _assembly.reset();
    _assembly = nullptr;
    CoqAssembly::KeyInformation ki(_config);
    std::shared_ptr<CoqAssemblyEncrypted> encrypted_assembly = finished_assembly->encrypt(ki);
    if (encrypted_assembly) {
        encrypted_assembly->extract();
    }
}
```

```coq
Definition finalise_and_recreate_assembly (e0 : environment) : environment :=
    let e1 := finalise_assembly e0 in
    recreate_assembly e1.
```
```cpp
void CoqEnvironmentWriteable::finalise_and_recreate_assembly()
{
    finalise_assembly();
    recreate_assembly();
}
```

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
    CoqAssembly::AssemblyInformation ainfo;
    ainfo._nchunks = _config.nchunks();
    ainfo._aid = aid; ainfo._apos = 0;
    std::shared_ptr<CoqAssemblyEncrypted> new_assembly = CoqAssemblyEncrypted::recall(_config, ainfo);
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
bool CoqEnvironmentWriteable::restore_assembly(const CoqAssembly::aid_t & aid, const CoqAssembly::KeyInformation & ki)
{
    return false;
}
```

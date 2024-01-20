## calculate position in assembly

```cpp
inline const uint32_t idx2apos (const uint32_t idx, const uint32_t nchunks) {
    uint32_t cnum = idx % nchunks;
    uint32_t cidx = idx / nchunks;
    return cnum * CoqAssembly::chunklength + cidx;
}
```

## compute file path of a chunk

```cpp
std::optional<const std::filesystem::path> CoqAssembly::chunk_path(const uint16_t cnum, const aid_t aid) const {
    std::stringstream ss;
    ss << _config.my_id << aid << cnum;
    const auto h = Sha256::hash(ss.str()).toHex();
    const int len = h.length();
    if (len != 64) {
        std::clog << "chunk_path |sha256| != 64 chars" << std::endl;
        return {};
    }
    std::filesystem::path fp = _config.path_chunks;
    char subdir[3];
    subdir[0]=h[len-2];
    subdir[1]=h[len-1];
    subdir[2]='\0';
    fp /= subdir;
    subdir[0]=h[len-4];
    subdir[1]=h[len-3];
    subdir[2]='\0';
    fp /= subdir;
    (fp /= h) . replace_extension("lxr");
    return fp;
}
```

```coq
    Definition buffer_len : B -> N := BufferPlain.buffer_len.
```
```cpp

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
```

```coq
    Definition calc_checksum : B -> string := fun _ => "<>".
```
```cpp

std::optional<const Key256> CoqAssemblyPlainWritable::calc_checksum() const
{
    return {};
}
```

```coq
    Definition calc_checksum : B -> string := BufferPlain.calc_checksum.
```
```cpp

std::optional<const Key256> CoqAssemblyPlainFull::calc_checksum() const
{
    if (_buffer) {
        return _buffer->calc_checksum();
    } else {
        return {};
    }
}
```

```coq
    Definition calc_checksum : B -> string := BufferEncrypted.calc_checksum.
```
```cpp

std::optional<const Key256> CoqAssemblyEncrypted::calc_checksum() const
{
    if (_buffer) {
        return _buffer->calc_checksum();
    } else {
        return {};
    }
}
```

```coq
Program Definition decrypt (a : AssemblyEncrypted.H) (b : AssemblyEncrypted.B) (ki : keyinformation) : option (AssemblyPlainWritable.H * AssemblyPlainWritable.B) :=
    let a' := set apos (const 0) a in
    let bdec := Buffer.decrypt (id_buffer_t_from_enc b) (ivec ki) (pkey ki) in
    let b' := id_assembly_plain_buffer_t_from_buf bdec in
    Some (a', b').
```
```cpp

std::shared_ptr<CoqAssemblyPlainFull> CoqAssemblyEncrypted::decrypt(const CoqAssembly::KeyInformation & ki)
{
    return std::make_shared<CoqAssemblyPlainFull>(this);
}
```

```coq
Program Definition finish (a : AssemblyPlainWritable.H) (b : AssemblyPlainWritable.B) : (AssemblyPlainFull.H * AssemblyPlainFull.B) :=
    ( a
    , id_assembly_full_buffer_from_writable b ).
```
```cpp

std::shared_ptr<CoqAssemblyPlainFull> CoqAssemblyPlainWritable::finish()
{
    return std::make_shared<CoqAssemblyPlainFull>(this);
}
```

```coq
Program Definition encrypt (a : AssemblyPlainFull.H) (b : AssemblyPlainFull.B) (ki : keyinformation) : option (AssemblyEncrypted.H * AssemblyEncrypted.B) :=
    let a' := set apos (const (assemblysize (nchunks a))) a in
    let benc  := Buffer.encrypt (id_buffer_t_from_full b) (ivec ki) (pkey ki) in
    let b' := id_assembly_enc_buffer_t_from_buf benc in
    Some (a', b').
```
```cpp

std::shared_ptr<CoqAssemblyEncrypted> CoqAssemblyPlainFull::encrypt(const CoqAssembly::KeyInformation & ki)
{
    return std::make_shared<CoqAssemblyEncrypted>(this);
}
```

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
```cpp
CoqAssembly::BlockInformation CoqAssemblyPlainWritable::backup(uint64_t fpos, const CoqBufferPlain &b)
{
    uint32_t apos_n = _assemblyinformation._apos;
    uint32_t bsz = b.len();
    if (bsz > 0) {
        auto const chksum = b.calc_checksum();
        if (chksum) {
            int nwritten = b.copy_sz_pos(0, bsz, *_buffer, apos_n);
            if (nwritten > 0 && nwritten == bsz) {
                _assemblyinformation._apos += nwritten;
                return { 1, chksum.value(), static_cast<uint32_t>(nwritten), fpos,
                         _assemblyinformation._aid, apos_n };
            }
        }
    }
    return {};
}
```

This function is not used:
```coq
Program Definition restore (b : AssemblyPlainFull.B) (bi : blockinformation) : option BufferPlain.buffer_t :=
    let bsz := blocksize bi in
    let b' := BufferPlain.buffer_create bsz in
    let nw := assembly_get_content b bsz (blockapos bi) b' in
    if N.eqb nw bsz then
        let bcksum := calc_checksum b' in
        if String.eqb bcksum (bchecksum bi) then
            Some b'
        else
            None
    else None.```
```cpp
/*
CoqBufferPlain CoqAssemblyPlainFull::restore(const CoqAssembly::BlockInformation &bi)
{
    // TODO
    return {};
}
*/
```

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
```cpp
uint32_t CoqAssemblyEncrypted::extract() const
{
    if (! _buffer) { return 0; }
    const aid_t aid = _assemblyinformation._aid;
    uint32_t nwritten{0};
    for (int cid = 1; cid <= _assemblyinformation._nchunks; cid++) {
        if (const auto cfpathopt = chunk_path(cid); cfpathopt) {
            const std::filesystem::path cfpath = cfpathopt.value();
            if (! std::filesystem::exists(cfpath)) {
                if (FILE * fstr = fopen(cfpath.string().c_str(), "wb"); ! fstr) {
                    if (int n = _buffer->fileout_sz_pos(CoqAssembly::chunksize * (cid - 1), CoqAssembly::chunksize, fstr); n > 0) {
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
    return nwritten;
}
```

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
```cpp
std::shared_ptr<CoqAssemblyEncrypted> CoqAssemblyEncrypted::recall(const CoqConfiguration & c, const CoqAssembly::AssemblyInformation & ainfo)
{
    CoqAssemblyEncrypted *newassembly = new CoqAssemblyEncrypted(c);
    newassembly->_assemblyinformation._aid = ainfo._aid;
    uint32_t nread{0};
    for (int cid = 1; cid <= ainfo._nchunks; cid++) {
        if (const auto cfpathopt = newassembly->chunk_path(cid); cfpathopt) {
            const std::filesystem::path cfpath = cfpathopt.value();
            if (std::filesystem::exists(cfpath)) {
                if (FILE * fstr = fopen(cfpath.string().c_str(), "rb"); ! fstr) {
                    if (int n = newassembly->_buffer->filein_sz_pos(CoqAssembly::chunksize * (cid - 1), CoqAssembly::chunksize, fstr); n > 0) {
                        nread += n;
                    } else {
                        std::clog << "only read " << n << " bytes from chunk with path " << cfpath.string() << std::endl;
                    }
                    fclose(fstr);
                } else {
                    std::clog << "cannot write to chunk with path " << cfpath.string() << std::endl;
                }
            } else {
                std::clog << "chunk file does not exists at path " << cfpath.string() << std::endl;
            }
        }
    }
    return {};
}
```

```coq
Axiom assembly_get_content : AssemblyPlainFull.B -> N -> N -> BufferPlain.buffer_t -> N.
Program Definition restore (b : AssemblyPlainFull.B) (bi : blockinformation) : option BufferPlain.buffer_t :=
    let bsz := blocksize bi in
    let b' := BufferPlain.buffer_create bsz in
    let nw := assembly_get_content b bsz (blockapos bi) b' in
    if N.eqb nw bsz then
        let bcksum := calc_checksum b' in
        if String.eqb bcksum (bchecksum bi) then
            Some b'
        else
            None
    else None.
```
```cpp
std::shared_ptr<CoqBufferPlain> CoqAssemblyPlainFull::restore(const CoqAssembly::BlockInformation & bi) const
{
    CoqBufferPlain *b = new CoqBufferPlain(bi.blocksize);
    const int nchunks = _config.nchunks();
    uint32_t apos;
    for (uint32_t idx = 0; idx < bi.blocksize; idx++) {
        apos = idx2apos(idx + bi.blockapos, nchunks);
        b->at(idx, _buffer->at(apos)); 
    }
    return {};
}
```
## close()

```coq
Program Definition close : processor :=
    let ac := AssemblyCache.close this.(cache) in
    update_cache ac.
```

```cpp
void CoqProcessor::close()
{
    if (_cache)
        _cache->close();
}
```

## file_backup()

```coq
Definition block_sz := 32768.

Local Program Fixpoint rec_file_backup_inner (n_blocks : nat) (this : processor) (fi : fileinformation) (fpos : N) (fptr : Cstdio.fptr) : processor :=
    match n_blocks with
    | O => this
    | S n_blocks' =>
        if N.ltb fpos fi.(fsize) then
        let dsz : N := fi.(fsize) - fpos in
            let sz : N := if (N.ltb block_sz dsz) then block_sz else dsz in
            let _ := Cstdio.fseek fptr fpos in
            match Cstdio.fread fptr sz with
                | None => this
                | Some (nread, b) =>
                    let b' := BufferPlain.from_buffer b in
                    let wqe : writequeueentity :=
                        {| qfhash := Utilities.sha256 fi.(fname)
                         ; qfpos := fpos
                         ; qbuffer := b'
                        |} in
                    let this' := backup_block this wqe in
                    rec_file_backup_inner n_blocks' this' fi (fpos + sz) fptr
            end
        else this
    end.

Local Program Definition open_file_backup (n_blocks : N) (fi : fileinformation) (fpos : N) : processor :=
    match Cstdio.fopen fi.(fname) Cstdio.read_mode with
    | Some fptr =>
        let proc' := rec_file_backup_inner (nat_of_N n_blocks) this fi fpos fptr in
        match Cstdio.fclose fptr with
        | None => proc'
        | Some _ =>
            update_cache proc' (AssemblyCache.add_fileinformation proc'.(cache) fi)
        end
    | None => this
    end.

Program Definition file_backup (fp : Filesystem.path) : processor :=
    let fn := Filesystem.Path.to_string fp in
    let fi := get_file_information fn in
    (* deduplication level 1: compare file checksums *)
    let ofi' := FileinformationStore.find fn this.(cache).(acfistore) in
    let found :=
        match ofi' with
        | None => false
        | Some fi' => if fi'.(fchecksum) =? fi.(fchecksum) then true else false
        end
    in
    if (found) then
        this
    else
        let n_blocks := 1 + (fi.(fsize) + (block_sz / 2) - 1) / block_sz in
        let proc1 := open_file_backup n_blocks fi 0 in
        let (_, proc2) := run_write_requests proc1 in
        proc2.
```

```cpp
std::optional<CoqFilesupport::FileInformation> CoqProcessor::file_backup(const std::filesystem::path &fp)
{
    auto mayfi = CoqFilesupport::get_file_information(fp.string());
    if (mayfi) {
        CoqFilesupport::FileInformation fi = mayfi.value();
        // deduplication level 1: compare file checksums
        bool found = false;
        Key256 fhash = Sha3_256::hash(fp.string());
        std::optional<CoqFilesupport::FileInformation> ofi = _cache->get_finfo_store()->find(fhash.toHex());
        if (ofi) {
            CoqFilesupport::FileInformation prevfi = ofi.value();
            if (fi.fchecksum == prevfi.fchecksum) { found = true; }
        }
        if (found) {
#ifdef DEBUG
            std::clog << "file_backup skipped for '" << fp << "' as it is the same as before." << std::endl;
#endif
        } else {
            FILE *fstr = fopen(fp.c_str(), "rb");
            if (! fstr) { return {}; }
            {
                const uint64_t fsize{fi.fsize};
                uint64_t fpos{0};
                char buffer[block_sz];
                while (fpos < fsize) {
                    auto dsz = fsize - fpos;
                    int sz = block_sz;
                    if (dsz < block_sz) { sz = dsz; }
                    fseek(fstr, fpos, SEEK_SET);
                    int nread = fread(buffer, 1, sz, fstr);
                    
                    WriteQueueEntity wqe;
                    wqe._fhash = fhash;
                    wqe._fpos = fpos;
                    wqe._buffer.reset(new CoqBufferPlain(nread, buffer));

                    if (! _cache->enqueue_write_request(wqe)) {
                        _cache->iterate_write_queue();
                        _cache->enqueue_write_request(wqe);
                    }
                    fpos += sz;
                }
                _cache->iterate_write_queue();
            }
            fclose(fstr);
            _cache->get_finfo_store()->add(fhash.toHex(), fi);
            return fi;
        }
    }
    return {};
}
```

## directory_backup()

```coq
Local Program Definition internal_directory_entries (fp : Filesystem.path) : (list Filesystem.path * list Filesystem.path) :=
    Filesystem.list_directory fp ([],[]) (fun de '(lfiles,ldirs) =>
            if Filesystem.Direntry.is_directory de then
                let defp := Filesystem.Direntry.as_path de in
                (lfiles, defp :: ldirs)
            else if Filesystem.Direntry.is_regular_file de then
                let defp := Filesystem.Direntry.as_path de in
                (defp :: lfiles, ldirs)
            else
                (lfiles, ldirs)
        ).
```

```cpp
std::vector<std::filesystem::path> directory_files(const std::filesystem::path &fp) {
    std::vector<std::filesystem::path> _res{};
    for (const auto& de : std::filesystem::directory_iterator(fp)) {
        if (de.is_regular_file()) {
            _res.push_back(de);
        }
    }
    return _res;
}

std::vector<std::filesystem::path> directory_subdirs(const std::filesystem::path &fp) {
    std::vector<std::filesystem::path> _res{};
    for (const auto& de : std::filesystem::directory_iterator(fp)) {
        if (de.is_directory()) {
            _res.push_back(de);
        }
    }
    return _res;
}
```

```coq
Program Definition directory_backup (this : processor) (fp : Filesystem.path) : processor :=
    let '(lfiles, _) := internal_directory_entries fp in
    List.fold_left file_backup lfiles this.
```

```cpp
void CoqProcessor::directory_backup(const std::filesystem::path &fp) {
    if (std::filesystem::is_directory(fp)) {
        auto lfiles = directory_files(fp);
        for (auto const &p : lfiles) {
            file_backup(p);
        }
    }
}
```

## recursive_backup()

```coq
Local Program Fixpoint internal_recursive_backup (maxdepth : nat) (this : processor) (fp : Filesystem.path) : processor :=
    match maxdepth with
    | O => this
    | S depth =>
        Filesystem.list_directory fp this (fun de proc =>
            if Filesystem.Direntry.is_directory de then
                let defp := Filesystem.Direntry.as_path de in
                internal_recursive_backup depth proc defp
            else if Filesystem.Direntry.is_regular_file de then
                let defp := Filesystem.Direntry.as_path de in
                file_backup proc defp
            else
                proc
        )
    end.
Program Definition recursive_backup (this : processor) (maxdepth : N) (fp : Filesystem.path) : processor :=
    if Filesystem.Path.is_directory fp then
        internal_recursive_backup (nat_of_N maxdepth) this fp
    else
        this.
```

```cpp
void CoqProcessor::recursive_backup(const std::filesystem::path &fp) {
    if (std::filesystem::is_directory(fp)) {
        directory_backup(fp);
        auto ldirs = directory_subdirs(fp);
        for (auto const &p : ldirs) {
            recursive_backup(p);
        }
    }
}
```

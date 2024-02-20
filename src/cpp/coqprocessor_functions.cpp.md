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
        let _ := Cstdio.fclose fptr in
        proc'
    | None => this
    end.

Program Definition file_backup (fp : Filesystem.path) : (fileinformation * processor) :=
    let fi := get_file_information (Filesystem.Path.to_string fp) in
    let n_blocks := 1 + (fi.(fsize) + (block_sz / 2) - 1) / block_sz in
    let proc1 := open_file_backup n_blocks fi 0 in
    let (_, proc2) := run_write_requests proc1 in
    (fi, proc2).
```

```cpp
std::optional<CoqFileSupport::FileInformation> CoqProcessor::file_backup(const std::filesystem::path &fp)
{
    auto mayfi = CoqFileSupport::get_file_information(fp.string());
    if (mayfi) {
        CoqFileSupport::FileInformation fi = mayfi.value();
        FILE *fstr = fopen(fp.c_str(), "rb");
        if (! fstr) { return {}; }
        {
            Key256 fhash = Sha256::hash(fp.string());
            const uint64_t fsize(fi.fsize);
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
        }
        fclose(fstr);
        return fi;
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
Program Definition directory_backup (this : processor) (fp : Filesystem.path) : (list fileinformation * processor) :=
    let '(lfiles, _) := internal_directory_entries fp in
    List.fold_left (fun '(fis,proc0) filepath => let (fi, proc1) := file_backup proc0 filepath in (fi :: fis, proc1)) lfiles ([], this).
```

```cpp
std::vector<CoqFileSupport::FileInformation> CoqProcessor::directory_backup(const std::filesystem::path &fp) {
    std::vector<CoqFileSupport::FileInformation> _res{};
    if (std::filesystem::is_directory(fp)) {
        auto lfiles = directory_files(fp);
        for (auto const &p : lfiles) {
            auto mayfi = file_backup(p);
            if (mayfi) {
                _res.push_back(mayfi.value());
            }
        }
    }
    return _res;
}

void CoqProcessor::directory_backup_0(const std::filesystem::path &fp) {
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
Local Program Fixpoint internal_recursive_backup (maxdepth : nat) (this : processor) (fis0 : list fileinformation) (fp : Filesystem.path) : (list fileinformation * processor) :=
    match maxdepth with
    | O => (fis0, this)
    | S depth =>
        Filesystem.list_directory fp (fis0, this) (fun de '(fis, proc) =>
            if Filesystem.Direntry.is_directory de then
                let defp := Filesystem.Direntry.as_path de in
                let '(fis', proc') := internal_recursive_backup depth proc fis0 defp in
                (fis' ++ fis, proc')
            else if Filesystem.Direntry.is_regular_file de then
                let defp := Filesystem.Direntry.as_path de in
                let '(fi, proc') := file_backup proc defp in
                (fi :: fis, proc')
            else
                (fis, proc)
        )
    end.
Program Definition recursive_backup (this : processor) (maxdepth : N) (fp : Filesystem.path) : (list fileinformation * processor) :=
    if Filesystem.Path.is_directory fp then
        internal_recursive_backup (nat_of_N maxdepth) this [] fp
    else
        ([], this).
```

```cpp
std::vector<CoqFileSupport::FileInformation> CoqProcessor::recursive_backup(const std::filesystem::path &fp) {
    std::vector<CoqFileSupport::FileInformation> _res{};
    if (std::filesystem::is_directory(fp)) {
        auto vfiles = directory_backup(fp);
        for (auto const &f : vfiles) {
            _res.push_back(f);
        }
        auto ldirs = directory_subdirs(fp);
        for (auto const &p : ldirs) {
            auto fis = recursive_backup(p);
            for (auto const &fi : fis) {
                _res.push_back(fi);
            }
        }
    }
    return _res;
}

void CoqProcessor::recursive_backup_0(const std::filesystem::path &fp) {
    if (std::filesystem::is_directory(fp)) {
        directory_backup_0(fp);
        auto ldirs = directory_subdirs(fp);
        for (auto const &p : ldirs) {
            recursive_backup_0(p);
        }
    }
}
```

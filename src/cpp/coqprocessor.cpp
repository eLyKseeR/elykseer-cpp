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

#include "generator.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <iostream>
#include <string>
#include <vector>

import lxr_key256;

import lxr_coqconfiguration;
import lxr_coqbuffer;
import lxr_coqassemblycache;
import lxr_coqfilesupport;


module lxr_coqprocessor;


namespace lxr {

CoqProcessor::CoqProcessor(const CoqConfiguration &c, const std::shared_ptr<CoqAssemblyCache> &cache)
  : _config(c), _cache(cache)
{}

CoqProcessor::~CoqProcessor() {
    if (_cache) {
        _cache->close();
        _cache.reset();
    }
}

// close()
/*
```coq
Program Definition close : processor :=
    let ac := AssemblyCache.close this.(cache) in
    update_cache ac.
```
*/
void CoqProcessor::close()
{
    if (_cache)
        _cache->close();
}

// file_backup()
/*
```coq
Definition block_sz := 32768.

Program Definition file_backup (find_fchecksum : string -> option string) (find_fblocks : string -> list blockinformation) (fp : Filesystem.path) : processor :=
    let fn := Filesystem.Path.to_string fp in
    let fi := get_file_information this.(config) fn in
    (* deduplication level 1: compare file checksums *)
    let found :=
        match find_fchecksum fi.(fhash) with
        | None => false
        | Some fchecksum' => fchecksum' =? fi.(fchecksum)
        end
    in
    ...
```
   deduplication level 2 (reuse of unchanged blocks, prepare_blocks/zip_blocks) is not implemented.
*/
std::optional<CoqFilesupport::FileInformation> CoqProcessor::file_backup(const std::filesystem::path &fp)
{
    auto mayfi = CoqFilesupport::get_file_information(fp.string(), _config.my_id);
    if (mayfi) {
        CoqFilesupport::FileInformation fi = mayfi.value();
        // deduplication level 1: compare file checksums
        bool found = false;
        Key256 fhash{true}; fhash.fromHex(fi.fhash);
        std::optional<CoqFilesupport::FileInformation> ofi = _cache->get_finfo_store()->find(fi.fhash);
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
            _cache->get_finfo_store()->add(fi.fhash, fi);
            return fi;
        }
    }
    return {};
}

// directory_backup()
/*
   in elykseer-ml @ e6db580 (0.10.1) the directory traversal is commented out in
   theories/Processor.v and implemented in the tools; the earlier definitions:
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
*/
std::generator<std::filesystem::path> directory_files(const std::filesystem::path &fp) {
    for (const auto& de : std::filesystem::directory_iterator(fp)) {
        if (de.is_regular_file()) {
            co_yield de;
        }
    }
}

std::generator<std::filesystem::path> directory_subdirs(const std::filesystem::path &fp) {
    for (const auto& de : std::filesystem::directory_iterator(fp)) {
        if (de.is_directory()) {
            co_yield de;
        }
    }
}

/*
```coq
Program Definition directory_backup (this : processor) (fp : Filesystem.path) : processor :=
    let '(lfiles, _) := internal_directory_entries fp in
    List.fold_left file_backup lfiles this.
```
*/

void CoqProcessor::directory_backup(const std::filesystem::path &fp) {
    if (std::filesystem::is_directory(fp)) {
        for (auto const &p : directory_files(fp)) {
            file_backup(p);
        }
    }
}

// recursive_backup()
/*
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
*/
void CoqProcessor::recursive_backup(const std::filesystem::path &fp) {
    if (std::filesystem::is_directory(fp)) {
        directory_backup(fp);
        for (auto const &p : directory_subdirs(fp)) {
            recursive_backup(p);
        }
    }
}

} // namespace
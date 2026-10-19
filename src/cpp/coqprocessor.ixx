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

#include <filesystem>
#include <memory>
#include <optional>

import lxr_coqconfiguration;
import lxr_coqfilesupport;
import lxr_coqassemblycache;


export module lxr_coqprocessor;


export namespace lxr {

/*

from elykseer-ml @ e6db580 (0.10.1), theories/Processor.v (outline):

```coq
Module Export Processor.

Definition RecordProcessor := Type.
Record processor : RecordProcessor :=
    mkprocessor
        { config : configuration
        ; cache : assemblycache
        }.

Definition cache_sz : positive := 3.
Definition block_sz := 32768.
Definition prepare_processor (c : configuration) : processor

Program Definition file_backup (find_fchecksum : string -> option string) (find_fblocks : string -> list blockinformation) (fp : Filesystem.path) : processor

Program Definition directory_backup (this : processor) (fp : Filesystem.path) : processor

Program Definition recursive_backup (this : processor) (maxdepth : N) (fp : Filesystem.path) : processor

Program Definition close (this : processor) : processor

End Processor.
```
*/

class CoqProcessor
{
    public:
        static constexpr int block_sz = 32768;
        explicit CoqProcessor(const CoqConfiguration & c, const std::shared_ptr<CoqAssemblyCache> & cache);
        ~CoqProcessor();
        void close();
        std::optional<CoqFilesupport::FileInformation> file_backup(const std::filesystem::path &);
        void directory_backup(const std::filesystem::path &);
        void recursive_backup(const std::filesystem::path &);
    private:
        const CoqConfiguration _config;
        std::shared_ptr<CoqAssemblyCache> _cache{nullptr};
        CoqProcessor(CoqProcessor const &) = delete;
        CoqProcessor & operator=(CoqProcessor const &) = delete;
};

} // namespace
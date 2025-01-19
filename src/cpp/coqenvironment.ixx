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

#include <memory>
#include <optional>
#include <vector>

import lxr_coqassembly;
import lxr_coqbuffer;
import lxr_coqconfiguration;


export module lxr_coqenvironment;


export namespace lxr {

/*

```coq
Module Export Environment.

Definition RecordEnvironment := Type.
Record environment (AB : Type) : RecordEnvironment :=
    mkenvironment
        { cur_assembly : assemblyinformation
        ; cur_buffer : AB
        ; econfig : configuration
        }.

Definition initial_environment (c : configuration) : environment := ...

Definition recreate_assembly (e : environment) : environment := ...

Definition restore_assembly (e0 : environment AB) (aid : aid_t) (ki : keyinformation) : option (environment AB) := ...

Definition finalise_assembly (e0 : environment AB) : option (aid_t * keyinformation) := ...

Definition finalise_and_recreate_assembly (e0 : environment AB) : option (environment AB * (aid_t * keyinformation)) ...

Program Definition backup (e0 : environment AB) (fp : string) (fpos : N) (content : BufferPlain.buffer_t) : (environment AB * (blockinformation * option (aid_t * keyinformation))) := ...

End Environment.
```

*/

class CoqEnvironment
{
    public:
        typedef std::pair<std::string, CoqAssembly::BlockInformation> bipair_t;
        typedef std::vector<bipair_t> rel_fname_fblocks;
        typedef std::pair<CoqAssembly::aid_t, CoqAssembly::KeyInformation> pkpair_t;
        typedef std::vector<pkpair_t> rel_aid_keys;

    protected:
        explicit CoqEnvironment(const CoqConfiguration & c);
        virtual ~CoqEnvironment();

        virtual void recreate_assembly() = 0;
        virtual std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>> finalise_assembly() = 0;
        virtual std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>> finalise_and_recreate_assembly() = 0;
        virtual bool restore_assembly(const CoqAssembly::aid_t &aid, const CoqAssembly::KeyInformation & ki) = 0;
        virtual std::pair<rel_fname_fblocks, rel_aid_keys> backup(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen) = 0;

        const CoqConfiguration _config;

    private:
        CoqEnvironment(CoqEnvironment *) = delete;
        CoqEnvironment(CoqEnvironment const &) = delete;
        CoqEnvironment & operator=(CoqEnvironment const &) = delete;
};


class CoqEnvironmentReadable : public CoqEnvironment
{
    public:
        explicit CoqEnvironmentReadable(const CoqConfiguration & c);
        virtual ~CoqEnvironmentReadable() = default;
        virtual bool restore_assembly(const CoqAssembly::aid_t &aid, const CoqAssembly::KeyInformation & ki) override final;
        std::shared_ptr<CoqAssemblyPlainFull> _assembly{nullptr};

    private:
        virtual void recreate_assembly() override final;
        virtual std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>> finalise_assembly() override final;
        virtual std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>> finalise_and_recreate_assembly() override final;
        virtual std::pair<rel_fname_fblocks, rel_aid_keys> backup(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen) override final;
};


class CoqEnvironmentWritable : public CoqEnvironment
{
    public:
        explicit CoqEnvironmentWritable(const CoqConfiguration & c);
        virtual ~CoqEnvironmentWritable() = default;
        virtual void recreate_assembly() override final;
        virtual std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>> finalise_assembly() override final;
        virtual std::optional<std::pair<CoqAssembly::aid_t,CoqAssembly::KeyInformation>> finalise_and_recreate_assembly() override final;
        virtual std::pair<rel_fname_fblocks, rel_aid_keys> backup(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen) override final;
        std::shared_ptr<CoqAssemblyPlainWritable> _assembly{nullptr};

    private:
        virtual bool restore_assembly(const CoqAssembly::aid_t &aid, const CoqAssembly::KeyInformation & ki) override;
};

} // namespace
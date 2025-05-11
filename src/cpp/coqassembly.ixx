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

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

import lxr_coqbuffer;
import lxr_coqconfiguration;
import lxr_nchunks;

import lxr_key256;
import lxr_key128;


export module lxr_coqassembly;


export namespace lxr {

class CoqAssembly
{
    public:
        typedef std::string aid_t;
        static constexpr uint32_t chunkwidth = 256;
        static constexpr uint32_t chunklength = 1024;
        static constexpr uint32_t chunksize = chunkwidth * chunklength;
        static const uint32_t assemblysize (const Nchunks & nchunks) { return chunksize * nchunks.u(); }

        struct AssemblyInformation
        {
            AssemblyInformation();
            AssemblyInformation(const CoqConfiguration & c);
            AssemblyInformation(const CoqConfiguration & c, const std::string & aid);
            Nchunks _nchunks{ChunkRange::min_n};
            uint32_t _apos{0};
            aid_t _aid{""};
        };

        struct BlockInformation
        {
            int blockid;
            Key256 bchecksum{true};
            uint32_t blocksize;
            uint64_t filepos;
            aid_t blockaid;
            uint32_t blockapos;
        };

        struct KeyInformation
        {
            KeyInformation() {};
            KeyInformation(const CoqConfiguration & c);
            Key128 _ivec{false};
            Key256 _pkey{false};
            Nchunks _localnchunks{ChunkRange::min_n};
        };

    protected:
        explicit CoqAssembly(const CoqConfiguration & c);
        explicit CoqAssembly(CoqAssembly*);
        CoqAssembly();
        virtual ~CoqAssembly();

        virtual void mk_assembly_id() final;
        virtual int buffer_len() const = 0;
        virtual std::optional<const Key256> calc_checksum() const = 0;
        virtual std::optional<const std::filesystem::path> chunk_path(const uint16_t cnum, const aid_t aid) const final;
        
        public:
        virtual int apos() const final;
        virtual int afree() const final;
        virtual std::string aid() const final;
        const uint32_t idx2apos (const uint32_t idx) const {
            uint32_t nc = _assemblyinformation._nchunks.u();
            uint32_t asize = assemblysize(_assemblyinformation._nchunks);
            uint32_t eff_asize = asize - nc;
            uint32_t proj = idx * nc;
            return (proj % eff_asize) + floor(proj / eff_asize) + nc;
        }

    protected:
        const CoqConfiguration _config;
        struct AssemblyInformation _assemblyinformation;

    private:
        CoqAssembly(CoqAssembly const &) = delete;
        CoqAssembly & operator=(CoqAssembly const &) = delete;
};


class CoqAssemblyPlainFull;

class CoqAssemblyPlainWritable : public CoqAssembly
{
    public:
        explicit CoqAssemblyPlainWritable(const CoqConfiguration & c);
        virtual ~CoqAssemblyPlainWritable();
        virtual int buffer_len() const override final;
        virtual std::optional<const Key256> calc_checksum() const override final;
        virtual std::shared_ptr<CoqAssemblyPlainFull> finish() final;
        virtual CoqAssembly::BlockInformation backup(const CoqBufferPlain &b) final;
        virtual CoqAssembly::BlockInformation backup(const CoqBufferPlain &b, const uint32_t offset, const uint32_t dlen) final;
    private:
        std::shared_ptr<CoqBufferPlain> _buffer{nullptr};
};


class CoqAssemblyEncrypted;

class CoqAssemblyPlainFull : public CoqAssembly
{
    public:
        explicit CoqAssemblyPlainFull(const CoqConfiguration & c);
        explicit CoqAssemblyPlainFull(CoqAssemblyEncrypted *, std::shared_ptr<CoqBufferPlain> &b);
        explicit CoqAssemblyPlainFull(CoqAssemblyPlainWritable *, std::shared_ptr<CoqBufferPlain> &b);
        virtual ~CoqAssemblyPlainFull();
        virtual int buffer_len() const override final;
        virtual std::optional<const Key256> calc_checksum() const override final;
        virtual std::shared_ptr<CoqAssemblyEncrypted> encrypt(const CoqAssembly::KeyInformation & ki) final;
        virtual std::shared_ptr<CoqBufferPlain> restore(const CoqAssembly::BlockInformation & bi) const final;
    private:
        std::shared_ptr<CoqBufferPlain> _buffer{nullptr};
};


class CoqAssemblyEncrypted : public CoqAssembly
{
    public:
        explicit CoqAssemblyEncrypted(const CoqConfiguration & c);
        explicit CoqAssemblyEncrypted(CoqAssemblyPlainFull*, std::shared_ptr<CoqBufferEncrypted> &b);
        virtual ~CoqAssemblyEncrypted();
        virtual int buffer_len() const override final;
        virtual std::optional<const Key256> calc_checksum() const override final;
        virtual std::shared_ptr<CoqAssemblyPlainFull> decrypt(const CoqAssembly::KeyInformation & ki) final;
        virtual uint32_t extract() final;
        static std::shared_ptr<CoqAssemblyEncrypted> recall(const CoqConfiguration & c, const CoqAssembly::aid_t & aid);
    private:
        std::shared_ptr<CoqBufferEncrypted> _buffer{nullptr};
};

} // namespace
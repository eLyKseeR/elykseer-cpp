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

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <cassert>
#include <string>
#include <string_view>

import lxr_coqbuffer;
import lxr_coqconfiguration;
import lxr_nchunks;

import lxr_key256;
import lxr_key128;


export module lxr_coqassembly;


export namespace lxr {

/*
   implements the archive format of elykseer-ml 0.10.1 (doc/08_Format.md),
   see theories/Assembly.v @ e6db580
*/

class CoqAssembly
{
    public:
        typedef std::string aid_t;
        // the elykseer-ml version whose archive format is implemented; stored in the key records
        static constexpr std::string_view format_version{"0.10.1"};
        // assemblies of all 0.10.x versions can be read
        static bool supported_format(const std::string & version) { return version.starts_with("0.10."); }
        static constexpr uint32_t chunkwidth = 256;
        static constexpr uint32_t chunklength = 1024;
        static constexpr uint32_t chunksize = chunkwidth * chunklength;
        // AES-256-GCM tag in the last tag_len bytes of the last chunk (Cstdio.tag_len)
        static constexpr uint32_t tag_len = CoqBuffer::tag_len;
        static const uint32_t assemblysize (const Nchunks & nchunks) { return chunksize * nchunks.u(); }
        // random header: h = min(n - tag_len, 128) bytes (AssemblyPlainWritable.create)
        static const uint32_t header_len (const Nchunks & nchunks) { return std::min(nchunks.u() - tag_len, 128U); }
        // aid = sha3_256(my_id ^ hex(256 random bits)) (Utilities.rnd256)
        static aid_t mk_aid(const std::string & my_id, const std::string & rnd_hex);
        // chunk id = sha3_256(my_id ^ cid ^ aid) (chunk_identifier)
        static std::string chunk_identifier(const std::string & my_id, const aid_t & aid, const uint16_t cid);

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
            // archive format of the assembly; empty for records of elykseer-cpp < 5
            std::string _version{CoqAssembly::format_version};
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
        // logical index i -> chunk (i mod n), offset (i div n) (elykseer-base/assembly.cxx)
        static inline const uint32_t idx2apos (const uint32_t idx, const Nchunks & nchunks) {
            uint32_t nc = nchunks.u();
            assert(idx < nc * CoqAssembly::chunksize);
            uint32_t cnum = idx % nc;
            uint32_t cidx = idx / nc;
            return cnum * CoqAssembly::chunksize + cidx;
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
        // with a given aid, for conformance tests
        explicit CoqAssemblyPlainWritable(const CoqConfiguration & c, const aid_t & aid);
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
        // nullptr if the encryption failed
        virtual std::shared_ptr<CoqAssemblyEncrypted> encrypt(const CoqAssembly::KeyInformation & ki) final;
        // nullptr if the block is out of range or its checksum differs
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
        // nullptr if the key or the aid do not match, or the data was modified
        virtual std::shared_ptr<CoqAssemblyPlainFull> decrypt(const CoqAssembly::KeyInformation & ki) final;
        virtual uint32_t extract() final;
        // nullptr unless all n chunks were read completely
        static std::shared_ptr<CoqAssemblyEncrypted> recall(const CoqConfiguration & c, const CoqAssembly::aid_t & aid);
    private:
        std::shared_ptr<CoqBufferEncrypted> _buffer{nullptr};
};

} // namespace
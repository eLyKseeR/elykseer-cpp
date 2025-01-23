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
#include <variant>

import lxr_key256;

import lxr_coqassembly;
import lxr_coqbuffer;
import lxr_coqconfiguration;
import lxr_coqenvironment;
import lxr_coqstore;


export module lxr_coqassemblycache;


export namespace lxr {

/*
Coq defines data structures in module [Configuration](https://github.com/eLyKseeR/elykseer-ml/blob/main/theories/Configuration.v)
```coq
Record readqueueentity : Type :=
    mkreadqueueentity
        { rqaid : Assembly.aid_t
        ; rqapos : N
        ; rqrlen : N
        ; rqfpos : N
        }.
Record readqueueresult : Type :=
    mkreadqueueresult
        { readrequest : readqueueentity
        ; rresult : BufferPlain.buffer_t
        }.

Record writequeueentity : Type :=
    mkwritequeueentity
        { qfhash : string
        ; qfpos : N
        ; qbuffer : BufferPlain.buffer_t (* implicit write data size = buffer size *)
        }.
Record writequeueresult : Type :=
    mkwritequeueresult
        { writerequest : writequeueentity
        ; wresult : readqueueentity
        }.
```
*/

struct ReadQueueEntity
{
    CoqAssembly::aid_t _aid;
    uint32_t _apos;
    uint32_t _rlen;
    uint64_t _fpos;
    Key256 _chksum{true};
};

struct ReadQueueResult
{
    struct ReadQueueEntity _readrequest;
    std::shared_ptr<CoqBufferPlain> _rresult{nullptr};
};

struct WriteQueueEntity
{
    Key256 _fhash{true};
    uint64_t _fpos{0};
    std::shared_ptr<CoqBufferPlain> _buffer{nullptr};
};

struct WriteQueueResult
{
    struct WriteQueueEntity _writerequest;
    struct ReadQueueEntity _wresult;
};


class CoqAssemblyCache
{
    public:
        typedef std::variant<uint64_t,uint32_t,std::string> mvalue_t;
        typedef std::pair<std::string, mvalue_t> metric_t;
        typedef std::vector<metric_t> vmetric_t;
        explicit CoqAssemblyCache(const CoqConfiguration &c, const int n_envs, const int depth);
        ~CoqAssemblyCache();
        bool enqueue_write_request(const WriteQueueEntity &);
        bool enqueue_read_request(const ReadQueueEntity &);
        std::vector<ReadQueueResult> iterate_read_queue();
        std::vector<WriteQueueResult> iterate_write_queue();
        void flush();
        void close();
        void register_key_store(std::shared_ptr<CoqKeyStore> &);
        void register_fblock_store(std::shared_ptr<CoqFBlockStore> &);
        void register_finfo_store(std::shared_ptr<CoqFInfoStore> &);
        std::shared_ptr<CoqKeyStore> get_key_store() const;
        std::shared_ptr<CoqFBlockStore> get_fblock_store() const;
        std::shared_ptr<CoqFInfoStore> get_finfo_store() const;
        vmetric_t metrics() const;
    private:
        struct pimpl;
        std::unique_ptr<struct pimpl> _pimpl;
        CoqAssemblyCache(CoqAssemblyCache const &) = delete;
        CoqAssemblyCache & operator=(CoqAssemblyCache const &) = delete;
};

} // namespace
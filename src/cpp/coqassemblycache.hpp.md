```cpp

/*
    eLyKseeR or LXR - cryptographic data archiving software
    https://github.com/eLyKseeR/elykseer-cpp
    Copyright (C) 2023 Alexander Diemand

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

#pragma once
```

`#include` "[lxr/coqassembly.hpp](coqassembly.hpp.md)"

`#include` "[lxr/coqbuffer.hpp](coqbuffer.hpp.md)"

`#include` "[lxr/coqenvironment.hpp](coqenvironment.hpp.md)"

`#include` "[lxr/coqstore.hpp](coqstore.hpp.md)"

`#include <memory>`

`#include <variant>`


namespace [lxr](namespace.list) {

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

# struct ReadQueueEntity

{

>CoqAssembly::aid_t _aid;

>uint32_t _apos;

>uint32_t _rlen;

>uint64_t _fpos;

>Key256 _chksum{true};

};

# struct ReadQueueResult

{

>struct ReadQueueEntity _readrequest;

>std::shared_ptr&lt;[CoqBufferPlain](coqbuffer.hpp.md)&gt; _rresult{nullptr};

};

# struct WriteQueueEntity

{

>Key256 _fhash{true};

>uint64_t _fpos{0};

>std::shared_ptr&lt;[CoqBufferPlain](coqbuffer.hpp.md)&gt; _buffer{nullptr};

};

# struct WriteQueueResult

{

>struct WriteQueueEntity _writerequest;

>struct ReadQueueEntity _wresult;

};


# class CoqAssemblyCache

{

> public:

>typedef std::variant&lt;uint64_t,uint32_t,std::string&gt; mvalue_t;

>typedef std::pair&lt;std::string, mvalue_t&gt; metric_t;

>typedef std::vector&lt;metric_t&gt; vmetric_t;

>explicit [CoqAssemblyCache](coqassemblycache_ctor.cpp.md)(const [CoqConfiguration](coqconfiguration.hpp.md) &c, const int n_envs, const int depth);

>[~CoqAssemblyCache](coqassemblycache_ctor.cpp.md)();

>bool [enqueue_write_request](coqassemblycache_functions.cpp.md)(const WriteQueueEntity &);

>bool [enqueue_read_request](coqassemblycache_functions.cpp.md)(const ReadQueueEntity &);

>std::vector&lt;ReadQueueResult&gt; [iterate_read_queue](coqassemblycache_functions.cpp.md)();

>std::vector&lt;WriteQueueResult&gt; [iterate_write_queue](coqassemblycache_functions.cpp.md)();

>void [flush](coqassemblycache_functions.cpp.md)();

>void [close](coqassemblycache_functions.cpp.md)();

>void [register_key_store](coqassemblycache_functions.cpp.md)(std::shared_ptr&lt;CoqKeyStore&gt; &);

>void [register_fblock_store](coqassemblycache_functions.cpp.md)(std::shared_ptr&lt;CoqFBlockStore&gt; &);

>std::shared_ptr&lt;CoqKeyStore&gt; [get_key_store](coqassemblycache_functions.cpp.md)() const;

>std::shared_ptr&lt;CoqFBlockStore&gt; [get_fblock_store](coqassemblycache_functions.cpp.md)() const;

>vmetric_t metrics() const;

> private:

>struct pimpl;

>std::unique_ptr&lt;struct pimpl&gt; _pimpl;

>CoqAssemblyCache(CoqAssemblyCache const &) = delete;

>CoqAssemblyCache & operator=(CoqAssemblyCache const &) = delete;

};

```cpp
} // namespace
```

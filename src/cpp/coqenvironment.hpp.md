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

#include "lxr/coqassembly.hpp"
#include "lxr/coqconfiguration.hpp"

#include <memory>
#include <vector>

````

namespace [lxr](namespace.list) {

/*

```coq
Module Export Environment.

Record environment : Type :=
    mkenvironment
        { cur_assembly : AssemblyPlainWritable.H
        ; cur_buffer : AssemblyPlainWritable.B
        ; config : configuration
        ; fblocks : list (string * Assembly.blockinformation)
        ; keys : list (string * Assembly.keyinformation)
        }.

Definition initial_environment (c : configuration) : environment := ...

Definition recreate_assembly (e : environment) : environment := ...

Definition env_add_file_block (fname : string) (e : environment) (bi : Assembly.blockinformation) : environment := ...

Definition env_add_aid_key (aid : string) (e : environment) (ki : keyinformation) : environment := ...

Definition key_for_aid (e : environment) (aid : Assembly.aid_t) : option keyinformation := ...

Definition restore_assembly (e0 : environment) (aid : Assembly.aid_t) : option environment := ...

Definition finalise_assembly (e0 : environment) : environment := ...

Definition finalise_and_recreate_assembly (e0 : environment) : environment := ...

Program Definition backup (e0 : environment) (fp : string) (fpos : N) (content : BufferPlain.buffer_t) : environment := ...

End Environment.
```

*/

# class CoqEnvironment

{

>public:

>typedef std::pair&lt;std::string, CoqAssembly::BlockInformation&gt; bipair_t;

>typedef std::vector&lt;bipair_t&gt; rel_fname_fblocks;

>typedef std::pair&lt;CoqAssembly::aid_t, CoqAssembly::KeyInformation&gt; pkpair_t;

>typedef std::vector&lt;pkpair_t&gt; rel_aid_keys;

>virtual rel_fname_fblocks [extract_fblocks](coqenvironment_functions.cpp.md)() final;

>virtual rel_aid_keys [extract_keys](coqenvironment_functions.cpp.md)() final;


>protected:

>explicit [CoqEnvironment](coqenvironment_ctor.cpp.md)(const CoqConfiguration & c);

>virtual [~CoqEnvironment](coqenvironment_ctor.cpp.md)();


>virtual void [recreate_assembly](coqenvironment_functions.cpp.md)() = 0;

>virtual void [finalise_assembly](coqenvironment_functions.cpp.md)() = 0;

>virtual void [finalise_and_recreate_assembly](coqenvironment_functions.cpp.md)() = 0;

>virtual bool [restore_assembly](coqenvironment_functions.cpp.md)(const CoqAssembly::aid_t &aid, const CoqAssembly::KeyInformation & ki) = 0;

>virtual bool [backup](coqenvironment_functions.cpp.md)(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen) = 0;


>const CoqConfiguration _config;

>rel_fname_fblocks _fblocks{};

>rel_aid_keys _keys{};


>private:

>CoqEnvironment(CoqEnvironment *) = delete;

>CoqEnvironment(CoqEnvironment const &) = delete;

>CoqEnvironment & operator=(CoqEnvironment const &) = delete;

};


# class CoqEnvironmentReadable : public CoqEnvironment

{

>public:

>explicit [CoqEnvironmentReadable](coqenvironment_ctor.cpp.md)(const CoqConfiguration & c);

>virtual [~CoqEnvironmentReadable](coqenvironment_ctor.cpp.md)() = default;

>virtual bool [restore_assembly](coqenvironment_functions.cpp.md)(const CoqAssembly::aid_t &aid, const CoqAssembly::KeyInformation & ki) override final;

>std::shared_ptr&lt;CoqAssemblyPlainFull&gt; _assembly{nullptr};


>private:

>virtual void [recreate_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual void [finalise_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual void [finalise_and_recreate_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual bool [backup](coqenvironment_functions.cpp.md)(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen) override final;

};


# class CoqEnvironmentWriteable : public CoqEnvironment

{

>public:

>explicit [CoqEnvironmentWriteable](coqenvironment_ctor.cpp.md)(const CoqConfiguration & c);

>virtual [~CoqEnvironmentWriteable](coqenvironment_ctor.cpp.md)() = default;

>virtual void [recreate_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual void [finalise_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual void [finalise_and_recreate_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual bool [backup](coqenvironment_functions.cpp.md)(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen) override final;

>std::shared_ptr&lt;CoqAssemblyPlainWritable&gt; _assembly{nullptr};


>private:

>virtual bool [restore_assembly](coqenvironment_functions.cpp.md)(const CoqAssembly::aid_t &aid, const CoqAssembly::KeyInformation & ki) override;

};

```cpp
} // namespace
```

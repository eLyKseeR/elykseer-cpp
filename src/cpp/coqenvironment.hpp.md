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
```

`#pragma once`

`#include <memory>`

`#include <optional>`

`#include <vector>`

`#include` "[lxr/coqassembly.hpp](coqassembly.hpp.md)"

`#include` "[lxr/coqconfiguration.hpp](coqconfiguration.hpp.md)"

`#include` "[lxr/coqenvironment.hpp](coqenvironment.hpp.md)"


namespace [lxr](namespace.list) {

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

# class CoqEnvironment

{

>public:

>typedef std::pair&lt;std::string, [CoqAssembly::BlockInformation](coqassembly.hpp.md#struct-blockinformation)&gt; bipair_t;

>typedef std::vector&lt;bipair_t&gt; rel_fname_fblocks;

>typedef std::pair&lt;CoqAssembly::aid_t, [CoqAssembly::KeyInformation](coqassembly.hpp.md#struct-keyinformation)&gt; pkpair_t;

>typedef std::vector&lt;pkpair_t&gt; rel_aid_keys;


>protected:

>explicit [CoqEnvironment](coqenvironment_ctor.cpp.md)(const CoqConfiguration & c);

>virtual [~CoqEnvironment](coqenvironment_ctor.cpp.md)();


>virtual void [recreate_assembly](coqenvironment_functions.cpp.md)() = 0;

>virtual std::optional&lt;std::pair&lt;CoqAssembly::aid_t,CoqAssembly::KeyInformation&gt;&gt; [finalise_assembly](coqenvironment_functions.cpp.md)() = 0;

>virtual std::optional&lt;std::pair&lt;CoqAssembly::aid_t,CoqAssembly::KeyInformation&gt;&gt; [finalise_and_recreate_assembly](coqenvironment_functions.cpp.md)() = 0;

>virtual bool [restore_assembly](coqenvironment_functions.cpp.md)(const CoqAssembly::aid_t &aid, const CoqAssembly::KeyInformation & ki) = 0;

>virtual rel_fname_fblocks [backup](coqenvironment_functions.cpp.md)(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen) = 0;


>const CoqConfiguration _config;


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

>virtual std::optional&lt;std::pair&lt;CoqAssembly::aid_t,CoqAssembly::KeyInformation&gt;&gt; [finalise_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual std::optional&lt;std::pair&lt;CoqAssembly::aid_t,CoqAssembly::KeyInformation&gt;&gt; [finalise_and_recreate_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual rel_fname_fblocks [backup](coqenvironment_functions.cpp.md)(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen) override final;

};


# class CoqEnvironmentWritable : public CoqEnvironment

{

>public:

>explicit [CoqEnvironmentWritable](coqenvironment_ctor.cpp.md)(const CoqConfiguration & c);

>virtual [~CoqEnvironmentWritable](coqenvironment_ctor.cpp.md)() = default;

>virtual void [recreate_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual std::optional&lt;std::pair&lt;CoqAssembly::aid_t,CoqAssembly::KeyInformation&gt;&gt; [finalise_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual std::optional&lt;std::pair&lt;CoqAssembly::aid_t,CoqAssembly::KeyInformation&gt;&gt; [finalise_and_recreate_assembly](coqenvironment_functions.cpp.md)() override final;

>virtual rel_fname_fblocks [backup](coqenvironment_functions.cpp.md)(const std::string &fname, uint64_t fpos, const CoqBufferPlain &b, const uint32_t dlen) override final;

>std::shared_ptr&lt;CoqAssemblyPlainWritable&gt; _assembly{nullptr};


>private:

>virtual bool [restore_assembly](coqenvironment_functions.cpp.md)(const CoqAssembly::aid_t &aid, const CoqAssembly::KeyInformation & ki) override;

};

```cpp
} // namespace
```

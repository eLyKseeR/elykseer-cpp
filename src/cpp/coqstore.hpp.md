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

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

````

namespace [lxr](namespace.list) {

/*
```coq
Definition RecordStore := Type.
Record store (KVs : Type): RecordStore :=
    mkstore
        { config : configuration
        ; entries : KVs
        }.

Module Type STORE.
    Parameter K : Type.
    Parameter V : Type.
    Parameter KVs : Type.
    Parameter R : Type.
    Parameter init : configuration -> R.
    Parameter add : K -> V -> R -> R.
    Parameter find : K -> R -> option V.
End STORE.
Print STORE.

Module KeyListStore <: STORE.
    Definition K := String.string.
    Definition V := Assembly.keyinformation.
    Definition KVs := list (K * V).
    Definition R : RecordStore := store KVs.
    Definition init (c : configuration) : R := {| config := c; entries := [] |}.
    Definition add (k : K) (v : V) (r : R) : R :=
        {| config := r.(config KVs); entries := (k, v) :: r.(entries KVs) |}.
    Definition find (k : K) (r : R) : option V :=
        rec_find k r.(entries KVs).
End KeyListStore.
Print KeyListStore.

Module FBlockListStore <: STORE.
    Definition K := Assembly.aid_t.
    Definition V := Assembly.blockinformation.
    Definition KVs := list (K * V).
    Definition R : RecordStore := store KVs.
    Definition init (c : configuration) : R := {| config := c; entries := [] |}.
    Definition add (k : K) (v : V) (r : R) : R :=
        {| config := r.(config KVs); entries := (k, v) :: r.(entries KVs) |}.
    Definition find (k : K) (r : R) : option V :=
        rec_find k r.(entries KVs).
```
*/

template &lt;typename V, typename K = std::string&gt;

# class CoqStore

{

>protected:

>explicit [CoqStore](coqstore_ctor.cpp.md)(const CoqConfiguration &c);

>public:

>virtual [~CoqStore](coqstore_ctor.cpp.md)();

>int [size](coqstore_functions.cpp.md)() const;

>bool [add](coqstore_functions.cpp.md)(const K &, const V &);

>std::optional&lt;const V&gt; [find](coqstore_functions.cpp.md)(const K &) const;

>std::optional&lt;std::pair&lt;const K, const V&gt;&gt; [at](coqstore_functions.cpp.md)(int idx) const;

>std::vector&lt;std::pair&lt;K,V&gt;&gt; [list](coqstore_functions.cpp.md)() const;

>void [iterate](coqstore_functions.cpp.md)(std::function<void(const std::pair&lt;K,V&gt; &)> f) const;

>void [reset](coqstore_functions.cpp.md)();

>virtual bool [encrypted_output](coqstore_functions.cpp.md)(const std::filesystem::path &, const std::string & gpg_recipient) const = 0;

>protected:

>const CoqConfiguration _config;

>std::unordered_map&lt;K,V&gt; _entries{};

>private:

>CoqStore(CoqStore const &) = delete;

>CoqStore & operator=(CoqStore const &) = delete;

};


# class CoqKeyStore : public CoqStore&lt;CoqAssembly::KeyInformation, std::string&gt;

{

>public:

>explicit CoqKeyStore(const CoqConfiguration &c) : CoqStore(c) {}

>virtual bool [encrypted_output](coqstore_functions.cpp.md)(const std::filesystem::path &, const std::string & gpg_recipient) const override final;

>private:

>CoqKeyStore & operator=(CoqKeyStore const &) = delete;

};


# class CoqFBlockStore : public CoqStore&lt;CoqAssembly::BlockInformation, std::string&gt;

{

>public:

>explicit CoqFBlockStore(const CoqConfiguration &c) : CoqStore(c) {}

>virtual bool [encrypted_output](coqstore_functions.cpp.md)(const std::filesystem::path &, const std::string & gpg_recipient) const override final;

>private:

>CoqFBlockStore & operator=(CoqFBlockStore const &) = delete;

};

```cpp
} // namespace
```

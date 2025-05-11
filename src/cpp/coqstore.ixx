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
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

import lxr_coqassembly;
import lxr_coqconfiguration;
import lxr_coqfilesupport;
import lxr_streamio;


export module lxr_coqstore;


export namespace lxr {


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

Module FileinformationStore <: STORE.
    Definition K := String.string.
    Definition V := Filesupport.fileinformation.
    Definition KVs := list (K * V).
    Definition R : RecordStore := store KVs.
    Definition init (c : configuration) : R := {| config := c; entries := [] |}.
    Definition add (k : K) (v : V) (r : R) : R :=
        {| config := r.(config KVs); entries := (k, v) :: r.(entries KVs) |}.
    Definition find (k : K) (r : R) : option V :=
        rec_find k r.(entries KVs).

End FileinformationStore.
```
*/

template <typename V, typename K = std::string>

class CoqStore : public StreamIO
{
    protected:
        explicit CoqStore();
    public:
        virtual ~CoqStore();
        virtual int size() const final;
        virtual bool contains(const K &) const final;
        virtual bool add(const K &, const V &);
        virtual std::optional<V> find(const K &) const final;
        virtual std::optional<std::pair<const K, const V>> at(int idx) const final;
        virtual std::vector<std::pair<K,V>> list() const final;
        virtual void iterate(std::function<void(const std::pair<K, V> &)>) const final;
        virtual void reset() final;
        virtual bool encrypted_output(const std::filesystem::path &, const std::string & gpg_recipient) const final;
    protected:
        std::unordered_map<K,V> _entries{};
    private:
        CoqStore(CoqStore const &) = delete;
        CoqStore & operator=(CoqStore const &) = delete;
};


class CoqKeyStore : public CoqStore<CoqAssembly::KeyInformation, std::string>
{
    public:
        explicit CoqKeyStore() : CoqStore() {}
        virtual bool add(const std::string &, const CoqAssembly::KeyInformation &) override final;
        virtual void outStream(std::ostream & os) const override final;
        virtual void inStream(std::istream & ins) override final;
    private:
        CoqKeyStore & operator=(CoqKeyStore const &) = delete;
};


class CoqFBlockStore : public CoqStore<std::vector<CoqAssembly::BlockInformation>, std::string>
{
    public:
        explicit CoqFBlockStore() : CoqStore() {}
        bool add(const std::string &, const CoqAssembly::BlockInformation &);
        virtual void outStream(std::ostream & os) const override final;
        virtual void inStream(std::istream & ins) override final;
    private:
        virtual bool add(const std::string &, const std::vector<CoqAssembly::BlockInformation> &) override final
            { return false; }
        CoqFBlockStore & operator=(CoqFBlockStore const &) = delete;
};


class CoqFInfoStore : public CoqStore<CoqFilesupport::FileInformation, std::string>
{
    public:
        explicit CoqFInfoStore() : CoqStore() {}
        virtual bool add(const std::string &, const CoqFilesupport::FileInformation &) override final;
        virtual void outStream(std::ostream & os) const override final;
        virtual void inStream(std::istream & ins) override final;
    private:
        CoqFInfoStore & operator=(CoqFInfoStore const &) = delete;
};

} // namespace
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
#include <functional>
#include <memory>
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
   the meta data is exchanged as XML following the schemas of elykseer-ml @ e6db580 (0.10.1):
   - schema/keys.xsd:            <keys><key nchunks version aid><secret/><iv/></key>...</keys>
   - schema/fileinformation.xsd: <files><fileinformation version fhash><name/>...<fileblocks/></fileinformation>...</files>

   the keys are stored in plain text in the XML; the documents are encrypted with GnuPG.

   Store.add prepends, so find returns the newest entry. CoqFInfoStore follows this.
   CoqKeyStore refuses to replace the key of an existing aid.

from elykseer-ml @ e6db580 (0.10.1), theories/Store.v (excerpt):
```coq
Definition RecordStore := Type.
Record store (KVs : Type): RecordStore :=
    mkstore
        { sconfig : configuration
        ; entries : KVs
        }.

Fixpoint rec_find {V : Type} (k : string) (es : list (string * V)) : option V :=
    match es with
    | [] => None
    | (k', v')::r => if k' =? k then Some v'
                     else rec_find k r
    end.
Fixpoint rec_find_all {V : Type} (k : string) (es : list (string * V)) (agg : list V) : list V :=
    match es with
    | [] => agg
    | (k', v')::r => if k' =? k
                     then rec_find_all k r (v' :: agg)
                     else rec_find_all k r agg
    end.

Module Type STORE.
    Parameter K : Type.
    Parameter V : Type.
    Parameter KVs : Type.
    Parameter R : Type.
    Parameter init : configuration -> R.
    Parameter add : K -> V -> R -> R.
    Parameter find : K -> R -> option V.
    Parameter find_all : K -> R -> list V.
End STORE.

Module KeyListStore <: STORE.
    Definition K := Assembly.aid_t.
    Definition V := Assembly.keyinformation.
    Definition KVs := list (K * V).
    Definition R : RecordStore := store KVs.
    Definition init (c : configuration) : R := {| sconfig := c; entries := [] |}.
    Definition add (k : K) (v : V) (r : R) : R :=
        {| sconfig := r.(sconfig KVs); entries := (k, v) :: r.(entries KVs) |}.
    Definition find (k : K) (r : R) : option V :=
        rec_find k r.(entries KVs).
    Definition find_all (k : K) (r : R) : list V := nil.
End KeyListStore.

Module FBlockListStore <: STORE.
    Definition K := String.string.
    Definition V := Assembly.blockinformation.
    Definition KVs := list (K * V).
    Definition R : RecordStore := store KVs.
    Definition init (c : configuration) : R := {| sconfig := c; entries := [] |}.
    Definition add (k : K) (v : V) (r : R) : R :=
        {| sconfig := r.(sconfig KVs); entries := (k, v) :: r.(entries KVs) |}.
    Definition find (k : K) (r : R) : option V := None.
    Definition find_all (k : K) (r : R) : list V :=
        rec_find_all k r.(entries KVs) [].

End FBlockListStore.

Module FileinformationStore <: STORE.
    Definition K := String.string.
    Definition V := Filesupport.fileinformation.
    Definition KVs := list (K * V).
    Definition R : RecordStore := store KVs.
    Definition init (c : configuration) : R := {| sconfig := c; entries := [] |}.
    Definition add (k : K) (v : V) (r : R) : R :=
        {| sconfig := r.(sconfig KVs); entries := (k, v) :: r.(entries KVs) |}.
    Definition find (k : K) (r : R) : option V :=
        rec_find k r.(entries KVs).
    Definition find_all (k : K) (r : R) : list V := nil.

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


// encrypts the output of a StreamIO to a GnuPG recipient and writes it to a file
bool encrypted_output(const StreamIO &, const std::filesystem::path &, const std::string & gpg_recipient);


class CoqKeyStore : public CoqStore<CoqAssembly::KeyInformation, std::string>
{
    public:
        explicit CoqKeyStore() : CoqStore() {}
        virtual bool add(const std::string &, const CoqAssembly::KeyInformation &) override final;
        virtual void outStream(std::ostream & os) const override final;
        virtual void inStream(std::istream & ins) override final;
    private:
        CoqKeyStore(CoqKeyStore const &) = delete;
        CoqKeyStore & operator=(CoqKeyStore const &) = delete;
};


class CoqFBlockStore : public CoqStore<std::shared_ptr<std::vector<CoqAssembly::BlockInformation>>, std::string>
{
    public:
        explicit CoqFBlockStore() : CoqStore() {}
        virtual bool add(const std::string &, const CoqAssembly::BlockInformation &) final;
        virtual void outStream(std::ostream & os) const override final;
        virtual void inStream(std::istream & ins) override final;
    private:
        CoqFBlockStore(CoqFBlockStore const &) = delete;
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
        CoqFInfoStore(CoqFInfoStore const &) = delete;
        CoqFInfoStore & operator=(CoqFInfoStore const &) = delete;
};


// the <files> document with file information and file blocks of both stores;
// either store reads its part of such a document with inStream
class CoqFilesStream : public StreamIO
{
    public:
        explicit CoqFilesStream(const std::shared_ptr<CoqFInfoStore> &, const std::shared_ptr<CoqFBlockStore> &);
        virtual void outStream(std::ostream & os) const override final;
        virtual void inStream(std::istream & ins) override final;
    private:
        std::shared_ptr<CoqFInfoStore> _finfostore;
        std::shared_ptr<CoqFBlockStore> _fblockstore;
        CoqFilesStream(CoqFilesStream const &) = delete;
        CoqFilesStream & operator=(CoqFilesStream const &) = delete;
};

} // namespace
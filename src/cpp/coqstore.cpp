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
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"

import lxr_key256;
import lxr_gpg;

import lxr_coqassembly;
import lxr_coqconfiguration;
import lxr_coqfilesupport;
import lxr_liz;
import lxr_os;
import lxr_streamio;


module lxr_coqstore;


namespace lxr {

/*
```coq
    Definition init (c : configuration) : R := {| config := c; entries := [] |}.
```
*/

template <typename V, typename K>
CoqStore<V,K>::CoqStore()
{}

template <typename V, typename K>
CoqStore<V,K>::~CoqStore() = default;

/*
```coq
Fixpoint rec_find {V : Type} (k : string) (es : list (string * V)) : option V :=
    match es with
    | [] => None
    | (k', v')::r => if k' =? k then Some v'
                     else rec_find k r
    end.
```
*/

template <typename V, typename K>
std::optional<V> CoqStore<V,K>::find(const K &k) const
{
    if (auto const it = _entries.find(k); it != _entries.cend()) {
        return it->second;
    } else {
        return {};
    }
}

template <typename V, typename K>
std::optional<std::pair<const K, const V>> CoqStore<V,K>::at(int idx) const
{
    if (idx >= _entries.size()) {
        return {};
    }
    int i = 0;
    for (auto _entry : _entries) {
        if (i++ == idx) {
            return _entry;
        }
    }
    return {};
}

template <typename V, typename K>
int CoqStore<V,K>::size() const
{
    return _entries.size();
}

template <typename V, typename K>
bool CoqStore<V,K>::contains(const K & k) const
{
    return _entries.contains(k);
}

template <typename V, typename K>
bool CoqStore<V,K>::add(const K &k, const V &v)
{
    _entries[k] = v;
    return true;
}

template <typename V, typename K>
std::vector<std::pair<K,V>> CoqStore<V,K>::list() const
{
    std::vector<std::pair<K,V>> _res{};
    for (auto const &p : _entries) {
        _res.push_back(p);
    }
    return _res;
}

template <typename V, typename K>
void CoqStore<V,K>::iterate(std::function<void(const std::pair<K,V> &)> f) const
{
    for (auto const &p : _entries) {
        f(p);
    }
}

template <typename V, typename K>
void CoqStore<V,K>::reset()
{
    _entries = {};
}

// Output encrypted
// The entities in the store are output to an encrypted file. The encryption uses a key which is looked up in GnuPG for a specific recipient. Pass in the keys identifier.

bool encrypted_output(const StreamIO &sio, const std::filesystem::path &fp, const std::string & gpg_recipient)
{
    lxr::Gpg gpg;
    if (! gpg.has_public_key(gpg_recipient)) {
        std::clog << "Error: cannot find PGP recipient's encryption key !" << std::endl;
        return false;
    }
    sio.outStream(gpg.ostream());
    auto _enc = gpg.encrypt_to_key(gpg_recipient);
    if (_enc) {
        std::ofstream _outf; _outf.open(fp.string());
        _outf << _enc.value();
        _outf.close();
        return _outf.good();
    } else {
        std::clog << "Error: cannot encrypt Store !" << std::endl;
    }
    return false;
}

template <typename V, typename K>
bool CoqStore<V,K>::encrypted_output(const std::filesystem::path &fp, const std::string & gpg_recipient) const
{
    return lxr::encrypted_output(*this, fp, gpg_recipient);
}

// add entries

bool CoqKeyStore::add(const std::string &k, const CoqAssembly::KeyInformation &v)
{
    // never replace the key of an assembly
    if (_entries.contains(k)) { return false; }
    return CoqStore::add(k, v);
}

bool CoqFBlockStore::add(const std::string &k, const CoqAssembly::BlockInformation &v)
{
    CoqAssembly::BlockInformation entry = v;
    auto it = _entries.find(k);
    if (it != _entries.end()) {
        entry.blockid = it->second->size() + 1;
        it->second->push_back(entry);
    } else {
        entry.blockid = 1;
        auto vect = new std::vector<CoqAssembly::BlockInformation>({entry});
        return CoqStore::add(k, std::shared_ptr<std::vector<CoqAssembly::BlockInformation>>(vect));
    }
    return true;
}

bool CoqFInfoStore::add(const std::string &k, const CoqFilesupport::FileInformation &v)
{
    // the newest entry replaces an existing one (Store.add)
    return CoqStore::add(k, v);
}

// XML documents

static std::string xml_escape(const std::string &s)
{
    std::string r; r.reserve(s.size());
    for (const char c : s) {
        switch (c) {
            case '&': r += "&amp;"; break;
            case '<': r += "&lt;"; break;
            case '>': r += "&gt;"; break;
            case '"': r += "&quot;"; break;
            case '\'': r += "&apos;"; break;
            default: r += c;
        }
    }
    return r;
}

static std::optional<boost::property_tree::ptree> read_xml_document(std::istream & ins)
{
    boost::property_tree::ptree pt;
    try {
        boost::property_tree::xml_parser::read_xml(ins, pt,
          boost::property_tree::xml_parser::no_comments |
          boost::property_tree::xml_parser::trim_whitespace);
    } catch (const boost::property_tree::ptree_error & e) {
        std::clog << "cannot parse meta data: " << e.what() << std::endl;
        return {};
    }
    return pt;
}

// Key::fromHex expects exactly length()/4 lowercase hex digits and reads past the end of shorter strings
template <typename K>
static bool from_hex(K & k, const std::string & h)
{
    if (h.size() != (std::size_t)k.length() / 4) { return false; }
    for (const char c : h) {
        if (! ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) { return false; }
    }
    k.fromHex(h);
    return true;
}

static void warn_legacy(const std::string & root)
{
    if (root == "DbKey" || root == "DbFBlock" || root == "DbFInfo") {
        std::clog << "ignoring meta data <" << root << "> of elykseer-cpp < 5.0.0: archives in this format cannot be read by this version" << std::endl;
    }
}

static void outputHeader(std::ostream & os, const std::string & ty)
{
    os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    os << "<!-- " << Liz::name() << " " << Liz::version() << " on " << xml_escape(OS::hostname()) << " at " << OS::timestamp() << " -->" << std::endl;
    os << "<" << ty << ">" << std::endl;
}

// schema/keys.xsd

bool CoqKeyStore::inStream(std::istream & ins)
{
    auto const pt = read_xml_document(ins);
    if (! pt) { return false; }
    bool ok{true};
    for (auto const & root : pt.value()) {
        warn_legacy(root.first);
        if (root.first != "keys") { continue; }
        for (auto const & k : root.second) {
            if (k.first != "key") { continue; }
            lxr::CoqAssembly::KeyInformation ki;
            const std::string _aid = k.second.get<std::string>("<xmlattr>.aid", "");
            ki._localnchunks = k.second.get<int>("<xmlattr>.nchunks", 16);
            ki._version = k.second.get<std::string>("<xmlattr>.version", "");
            if (! from_hex(ki._pkey, k.second.get<std::string>("secret", "")) ||
                ! from_hex(ki._ivec, k.second.get<std::string>("iv", ""))) {
                std::clog << "invalid key record for aid " << _aid << std::endl;
                ok = false;
                continue;
            }
            if (! _aid.empty()) { add(_aid, ki); }
        }
    }
    return ok;
}

void CoqKeyStore::outStream(std::ostream & os) const
{
    outputHeader(os, "keys");
    iterate([&os](const std::pair<std::string, lxr::CoqAssembly::KeyInformation> &p) {
        os << "<key nchunks=\"" << p.second._localnchunks.u() << "\" version=\"" << xml_escape(p.second._version) << "\" aid=\"" << xml_escape(p.first) << "\">"
           << "<secret>" << p.second._pkey.toHex() << "</secret><iv>" << p.second._ivec.toHex() << "</iv></key>" << std::endl;
    });
    os << "</keys>" << std::endl;
}

// schema/fileinformation.xsd

static bool read_files_document(std::istream & ins, CoqFInfoStore *finfostore, CoqFBlockStore *fblockstore)
{
    auto const pt = read_xml_document(ins);
    if (! pt) { return false; }
    bool ok{true};
    for (auto const & root : pt.value()) {
        warn_legacy(root.first);
        if (root.first != "files") { continue; }
        for (auto const & entry : root.second) {
            if (entry.first != "fileinformation") { continue; }
            const std::string fhash = entry.second.get<std::string>("<xmlattr>.fhash", "");
            if (fhash.empty()) { continue; }
            if (finfostore && entry.second.get_child_optional("name")) {
                lxr::CoqFilesupport::FileInformation fi;
                fi.fname = entry.second.get<std::string>("name", "");
                fi.fhash = fhash;
                fi.fsize = entry.second.get<uint64_t>("size", 0);
                fi.fowner = entry.second.get<std::string>("owner", "");
                fi.fpermissions = entry.second.get<uint32_t>("permissions", 0);
                fi.fmodified = entry.second.get<std::string>("modified", "");
                fi.fchecksum = entry.second.get<std::string>("checksum", "");
                finfostore->add(fhash, fi);
            }
            if (fblockstore) {
                std::shared_ptr<std::vector<lxr::CoqAssembly::BlockInformation>> bis {new std::vector<lxr::CoqAssembly::BlockInformation>()};
                if (auto const fbs = entry.second.get_child_optional("fileblocks"); fbs) {
                    for (auto const & b : fbs.value()) {
                        if (b.first != "fileblock") { continue; }
                        lxr::CoqAssembly::BlockInformation bi;
                        bi.blockid = b.second.get<int>("<xmlattr>.blockid", 0);
                        bi.blocksize = b.second.get<uint32_t>("<xmlattr>.blocksize", 0);
                        bi.filepos = b.second.get<uint64_t>("<xmlattr>.filepos", 0);
                        bi.blockapos = b.second.get<uint32_t>("<xmlattr>.apos", 0);
                        bi.blockaid = b.second.get<std::string>("aid", "");
                        if (! from_hex(bi.bchecksum, b.second.get<std::string>("checksum", ""))) {
                            std::clog << "invalid block checksum for file " << fhash << std::endl;
                            ok = false;
                            continue;
                        }
                        bis->push_back(bi);
                    }
                }
                if (bis->size() > 0) {
                    fblockstore->CoqStore::add(fhash, bis);
                }
            }
        }
    }
    return ok;
}

static void write_files_document(std::ostream & os, const CoqFInfoStore *finfostore, const CoqFBlockStore *fblockstore)
{
    outputHeader(os, "files");
    auto write_entry = [&os, finfostore, fblockstore](const std::string & fhash) {
        os << "<fileinformation version=\"" << CoqAssembly::format_version << "\" fhash=\"" << xml_escape(fhash) << "\">" << std::endl;
        lxr::CoqFilesupport::FileInformation fi{};
        if (finfostore) {
            if (auto const ofi = finfostore->find(fhash); ofi) { fi = ofi.value(); }
        }
        os << "<name>" << xml_escape(fi.fname) << "</name>" << std::endl;
        os << "<size>" << fi.fsize << "</size>" << std::endl;
        os << "<owner>" << xml_escape(fi.fowner) << "</owner>" << std::endl;
        os << "<permissions>" << fi.fpermissions << "</permissions>" << std::endl;
        os << "<modified>" << xml_escape(fi.fmodified) << "</modified>" << std::endl;
        os << "<checksum>" << xml_escape(fi.fchecksum) << "</checksum>" << std::endl;
        os << "<fileblocks>" << std::endl;
        if (fblockstore) {
            if (auto const obis = fblockstore->find(fhash); obis) {
                for (const auto & bi : *(obis.value())) {
                    os << "<fileblock blockid=\"" << bi.blockid << "\" blocksize=\"" << bi.blocksize << "\" filepos=\"" << bi.filepos << "\" apos=\"" << bi.blockapos << "\">"
                       << "<aid>" << xml_escape(bi.blockaid) << "</aid><checksum>" << bi.bchecksum.toHex() << "</checksum></fileblock>" << std::endl;
                }
            }
        }
        os << "</fileblocks>" << std::endl;
        os << "</fileinformation>" << std::endl;
    };
    if (fblockstore) {
        fblockstore->iterate([&write_entry](const std::pair<std::string, std::shared_ptr<std::vector<lxr::CoqAssembly::BlockInformation>>> &p) {
            write_entry(p.first);
        });
    }
    if (finfostore) {
        finfostore->iterate([&write_entry, fblockstore](const std::pair<std::string, lxr::CoqFilesupport::FileInformation> &p) {
            if (! fblockstore || ! fblockstore->contains(p.first)) { write_entry(p.first); }
        });
    }
    os << "</files>" << std::endl;
}

bool CoqFBlockStore::inStream(std::istream & ins)
{
    return read_files_document(ins, nullptr, this);
}

void CoqFBlockStore::outStream(std::ostream & os) const
{
    write_files_document(os, nullptr, this);
}

bool CoqFInfoStore::inStream(std::istream & ins)
{
    return read_files_document(ins, this, nullptr);
}

void CoqFInfoStore::outStream(std::ostream & os) const
{
    write_files_document(os, this, nullptr);
}

CoqFilesStream::CoqFilesStream(const std::shared_ptr<CoqFInfoStore> &fi, const std::shared_ptr<CoqFBlockStore> &fb)
  : _finfostore(fi), _fblockstore(fb)
{}

bool CoqFilesStream::inStream(std::istream & ins)
{
    return read_files_document(ins, _finfostore.get(), _fblockstore.get());
}

void CoqFilesStream::outStream(std::ostream & os) const
{
    write_files_document(os, _finfostore.get(), _fblockstore.get());
}

// Template class instances
// instantiate template classes with the used types.

template class CoqStore<std::shared_ptr<std::vector<CoqAssembly::BlockInformation>>, std::string>;
template class CoqStore<CoqAssembly::KeyInformation, std::string>;
template class CoqStore<CoqFilesupport::FileInformation, std::string>;
template class CoqStore<std::string, std::string>;

} // namespace
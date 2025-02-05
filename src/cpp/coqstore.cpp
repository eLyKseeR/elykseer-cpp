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
#include <fstream>
#include <iostream>
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

template <typename V, typename K>
bool CoqStore<V,K>::encrypted_output(const std::filesystem::path &fp, const std::string & gpg_recipient) const
{
    lxr::Gpg gpg;
    if (! gpg.has_public_key(gpg_recipient)) {
        std::clog << "Error: cannot find PGP recipient's encryption key !" << std::endl;
        return false;
    }
    outStream(gpg.ostream());
    auto _enc = gpg.encrypt_to_key(gpg_recipient);
    if (_enc) {
        std::ofstream _outf; _outf.open(fp.string());
        _outf << _enc.value();
        _outf.close();
        return true;
    } else {
        std::clog << "Error: cannot encrypt Store !" << std::endl;
    }
    return false;
}

// add entries

bool CoqKeyStore::add(const std::string &k, const CoqAssembly::KeyInformation &v)
{
    if (_entries.contains(k)) { return false; }
    return CoqStore::add(k, v);
}

bool CoqFBlockStore::add(const std::string &k, const CoqAssembly::BlockInformation &v)
{
    auto entry = v;
    auto it = _entries.find(k);
    if (it != _entries.end()) {
        entry.blockid = it->second.size() + 1;
        it->second.push_back(std::move(entry));
    } else {
        entry.blockid = 1;
        std::vector<CoqAssembly::BlockInformation> vect{std::move(entry)};
        return CoqStore::add(k, vect);
    }
    return true;
}

bool CoqFInfoStore::add(const std::string &k, const CoqFilesupport::FileInformation &v)
{
    if (_entries.contains(k)) { return false; }
    return CoqStore::add(k, v);
}

// Stream input/output functions

void CoqKeyStore::inStream(std::istream & ins)
{
    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(ins, pt,
      boost::property_tree::xml_parser::no_comments |
      boost::property_tree::xml_parser::trim_whitespace);

    for (auto root = pt.begin(); root != pt.end(); root++) {
        if (root->first == "DbKey") {
            for (auto k = root->second.begin(); k != root->second.end(); k++) {
                if (k->first == "Key") {
                    lxr::CoqAssembly::KeyInformation ki;
                    std::string _aid = k->second.data();
                    for (auto attr = k->second.begin(); attr != k->second.end(); attr++) {
                        if (attr->first == "<xmlattr>") {
                            ki._localnchunks = attr->second.get<int>("n");
                            ki._ivec.fromHex(attr->second.get<std::string>("iv"));
                            ki._pkey.fromHex(attr->second.get<std::string>("pk"));
                        }
                    }
                    add(_aid, ki);
                }
            }
        }
    }
}

void CoqFBlockStore::inStream(std::istream & ins)
{
    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(ins, pt,
      boost::property_tree::xml_parser::no_comments |
      boost::property_tree::xml_parser::trim_whitespace);

    for (auto root = pt.begin(); root != pt.end(); root++) {
        if (root->first == "DbFBlock") {
            for (auto fh = root->second.begin(); fh != root->second.end(); fh++) {
                if (fh->first == "File") {
                    std::vector<lxr::CoqAssembly::BlockInformation> bis;
                    std::string fhash{""};
                    for (auto b = fh->second.begin(); b != fh->second.end(); b++) {
                        if (b->first == "FBlock") {
                            lxr::CoqAssembly::BlockInformation bi;
                            bi.blockaid = b->second.data();
                            for (auto attr = b->second.begin(); attr != b->second.end(); attr++) {
                                if (attr->first == "<xmlattr>") {
                                    bi.blockid = attr->second.get<int>("idx");
                                    bi.blockapos = attr->second.get<uint32_t>("apos");
                                    bi.filepos = attr->second.get<uint64_t>("fpos");
                                    bi.blocksize = attr->second.get<uint32_t>("blen");
                                    // bi. = attr->second.get<int>("clen");
                                    // bi. = attr->second.get<int>("compressed");
                                    bi.bchecksum.fromHex(attr->second.get<std::string>("chksum"));
                                }
                            }
                            bis.push_back(bi);
                        }
                        else if (b->first == "<xmlattr>") {
                            fhash = b->second.get<std::string>("fhash");
                        }
                    }
                    if (fhash != "" && bis.size() > 0) {
                        CoqStore::add(fhash, bis);
                    }
                }
            }
        }
    }
}

void CoqFInfoStore::inStream(std::istream & ins)
{
    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(ins, pt,
      boost::property_tree::xml_parser::no_comments |
      boost::property_tree::xml_parser::trim_whitespace);

    for (auto root = pt.begin(); root != pt.end(); root++) {
        if (root->first == "DbFInfo") {
            for (auto entry = root->second.begin(); entry != root->second.end(); entry++) {
                if (entry->first == "FInfo") {
                    lxr::CoqFilesupport::FileInformation fi;
                    std::string _fhash = entry->second.data();
                    for (auto attr = entry->second.begin(); attr != entry->second.end(); attr++) {
                        if (attr->first == "<xmlattr>") {
                            fi.fname = attr->second.get<std::string>("fname");
                            fi.fsize = attr->second.get<uint64_t>("fsize");
                            fi.fowner = attr->second.get<std::string>("fowner");
                            fi.fpermissions = attr->second.get<int>("fpermissions");
                            fi.fmodified = attr->second.get<std::string>("fmodified");
                            fi.fchecksum = attr->second.get<std::string>("fchecksum");
                        }
                    }
                    add(_fhash, fi);
                }
            }
        }
    }
}

static void outputHeader(std::ostream & os, const std::string & ty)
{
    os << "<?xml version=\"1.0\"?>" << std::endl;
    os << "<" << ty << " xmlns=\"http://spec.elykseer.com/lxr/v1.0\">" << std::endl;
    os << "  <library><name>" << Liz::name() << "</name><version>" << Liz::version() << "</version></library>" << std::endl;
    os << "  <host>" << OS::hostname() << "</host>" << std::endl;
    os << "  <user>" << OS::username() << "</user>" << std::endl;
    os << "  <date>" << OS::timestamp() << "</date>" << std::endl;
}

void CoqKeyStore::outStream(std::ostream & os) const
{
    outputHeader(os, "DbKey");

    iterate([&os](const std::pair<std::string, lxr::CoqAssembly::KeyInformation> &p) {
        os << "  <Key n=\"" << p.second._localnchunks.u() << "\" iv=\"" << p.second._ivec.toHex() << "\" pk=\"" << p.second._pkey.toHex() << "\">" << p.first << "</Key>" << std::endl;
    });
    os << "</DbKey>" << std::endl;
}

void CoqFBlockStore::outStream(std::ostream & os) const
{
    outputHeader(os, "DbFBlock");

    iterate([&os](const std::pair<std::string, std::vector<lxr::CoqAssembly::BlockInformation>> &p) {
        os << "  <File fhash=\"" << p.first << "\">" << std::endl;
        for (const auto & bi : p.second) {
            os << "    <FBlock idx=\"" << bi.blockid << "\" apos=\"" << bi.blockapos << "\" fpos=\"" << bi.filepos << "\" blen=\"" << bi.blocksize << "\" clen=\"" << 0 << "\" compressed=\"" << 0 << "\" chksum=\"" << bi.bchecksum.toHex() << "\">" << bi.blockaid << "</FBlock>" << std::endl;
        }
        os << "  </File>" << std::endl;
    });
    os << "</DbFBlock>" << std::endl;
}

void CoqFInfoStore::outStream(std::ostream & os) const
{
    outputHeader(os, "DbFInfo");

    iterate([&os](const std::pair<std::string, lxr::CoqFilesupport::FileInformation> &p) {
        os << "    <FInfo fname=\"" << p.second.fname << "\" fsize=\"" << p.second.fsize << "\" fowner=\"" << p.second.fowner << "\" fpermissions=\"" << p.second.fpermissions << "\" fmodified=\"" << p.second.fmodified << "\" fchecksum=\"" << p.second.fchecksum << "\">" << p.first << "</FInfo>" << std::endl;
    });
    os << "</DbFInfo>" << std::endl;
}

// Template class instances
// instantiate template classes with the used types.

template class CoqStore<std::vector<CoqAssembly::BlockInformation>, std::string>;
template class CoqStore<CoqAssembly::KeyInformation, std::string>;
template class CoqStore<CoqFilesupport::FileInformation, std::string>;
template class CoqStore<std::string, std::string>;

} // namespace
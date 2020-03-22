declared in [BackupCtrl](backupctrl.hpp.md)

```cpp
void RestoreCtrl::addDbFp(DbFp const & _db) {
    _pimpl->_dbfp.unionWith(_db);
}
```

```cpp
void RestoreCtrl::addDbKey(DbKey const & _db) {
    _pimpl->_dbkey.unionWith(_db);
}
```

```cpp
bool RestoreCtrl::pimpl::load_assembly(Key256 const & _aid)
{
    auto const & said = _aid.toHex();
    if (_ass && _ass->said() == said) {
        return true;  // already in scope
    }
    auto dbkey = _dbkey.get(said);
    if (! dbkey) {
        std::cerr << "cannot load keys for aid=" << said << std::endl;
        return false;
    }
    _ass.reset(new Assembly(_aid, Options::current().nChunks()));
    if (! _ass->insertChunks()) {
        std::cerr << "cannot insert chunks into aid=" << said << std::endl;
        return false;
    }
    if (! _ass->decrypt(dbkey->_key, dbkey->_iv)) {
        std::cerr << "failed to decrypt aid=" << said << std::endl;
        return false;
    }

    return _ass->isReadable();
}
```

Datastructures used in streaming data
```cpp
class configuration {
  public:
    configuration() {};
    ~configuration() {};
};

class state {
  public:
    state() {};
    ~state() {};
    uint64_t& trx_in() { return _trx_in; }
    uint64_t& trx_out() { return _trx_out; }
  private:
    uint64_t _trx_in {0UL};
    uint64_t _trx_out {0UL};
};
```

```cpp
template <typename Ct, typename St, typename Vt, int sz>
int lxr::RestoreCtrl::pimpl::restore_block( DbFpBlock const &block
                        , sizebounded<Vt, sz> &buffer
                        , inflatestream<Ct,St,Vt,sz> &decomp )
{
    if (! load_assembly(block._aid)) {
        std::cerr << "assembly not available" << std::endl;
        return -1;
    }
    int trsz = block._clen;
    if (!block._compressed) { trsz = block._blen; }
    _ass->getData(block._apos, trsz, buffer);
    // decompress
    int nwritten = block._blen;
    if (block._compressed) {
        if ((nwritten = decomp.process(nullptr, nullptr, trsz, buffer)) != block._blen) {
            std::cerr << "decompression returned " << nwritten << " bytes; != " << block._blen << std::endl;
            return -2;
        }
    }
    // check checksum
    auto const md5 = Md5::hash((const char *)buffer.ptr(), block._blen);
    if (md5.operator!=(block._checksum)) {
        std::cerr << "checksum wrong for block " << block._idx << std::endl;
        return -3;
    }
    return block._blen;
}
```

The _restore_ function that decrypts a single file from the archive and outputs
it in the indicated directory.
```cpp
bool RestoreCtrl::restore(boost::filesystem::path const & root, std::string const & fp)
{
    // checkout output directory
    if (! FileCtrl::dirExists(root)) {
        std::cerr << "output directory does not exist: " << root << std::endl;
        return false;
    }
    // check output file does not exist
    auto targetfp = root + fp;
    if (FileCtrl::fileExists(targetfp)) {
        std::cerr << "output file already exists: " << fp << std::endl;
        return false;
    }
    
    // get entry from DbFp
    auto dbfp = _pimpl->_dbfp.get(fp);
    if (! dbfp) {
        std::cerr << "db entry not found: " << fp << std::endl;
        return false;
    }

    // stream for decompressing data
    constexpr int bsz = Assembly::datasz;
    configuration _config;
    state _state;
    inflatestream decomp = inflatestream<configuration,state,unsigned char,bsz>(&_config, &_state, nullptr, nullptr);
    sizebounded<unsigned char, bsz> buffer;

    // load blocks
    bool res = true;
    int trsz = 0;
    std::ofstream _fout; _fout.open(targetfp.native());
    for (auto const & block : dbfp->_blocks) {
        if ((trsz = _pimpl->restore_block<configuration,state,unsigned char,bsz>(block, buffer, decomp)) < 0) {
            std::cerr << "failed to restore block: " << block._idx << std::endl;
            res = false;
            break;
        }
    }
    _fout.close();

    return res;
}

```

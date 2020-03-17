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
bool RestoreCtrl::load_assembly(Key256 const & _aid)
{
    auto const & said = _aid.toHex();
    if (_pimpl->_ass && _pimpl->_ass->said() == said) {
        return true;  // already in scope
    }
    auto dbkey = _pimpl->_dbkey.get(said);
    if (! dbkey) {
        std::cerr << "cannot load keys for aid=" << said << std::endl;
        return false;
    }
    _pimpl->_ass.reset(new Assembly(_aid, Options::current().nChunks()));
    if (! _pimpl->_ass->insertChunks()) {
        std::cerr << "cannot insert chunks into aid=" << said << std::endl;
        return false;
    }
    if (! _pimpl->_ass->decrypt(dbkey->_key, dbkey->_iv)) {
        std::cerr << "failed to decrypt aid=" << said << std::endl;
        return false;
    }

    return _pimpl->_ass->isReadable();
}
```

```cpp
bool RestoreCtrl::restore_block(std::ofstream & fout, DbFpBlock const & block)
{
    if (! load_assembly(block._aid)) {
        std::cerr << "assembly not available" << std::endl;
        return false;
    }

    return true;
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

    // load blocks
    bool res = true;
    std::ofstream _fout; _fout.open(targetfp);
    for (auto const & block : dbfp->_blocks) {
        if (! restore_block(_fout, block)) {
            std::cerr << "failed to restore block: " << block._idx << std::endl;
            res = false;
            break;
        }
    }
    _fout.close();

    return res;
}

```

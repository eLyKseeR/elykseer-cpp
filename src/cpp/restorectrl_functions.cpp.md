declared in [RestoreCtrl](restorectrl.hpp.md)

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

## access an [Assembly](assembly.hpp.md)
```cpp
bool RestoreCtrl::pimpl::load_assembly(std::string const & said)
{
    if (_ass && _ass->said() == said) {
        return true;  // already in scope
    }
    auto const dbkey = _dbkey.get(said);
    if (! dbkey) {
        std::cerr << "cannot load keys for aid=" << said << std::endl;
        return false;
    }
    Key256 aid{true};
    aid.fromHex(said);
    _ass.reset(new Assembly(aid, dbkey->_n));
    auto t0 = clk::now();
    if (! _ass->insertChunks()) {
        std::cerr << "cannot insert chunks into aid=" << said << std::endl;
        return false;
    }
    auto t1 = clk::now();
    if (! _ass->decrypt(dbkey->_key, dbkey->_iv)) {
        std::cerr << "failed to decrypt aid=" << said << std::endl;
        return false;
    }
    auto t2 = clk::now();
    time_read = time_read + std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
    time_decr = time_decr + std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

#ifdef DEBUG
    { auto const tmpd = boost::filesystem::temp_directory_path();
      std::ofstream _fe; _fe.open((tmpd / std::string("test_assembly.restored")).native());
      const int bufsz = 4096;
      sizebounded<unsigned char, bufsz> buf;
      for (int i=0; i<Options::current().nChunks()*Chunk::size; i+=bufsz) {
          _ass->getData(i, i+bufsz-1, buf);
          _fe.write((const char*)buf.ptr(),bufsz); }
      _fe.close();
    }
#endif

    return _ass->isReadable();
}
```

## datastructures used in streaming data
```cpp
class configuration {
  public:
    configuration() {};
};

class state {
  public:
    state() {};
    uint64_t& trx_in() { return _trx_in; }
    void trx_in(int v) { _trx_in += v; }
    uint64_t& trx_out() { return _trx_out; }
    void trx_out(int v) { _trx_out += v; }
  private:
    state(state const &) = delete;
    uint64_t _trx_in {0UL};
    uint64_t _trx_out {0UL};
};
```

## restore a single block
```cpp
template <typename Ct, typename St, typename Vt, int sz>
int lxr::RestoreCtrl::pimpl::restore_block( DbFpBlock const &block
                        , sizebounded<Vt, sz> &buffer
                        , St &st
                        , std::ofstream &fout
                        , inflatestream<Ct,St,Vt,sz> &decomp )
{
#ifdef DEBUG
    std::cout << " restore block " << block._idx << " @ " << block._fpos << " len=" << block._blen << " clen=" << block._clen << std::endl;
    std::cout << "      in assembly: " << block._aid << " @ " << block._apos << std:: endl;
#endif
    if (! load_assembly(block._aid)) {
        std::cerr << "assembly not available" << std::endl;
        return -1;
    }
    int trsz = block._clen;
    if (!block._compressed) { trsz = block._blen; }
    auto t0 = clk::now();
    if (int checkback = _ass->getData(block._apos, block._apos + trsz - 1, buffer);
        checkback != trsz) {
        std::cerr << "wrong size of data returned! block " << block._idx << " @ " << block._fpos << " len=" << block._blen << " clen=" << block._clen << " returned=" << checkback << std::endl;
        std::cerr << "was accessing [" << block._apos << "," << block._apos+trsz-1 << "]" << std::endl;
        return -2;
    }
    // decompress
    int nread = trsz;
    if (block._compressed) {
        if ((nread = decomp.process(nullptr, nullptr, trsz, buffer)) != block._blen) {
            std::cerr << "decompression returned " << nread << " bytes; != " << block._blen << std::endl;
            return -3;
        }
    }
    auto t1 = clk::now();

    // check checksum
    auto const md5 = Md5::hash((const char *)buffer.ptr(), nread).toHex();
    if (md5 != (block._checksum)) {
        std::cerr << "checksum wrong for block " << block._idx << " len = " << nread << std::endl;
        std::cerr << " got:" << md5 << " expected:" << block._checksum << std::endl;
        std::cerr << "was accessing [" << block._apos << "," << block._apos+trsz-1 << "]" << std::endl;
        return -4;
    }

    // output bytes to file
    fout.write((const char *)buffer.ptr(), nread);
    auto t2 = clk::now();
    time_decomp = time_decomp + std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
    time_write = time_write + std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

    // set state
    st.trx_in(trsz);
    st.trx_out(nread);
    return nread;
}
```

## restore a file
The _restore_ function that decrypts a single file from the archive and outputs
it in the indicated directory.
```cpp
bool RestoreCtrl::restore(boost::filesystem::path const & root, std::string const & fp)
{
    auto time_begin = clk::now();

    // checkout output directory
    if (! FileCtrl::dirExists(root)) {
        std::cerr << "output directory does not exist: " << root << std::endl;
        return false;
    }
    // check output file does not exist
    auto targetfp = root / fp;
    if (FileCtrl::fileExists(targetfp)) {
        std::cerr << "output file already exists: " << targetfp << std::endl;
        return false;
    }
    // checkout output directory exists
    auto targetdir = targetfp;
    targetdir.remove_filename();
    if (! FileCtrl::dirExists(targetdir)) {
        boost::filesystem::create_directories(targetdir);
    }
    if (! FileCtrl::dirExists(targetdir)) {
        std::cerr << "output directory does not exist: " << targetdir << std::endl;
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
    std::ofstream _fout; _fout.open(targetfp.native());
    if (! _fout.good()) {
        std::cerr << "failed to open output file " << targetfp.native() << std::endl;
        _fout.close();
        return false;
    }
    for (auto const & block : *dbfp->_blocks) {
        if ((_pimpl->restore_block<configuration,state,unsigned char,bsz>(block, buffer, _state, _fout, decomp)) < 0) {
            std::cerr << "failed to restore block: " << block._idx << std::endl;
            res = false;
            break;
        }
    }
    _fout.close();
    _pimpl->trx_in += _state.trx_in();
    _pimpl->trx_out += _state.trx_out();

    auto time_end = clk::now();
    _pimpl->time_restore = _pimpl->time_restore + std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_begin);

#ifdef DEBUG
    std::cout << "restored to '" << targetfp.native() << "' in:" << _state.trx_in() << " out:" << _state.trx_out() << std::endl;
#endif
    return res;
}

```

## verify restoration of a file
The _verify_ function reproduces a single file from the archive block by block
and outputs whether they all are valid.
```cpp
bool RestoreCtrl::verify(std::string const & fp)
{
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
    std::ofstream _fout; _fout.open("/dev/null");
    if (! _fout.good()) {
        std::cerr << "failed to open /dev/null" << std::endl;
        _fout.close();
        return false;
    }
    for (auto const & block : *dbfp->_blocks) {
        if ((_pimpl->restore_block<configuration,state,unsigned char,bsz>(block, buffer, _state, _fout, decomp)) < 0) {
            std::cerr << "failed to restore block: " << block._idx << std::endl;
            res = false;
            break;
        }
    }
    _fout.close();
    _pimpl->trx_in += _state.trx_in();
    _pimpl->trx_out += _state.trx_out();

    return res;
}

```

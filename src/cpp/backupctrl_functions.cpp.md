declared in [BackupCtrl](backupctrl.hpp.md)

```cpp
void BackupCtrl::addReference(DbFp const &_db)
{
  _pimpl->_dbfpref.unionWith(_db);
}

void BackupCtrl::finalize()
{
    if (_pimpl->_ass && _pimpl->_ass->pos() > 0) {
        lxr::Key256 _k;
        lxr::Key128 _iv;
        DbKeyBlock keyblock;
        if (! _pimpl->_ass->encrypt(_k, _iv)) { return; }
        keyblock._n = _pimpl->_nChunks;
        keyblock._key = _k;
        keyblock._iv = _iv;
        _pimpl->_dbkey.set(_pimpl->_ass->said(), std::move(keyblock));
        _pimpl->_ass->extractChunks();
    }
}
```

```cpp
class configuration {
  public:
    configuration(int nc, bool iscmp)
     : _nchunks(nc), _compress(iscmp) {};
    ~configuration() {};
    int nchunks() const { return _nchunks; }
    int compress() const { return _compress; }

  private:
    int _nchunks{256};
    bool _compress{false};
};

class state {
  public:
    state (configuration const *c, DbFpDat *d) : _config(c), _dbentry(d) {};
    ~state () {};
    int next_bidx() {return ++_bidx;}
    void setcompressed(bool c) {_iscompressed = c;}
    bool iscompressed() const {return _iscompressed;}
    size_t fpos() const {return _fpos;}
    void nread(int n) {_nread += n; _lastread = n; _fpos += n;}
    int nread() const {return _nread;}
    int lastread() const {return _lastread;}
    void ncompressed(int n) {_ncompressed += n; _lastcompressed = n;}
    int ncompressed() const {return _ncompressed;}
    int lastcompressed() const {return _lastcompressed;}
    void nwritten(int n) {_nwritten += n; _lastwritten = n;}
    int nwritten() const {return _nwritten;}
    int lastwritten() const {return _lastwritten;}
    void lastchecksum(const std::string &k) {_md5 = k;}
    const std::string lastchecksum() const {return _md5;}
    DbFpDat* dbentry() const {return _dbentry;}
    void dbkeys(DbKey *db) {_dbkeys=db;}
    DbKey* dbkeys() const {return _dbkeys;}
    void assembly(std::shared_ptr<Assembly> &a) {_assembly=a;}
    std::shared_ptr<Assembly>& assembly() {return _assembly;}
    bool renew_assembly() {
        auto t0 = clk::now();
        if (_assembly && _assembly->pos() > 0) {
            lxr::Key256 _k;
            lxr::Key128 _iv;
            DbKeyBlock keyblock;
            auto t0 = clk::now();
            if (! _assembly->encrypt(_k, _iv)) { return false; }
            auto t1 = clk::now();
            keyblock._n = _config->nchunks();
            keyblock._key = _k;
            keyblock._iv = _iv;
            _dbkeys->set(_assembly->said(), std::move(keyblock));
            if (! _assembly->extractChunks()) { return false; }
            auto t2 = clk::now();
            time_encr += std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
            time_extract += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        }
        _assembly.reset(new Assembly(_config->nchunks()));
        return true;
    }
    std::chrono::microseconds time_encr {0};
    std::chrono::microseconds time_extract {0};
    std::chrono::microseconds time_compress {0};

  private:
    configuration const *_config;
    std::shared_ptr<Assembly> _assembly;
    DbKey *_dbkeys;
    DbFpDat *_dbentry;
    bool _iscompressed{false};
    size_t _fpos{0}; // pos in file
    std::string _md5;
    int _bidx{-1}; // block index
    int _nread{0}; // 1: read from file
    int _ncompressed{0}; // 2: compressed size
    int _nwritten{0}; // 3: written to assembly
    int _lastread{0};
    int _lastcompressed{0};
    int _lastwritten{0};
};
```

A stream that computes a hash over each buffer passed to them
```cpp
template <typename Ct, typename St, typename Vt, int sz>
class md5stream : public stream<Ct,St,Vt,sz>
{
  public:
    md5stream(Ct const * const c, St *st, stream<Ct,St,Vt,sz> *s, stream<Ct,St,Vt,sz> *t)
        : stream<Ct,St,Vt,sz>(c,st,s,t) {};
    virtual int process(Ct const * const c, St *st, int len, sizebounded<Vt,sz> &b) const override {
      if (len <= 0) { return len; }
      auto md5 = Md5::hash((const char *)b.ptr(), len).toHex();
      //std::cout << "   len = " << len << " chksum = " << md5 << std::endl;
      st->lastchecksum(md5);
      st->nread(len);
      return len;
    }
};
```

A stream that compresses each buffer and passes it on
```cpp
template <typename Ct, typename St, typename Vt, int sz>
class compressstream : public stream<Ct,St,Vt,sz>
{
  public:
    compressstream(Ct const * const c, St *st, stream<Ct,St,Vt,sz> *s, stream<Ct,St,Vt,sz> *t)
        : stream<Ct,St,Vt,sz>(c,st,s,t), lzs(c, st, nullptr, nullptr) {}
    virtual int process(Ct const * const c, St *st, int len, sizebounded<Vt,sz> &b) const override {
      if (len <= 0) { return len; }

      auto t0 = clk::now();
      int compressedlen = lzs.process(nullptr,nullptr,len,b);
      // std::cout << "  compressed " << len << " -> " << compressedlen << std::endl;
      if (compressedlen <= 0) { return -1; }
      auto t1 = clk::now();
      st->time_compress += std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
      st->setcompressed(true);
      st->ncompressed(compressedlen);
      return compressedlen;
    }
  private:
    deflatestream<Ct,St,Vt,sz> lzs;
};
```

This stream outputs the incoming buffer data to an Assembly and does some bookkeeping
```cpp
template <typename Ct, typename St, typename Vt, int sz>
class assemblystream : public stream<Ct,St,Vt,sz>
{
  public:
    assemblystream(Ct const * const c, St *st, stream<Ct,St,Vt,sz> *s, stream<Ct,St,Vt,sz> *t)
        : stream<Ct,St,Vt,sz>(c,st,s,t) {};
    virtual int process(Ct const * const c, St *st, int len, sizebounded<Vt,sz> &b) const override {
        if (len <= 0) { return len; }
        int _afree = st->assembly()->free();
        if (_afree < len) {
          st->renew_assembly();
        }
        int _apos = st->assembly()->pos();
        int _fpos = st->fpos() - st->lastread();
        int nwritten = st->assembly()->addData(len, b);
        st->nwritten(nwritten);
        auto dbblock = DbFpBlock(
            st->next_bidx(),
            _apos,
            _fpos,
            // blen, clen
            st->lastread(), st->lastcompressed(),
            st->iscompressed(),
            st->lastchecksum(),
            st->assembly()->said()
            );
        st->dbentry()->_blocks->push_back(std::move(dbblock));
        if (nwritten != len) {
            std::cout << "     only written: " << nwritten << " of len: " << len << std::endl;
        }
        return nwritten;
    };
};
```

The backup procedure:
```cpp
bool BackupCtrl::backup(boost::filesystem::path const & fp)
{
    if (! FileCtrl::fileExists(fp)) { return false; }

    auto time_begin = clk::now();

    // make DbFp entry; calculates file checksum (SHA256)
    auto t_dbentry = DbFpDat::fromFile(fp);

    if (Options::current().isDeduplicated() > 0) {
      auto fpref = _pimpl->_dbfpref.get(fp.native());
      if (fpref && fpref->_checksum == t_dbentry._checksum) {
        std::clog << "deduplication: skipping redundant file " << fp << std::endl;
        auto time_end = clk::now();
        _pimpl->time_backup += std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_begin);
        return true;
      }
    }

    _pimpl->_nChunks = Options::current().nChunks();
    if (! _pimpl->_ass) { _pimpl->_ass.reset(new Assembly(_pimpl->_nChunks)); }

    constexpr int bsz = Assembly::datasz;
    sizebounded<unsigned char, bsz> buffer;
    FILE *fptr = fopen(fp.native().c_str(), "r");
    if (! fptr) { return false; }

    // setup pipeline
    bool compressed = Options::current().isCompressed();
    configuration config(_pimpl->_nChunks, compressed);
    state st(&config, &t_dbentry);
    st.dbkeys(&_pimpl->_dbkey);
    st.assembly(_pimpl->_ass);
    constexpr int dwidth = 1; // bytes read at once
    int margin = 0;
    if (compressed) { margin = 64; };
    const int readsz = bsz / dwidth - margin;
    assemblystream<configuration,state,unsigned char,bsz> s3(&config, &st, nullptr, nullptr);
    stream<configuration,state,unsigned char,bsz> *tgt = &s3;
    compressstream<configuration,state,unsigned char,bsz> s2(&config, &st, nullptr, &s3);
    if (compressed) {
      tgt = &s2;
    }
    md5stream<configuration,state,unsigned char,bsz> s1(&config, &st, nullptr, tgt);

    while (! feof(fptr)) {
        auto t0 = clk::now();
        size_t nread = fread((void*)buffer.ptr(), dwidth, readsz, fptr);
        auto t1 = clk::now();
        _pimpl->time_read += std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        s1.push(nread*dwidth, buffer);
    }
    fclose(fptr);

    _pimpl->trx_in += st.nread();
    _pimpl->trx_out += st.nwritten();
    _pimpl->_ass = st.assembly();
    _pimpl->time_encr = st.time_encr;
    _pimpl->time_extract = st.time_extract;
    _pimpl->time_compress = st.time_compress;

#ifdef DEBUG
    { auto const tmpd = boost::filesystem::temp_directory_path();
      std::ofstream _fe; _fe.open((tmpd / std::string("test_assembly.backuped")).native());
      const int bufsz = 4096;
      sizebounded<unsigned char, bufsz> buf;
      for (int i=0; i<Options::current().nChunks()*Chunk::size; i+=bufsz) {
          _pimpl->_ass->getData(i, i+bufsz-1, buf);
          _fe.write((const char*)buf.ptr(),bufsz); }
      _fe.close();
    }
#endif

    _pimpl->_dbfp.set(fp.native(), std::move(t_dbentry));
    auto time_end = clk::now();
    _pimpl->time_backup += std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_begin);

    return true;
}

```

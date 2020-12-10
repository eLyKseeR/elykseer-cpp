declared in [BackupCtrl](backupctrl.hpp.md)

```cpp
void BackupCtrl::setReference()
{
 // TODO
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
        if (_assembly && _assembly->pos() > 0) {
            lxr::Key256 _k;
            lxr::Key128 _iv;
            DbKeyBlock keyblock;
            if (! _assembly->encrypt(_k, _iv)) { return false; }
            keyblock._n = _config->nchunks();
            keyblock._key = _k;
            keyblock._iv = _iv;
            _dbkeys->set(_assembly->said(), std::move(keyblock));
            if (! _assembly->extractChunks()) { return false; }
        }
        _assembly.reset(new Assembly(_config->nchunks()));
        return true;
    }

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

template <typename Ct, typename St, typename Vt, int sz>
class md5stream : public stream<Ct,St,Vt,sz>
{
  public:
    md5stream(Ct const * const c, St *st, stream<Ct,St,Vt,sz> *s, stream<Ct,St,Vt,sz> *t)
        : stream<Ct,St,Vt,sz>(c,st,s,t) {};
    virtual int process(Ct const * const c, St *st, int len, sizebounded<Vt,sz> &b) const override {
      if (len <= 0) { return 0; } 
      auto md5 = Md5::hash((const char *)b.ptr(), len).toHex();
      //std::cout << "   len = " << len << " chksum = " << md5 << std::endl;
      st->lastchecksum(md5);
      st->nread(len);
      return len;
    }
};

template <typename Ct, typename St, typename Vt, int sz>
class compressstream : public stream<Ct,St,Vt,sz>
{
  public:
    compressstream(Ct const * const c, St *st, stream<Ct,St,Vt,sz> *s, stream<Ct,St,Vt,sz> *t)
        : stream<Ct,St,Vt,sz>(c,st,s,t), lzs(c, st, nullptr, nullptr) {}
        // , _tgt(t), _config(c), _state(st) {};
    // virtual void push(int len, sizebounded<Vt,sz> &b) const override {
    //   if (len == 0) { return; } 
    //   if (_tgt) {
    //     int len2 = process(_config, _state, len, b);
    //     if (len2 > sz || len2 < 0) {
    //       std::cerr << "Error: compression returned len = " << len2 << ", orig len = " << len << std::endl;
    //     } else {
    //       _state->setcompressed(true);
    //       _state->ncompressed(len2);
    //       _tgt->push(len2, b);
    //     }
    //   } else {
    //     process(_config, _state, len, b);
    //   }
    // }
    virtual int process(Ct const * const c, St *st, int len, sizebounded<Vt,sz> &b) const override {
      if (len <= 0) { return 0; }
      // return the same size if not compressed
      if (! c->compress()) { return len; }

      //const int _afree = st->assembly()->free();
      //if (_afree < sz) {
      //  st->setcompressed(false);
      //  st->ncompressed(len);
      //} else {
        int compressedlen = lzs.process(nullptr,nullptr,len,b);
        // std::cout << "  compressed " << len << " -> " << compressedlen << std::endl;
        if (compressedlen <= 0) { return -1; }
        st->setcompressed(true);
        st->ncompressed(compressedlen);
        return compressedlen;
      //}
      //return len;
    }
  private:
    deflatestream<Ct,St,Vt,sz> lzs;
    // stream<Ct,St,Vt,sz> *_tgt;
    // Ct const * const _config;
    // St * _state;
};

template <typename Ct, typename St, typename Vt, int sz>
class assemblystream : public stream<Ct,St,Vt,sz>
{
  public:
    assemblystream(Ct const * const c, St *st, stream<Ct,St,Vt,sz> *s, stream<Ct,St,Vt,sz> *t)
        : stream<Ct,St,Vt,sz>(c,st,s,t) {};
    virtual int process(Ct const * const c, St *st, int len, sizebounded<Vt,sz> &b) const override {
        if (len <= 0) { return 0; }
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

bool BackupCtrl::backup(boost::filesystem::path const & fp)
{
    if (! FileCtrl::fileExists(fp)) { return false; }

    _pimpl->_nChunks = Options::current().nChunks();
    if (! _pimpl->_ass) { _pimpl->_ass.reset(new Assembly(_pimpl->_nChunks)); }

    constexpr int bsz = Assembly::datasz;
    sizebounded<unsigned char, bsz> buffer;
    FILE *fptr = fopen(fp.native().c_str(), "r");
    if (! fptr) { return false; }

    // make DbFp entry
    auto t_dbentry = DbFpDat::fromFile(fp);

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
        _pimpl->time_read = _pimpl->time_read + std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        s1.push(nread*dwidth, buffer);
    }
    fclose(fptr);

    _pimpl->trx_in += st.nread();
    _pimpl->trx_out += st.nwritten();
    _pimpl->_ass = st.assembly();

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

    return true;
}

```

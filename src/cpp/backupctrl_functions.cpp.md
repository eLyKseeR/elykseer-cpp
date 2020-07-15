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
    configuration(int nc) : _nchunks(nc) {};
    ~configuration() {};
    int nchunks() const { return _nchunks; }

  private:
    int _nchunks{256};
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
    void lastchecksum(const Key128 &k) {_md5 = k;}
    Key128 lastchecksum() const {return _md5;}
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
    Key128 _md5;
    int _bidx{-1}; // block index
    int _nread{0}; // 1: read from file
    int _ncompressed{0}; // 2: compressed size
    int _nwritten{0}; // 3: written to assembly
    int _lastread{0};
    int _lastcompressed{0};
    int _lastwritten{0};
};

template <typename Ct, typename St, typename Vt, int sz>
class compressstream : protected stream<Ct,St,Vt,sz>
{
  public:
    compressstream(Ct const * const c, St *st, stream<Ct,St,Vt,sz> *s, stream<Ct,St,Vt,sz> *t)
        : stream<Ct,St,Vt,sz>(st,nullptr,nullptr), lzs(c, st, nullptr, nullptr)
        , _tgt(t), _config(c), _state(st) {};
    virtual void push(int len, sizebounded<Vt,sz> &b) const override {
      if (len == 0) { return; } 
      sizebounded<Vt,sz> bsave;
      memcopy(bsave, b, sz);
      auto md5 = Md5::hash((const char *)bsave.ptr(), len);
      _state->lastchecksum(md5);
      int len2 = process(_config, _state, len, b);
      if (_tgt) {
        if (len2 < len && len2 >= 0) {
          _state->setcompressed(true);
          _state->ncompressed(len2);
          _tgt->push(len2, b);
        } else {
          _tgt->push(len, bsave);
        }
      } else {
        process(_config, _state, len, b);
      }
    }
    virtual int process(Ct const * const c, St *st, int len, sizebounded<Vt,sz> &b) const override {
      if (len <= 0) { return 0; }
      st->nread(len);
      // return the same size if not compressed
      if (! Options::current().isCompressed()) { return len; }

      int compressedlen = lzs.process(nullptr,nullptr,len,b);
      // std::cout << "  compressed " << len << " -> " << compressedlen << std::endl;
      if (compressedlen == 0) { return -1; }
      return compressedlen;
    }
  private:
    deflatestream<Ct,St,Vt,sz> lzs;
    stream<Ct,St,Vt,sz> *_tgt;
    Ct const * const _config;
    St * _state;
};

template <typename Ct, typename St, typename Vt, int sz>
class assemblystream : public stream<Ct,St,Vt,sz>
{
  public:
    assemblystream(Ct const * const c, St *st, stream<Ct,St,Vt,sz> *s, stream<Ct,St,Vt,sz> *t)
        : stream<Ct,St,Vt,sz>(c,st,s,t) {};
    virtual int process(Ct const * const c, St *st, int len, sizebounded<Vt,sz> &b) const override {
        int _apos = st->assembly()->pos();
        int nwritten = st->assembly()->addData(len, b);
        st->nwritten(nwritten);
        // std::cout << "     written: " << nwritten << std::endl;
        int nwritten2 = 0;
        if (nwritten != len) {
            // st->renew_assembly();
            // nwritten2 = st->assembly()->addData(len - nwritten, b, nwritten);
            // st->nwritten(nwritten2);
            // auto dbblock = DbFpBlock(
            //     st->next_bidx(),
            //     st->assembly()->pos() - nwritten2,
            //     st->fpos() - st->lastread(),
            //     // blen, clen
            //     st->lastread(), st->lastcompressed(),
            //     st->iscompressed(),
            //     st->lastchecksum(),
            //     st->assembly()->aid()
            //     );
            // st->dbentry()->_blocks.push_back(dbblock);
            // std::cout << "     written2: " << nwritten2 << std::endl;
            std::cout << " written: " << nwritten << " < " << len << std::endl;
        }
        auto dbblock = DbFpBlock(
            st->next_bidx(),
            _apos,
            st->fpos() - st->lastread(),
            // blen, clen
            st->lastread(), st->lastcompressed(),
            st->iscompressed(),
            st->lastchecksum(),
            st->assembly()->aid()
            );
        st->dbentry()->_blocks->push_back(std::move(dbblock));
        return (nwritten + nwritten2);
    };
};

bool BackupCtrl::backup(boost::filesystem::path const & fp)
{
    if (! _pimpl->_ass) { return false; }
    if (! FileCtrl::fileExists(fp)) { return false; }

    constexpr int bsz = Assembly::datasz;
    sizebounded<unsigned char, bsz> buffer;
    FILE *fptr = fopen(fp.native().c_str(), "r");
    if (! fptr) { return false; }

    // make DbFp entry
    auto t_dbentry = DbFpDat::fromFile(fp);

    // setup pipeline
    configuration config(_pimpl->_nChunks);
    state st(&config, &t_dbentry);
    st.dbkeys(&_pimpl->_dbkey);
    st.assembly(_pimpl->_ass);
    constexpr int dwidth = 1; // bytes read at once
    constexpr int readsz = bsz / dwidth;
    assemblystream<configuration,state,unsigned char,bsz> s3(&config, &st, nullptr, nullptr);
    compressstream<configuration,state,unsigned char,bsz> s2(&config, &st, nullptr, &s3);

    while (! feof(fptr)) {
        size_t nread = fread((void*)buffer.ptr(), dwidth, readsz, fptr);
        // std::cout << "   read: " << (nread*dwidth) << std::endl;
        s2.push(nread*dwidth, buffer);
    }
    fclose(fptr);
    s2.push(0, buffer);
    // std::cout << "finished." << std::endl;

    _pimpl->trx_in = st.nread();
    _pimpl->trx_out = st.nwritten();
    _pimpl->_ass = st.assembly();
    // std::cout << "** read " << _pimpl->trx_in << "   wrote " << _pimpl->trx_out << std::endl;

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

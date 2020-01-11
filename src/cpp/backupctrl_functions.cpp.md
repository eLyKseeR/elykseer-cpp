declared in [BackupCtrl](backupctrl.hpp.md)

```cpp
void BackupCtrl::setReference()
{
 // TODO
}

void BackupCtrl::finalize()
{
    if (! _pimpl->_ass) { return; }
    _pimpl->_ass->extractChunks();
}

bool BackupCtrl::pimpl::renew_assembly()
{
    if (_ass) {
        // extract
        if (! _ass->extractChunks()) {
            return false;
        }
    }
    _ass.reset(new Assembly(_o.nChunks()));
    return true;
}

class configuration {
  public:
    configuration() {};
    ~configuration() {};
    void nread(int n) {_nread=n;}
    void state(BackupCtrl::pimpl* p) {_state=p;}
    void dbentry(DbFpDat *d) {_dbentry=d;}
  private:
    pimpl *_state;
    DbFpDat *_dbentry;
    int _nread;
};

template <typename Ct, typename Vt, int sz>
class assemblystream : public stream<Ct,Vt,sz>
{
  public:
    assemblystream(Ct *c, stream<Ct,Vt,sz> *s, stream<Ct,Vt,sz> *t)
        : stream<Ct,Vt,sz>(c,s,t) {}
    virtual int process(Ct const * const c, int len, sizebounded<Vt,sz> &b) const {
        return b;
    };
};

bool BackupCtrl::backup(boost::filesystem::path const & fp)
{
    if (! _pimpl->_ass) { return false; }
    if (! FileCtrl::fileExists(fp)) { return false; }

    sizebounded<unsigned char, Assembly::datasz> buffer;
    FILE *fptr = fopen(fp.native().c_str(), "r");
    if (! fptr) { return false; }

    // make DbFP entry
    auto dbentry = DbFpDat::fromFile(fp);

    // setup pipeline
    configuration config;
    config.state(_pimpl.get());
    config.dbentry(&dbentry);
    constexpr int bsz = Assembly::datasz;
    assemblystream<configuration,char,bsz> s3(&config, nullptr, nullptr);
    compressor<configuration,char,bsz> s2(&config, nullptr, &s3);
    fileinstream<configuration,char,bsz> s1(&config, nullptr, &s2);

    int _bidx = 1;
    uint64_t _fpos = 0;
    while (! feof(fptr)) {
        size_t nread = fread((void*)buffer.ptr(), 64, Assembly::datasz / 64, fptr);
        config.nread(nread);
        s1.push(buffer);
        _pimpl->trx_in += nread;
        int nwritten = _pimpl->_ass->addData(nread * 64, buffer);
        _pimpl->trx_out += nwritten;
        auto dbblock = DbFpBlock(
            _bidx++,
            _pimpl->_ass->pos(),
            _fpos,
            // blen, clen
            nwritten, nwritten,
            false, // compressed?
            Md5::hash((const char *)buffer.ptr(), nwritten), // checksum
            _pimpl->_ass->aid()
            );

        // not everything written; 
        if (nwritten != nread * 64) {
            if (! _pimpl->renew_assembly()) {
                return false;
            }
            int nwritten2 = _pimpl->_ass->addData(nread * 64 - nwritten, buffer, nwritten);
        }
    }
    fclose(fptr);

    _pimpl->_dbfp.set(fp.native(), dbentry);

    return true;
}

```

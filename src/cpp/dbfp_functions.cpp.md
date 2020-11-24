declared in [DbFp](dbfp.hpp.md)

Create a *DbFpBlock* with given data.
```c++
DbFpBlock::DbFpBlock(int i,int a,uint64_t f,int bl,int cl,bool c, const std::string& chk, const std::string & aid)
  : _idx(i), _apos(a)
  , _fpos(f), _blen(bl)
  , _clen(cl), _compressed(c)
  , _checksum(chk), _aid(aid)
{ }
```

Helper method to create a *DbFpDat*.
```c++
DbFpDat DbFpDat::make(std::string const & fp)
{
    DbFpDat db;
    db._id = Md5::hash(fp).toHex();
    return db;
}
```

Helper method to create a *DbFpDat* from an existing file
```c++
DbFpDat DbFpDat::fromFile(boost::filesystem::path const & fp)
{
    DbFpDat db = make(fp.native());
    if (! FileCtrl::fileExists(fp)) {
        return db;
    }
    db._checksum = Sha256::hash(fp).toHex();

#if defined(__linux__) || defined(__APPLE__)
    struct stat _fst;
    if (stat(fp.native().c_str(), &_fst) == 0) {
        db._len = _fst.st_size;
        char _buf[65];
        snprintf(_buf, 65, "%d", _fst.st_uid);
        db._osusr = _buf;
        snprintf(_buf, 65, "%d", _fst.st_gid);
        db._osgrp = _buf;
#ifdef __APPLE__
        db._osattr = OS::time2string(_fst.st_mtimespec.tv_sec);
#endif
#ifdef __linux__
        db._osattr = OS::time2string(_fst.st_mtim.tv_sec);
#endif
    }
#else
    #error Such a platform is not yet a good host for this software
#endif
    return db;
}
```

```c++
void DbFp::inStream(std::istream & ins)
{
    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(ins, pt,
      boost::property_tree::xml_parser::no_comments |
      boost::property_tree::xml_parser::trim_whitespace);

#ifdef DEBUG
    int counter = 0;
    auto t0 = clk::now();
#endif
    for (auto root = pt.begin(); root != pt.end(); root++) {
        if (root->first == "DbFp") {
          for (auto db = root->second.begin(); db != root->second.end(); db++) {
            if (db->first == "Fp") {
              lxr::DbFpDat dat;
              std::string _fp;
              for (auto fp = db->second.begin(); fp != db->second.end(); fp++) {
#ifdef DEBUG
                counter++;
#endif
                if (fp->first == "<xmlattr>") {
                  _fp = fp->second.get<std::string>("fp");
                  dat._id = fp->second.get<std::string>("id");
                } else if (fp->first == "Fattrs") {
                  dat._osusr = fp->second.get<std::string>("osusr");
                  dat._osgrp = fp->second.get<std::string>("osgrp");
                  dat._len = fp->second.get<uint64_t>("length");
                  dat._osattr = fp->second.get<std::string>("last");
                  dat._checksum = fp->second.get<std::string>("chksum");
                } else if (fp->first == "Fblock") {
                  lxr::DbFpBlock block;
                  block._aid = fp->second.data();
                  for (auto bl = fp->second.begin(); bl != fp->second.end(); bl++) {
                    if (bl->first == "<xmlattr>") {
                      block._idx = bl->second.get<int>("idx");
                      block._apos = bl->second.get<int>("apos");
                      block._fpos = bl->second.get<uint64_t>("fpos");
                      block._blen = bl->second.get<int>("blen");
                      block._clen = bl->second.get<int>("clen");
                      block._compressed = 1 == bl->second.get<int>("compressed");
                      block._checksum = bl->second.get<std::string>("chksum");
                    }
                  }
                  dat._blocks->push_back(std::move(block));
                }
              }
              if (! _fp.empty()) {
                set(_fp, std::move(dat)); // add to db
              }
            }
          }
        }
    }
#ifdef DEBUG
    auto t1 = clk::now();
    auto tdiff = boost::chrono::round<boost::chrono::microseconds>(t1 - t0).count();
    std::clog << "inStream took " << tdiff << " microseconds" << std::endl;
    std::clog << "iterations: " << counter << std::endl;
#endif
}
```

```fsharp
    member this.outStream (s : TextWriter) =
        //let refl1 = Reflection.Assembly.GetCallingAssembly()
        let refl2 = Reflection.Assembly.GetExecutingAssembly()
        //let xname = refl1.GetName()
        let aname = refl2.GetName()
        s.WriteLine("<?xml version=\"1.0\"?>")
        s.WriteLine("<DbFp xmlns=\"http://spec.sbclab.com/lxr/v1.0\">")
        s.WriteLine("<library><name>{0}</name><version>{1}</version></library>", aname.Name, aname.Version.ToString())
        //s.WriteLine("<program><name>{0}</name><version>{1}</version></program>", xname.Name, xname.Version.ToString())
        s.WriteLine("<host>{0}</host>", System.Environment.MachineName)
        s.WriteLine("<user>{0}</user>", System.Environment.UserName)
        s.WriteLine("<date>{0}</date>", System.DateTime.Now.ToString("s"))
        this.idb.appValues (fun k v ->
             let l1 = sprintf "  <Fp fp=\"%s\" id=\"%s\">" k (Key128.toHex v.id)
             s.WriteLine(l1)
             let l2 = sprintf "     <Fattrs><osusr>%s</osusr><osgrp>%s</osgrp><length>%d</length><last>%s</last><chksum>%s</chksum></Fattrs>" v.osusr v.osgrp v.len v.osattr (Key256.toHex v._checksum)
             s.WriteLine(l2)
             for b in v.blocks do
                 let l2 = sprintf "    <Fblock idx=\"%d\" apos=\"%d\" fpos=\"%d\" blen=\"%d\" clen=\"%d\" compressed=\"%s\" chksum=\"%s\">%s</Fblock>" b.idx b.apos b.fpos b.blen b.clen (if b.compressed then "1" else "0") (Key128.toHex b._checksum ) (Key256.toHex b.aid)
                 s.WriteLine(l2)
             s.WriteLine("  </Fp>")
         )
        s.WriteLine("</DbFp>")
        s.Flush()
```

```c++
void DbFp::outStream(std::ostream & os) const
{
    os << "<?xml version=\\"1.0\\"?>" << std::endl;
    os << "<DbFp xmlns=\\"http://spec.sbclab.com/lxr/v1.0\\">" << std::endl;
    os << "<library><name>" << Liz::name() << "</name><version>" << Liz::version() << "</version></library>" << std::endl;
    os << "<host>" << OS::hostname() << "</host>" << std::endl;
    os << "<user>" << OS::username() << "</user>" << std::endl;
    os << "<date>" << OS::timestamp() << "</date>" << std::endl;
    appValues([&os](std::string const & k, struct DbFpDat const & v) {
        os << "  <Fp fp=\\"" << k << "\\" id=\\"" << v._id/* .toHex() */ << "\\">" << std::endl;
        os << "    <Fattrs><osusr>" << v._osusr << "</osusr><osgrp>" << v._osgrp << "</osgrp><length>" << v._len << "</length><last>" << v._osattr << "</last><chksum>" << v._checksum/* .toHex() */ << "</chksum></Fattrs>" << std::endl;
        for (const auto & b : *v._blocks) {
            os << "    <Fblock idx=\\"" << b._idx << "\\" apos=\\"" << b._apos << "\\" fpos=\\"" << b._fpos << "\\" blen=\\"" << b._blen << "\\" clen=\\"" << b._clen << "\\" compressed=\\"" << b._compressed << "\\" chksum=\\"" << b._checksum << "\\">" << b._aid << "</Fblock>" << std::endl;
        }
        os << "  </Fp>" << std::endl;
    });
    os << "</DbFp>" << std::endl;
}
```

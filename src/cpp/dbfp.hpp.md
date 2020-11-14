```cpp

/*
<fpaste ../../src/copyright.md>
*/

#pragma once

#include "lxr/dbctrl.hpp"
#include "lxr/key256.hpp"
#include "lxr/key128.hpp"
#include "lxr/md5.hpp"
#include "boost/filesystem.hpp"

#include <iostream>
#include <string>
#include <vector>
````

namespace [lxr](namespace.list) {

/*

```fsharp
type DbFpBlock = {
             idx : int;
             apos : int;
             fpos : int64;
             blen : int;
             clen : int;
             compressed : bool;
             checksum : Key128.t;
             aid : aid
            }

type DbFpDat = {
             id : Key128.t;
             len : int64;
             osusr : string;
             osgrp : string;
             osattr : string;
             checksum : Key256.t;
             blocks : DbFpBlock list
           }

```
*/

## struct DbFpBlock
```c++
{
    DbFpBlock() : _idx(-1) {};
    DbFpBlock(DbFpBlock && b) { _idx = b._idx; _apos = b._apos;
       _fpos = b._fpos; _blen = b._blen;
       _clen = b._clen; _compressed = b._compressed;
       _checksum = b._checksum; _aid = b._aid; }
    DbFpBlock& operator=(const DbFpBlock && b) { _idx = b._idx; _apos = b._apos;
       _fpos = b._fpos; _blen = b._blen;
       _clen = b._clen; _compressed = b._compressed;
       _checksum = b._checksum; _aid = b._aid;
       return *this; }
    bool operator==(const DbFpBlock & b) const {
      return(_idx == b._idx && _apos == b._apos
      && _fpos == b._fpos && _blen == b._blen
      && _clen == b._clen && _compressed == b._compressed
      && _checksum == b._checksum && _aid == b._aid); }
    DbFpBlock(DbFpBlock const & b) = delete;
    DbFpBlock& operator=(DbFpBlock const & b) = delete;
    DbFpBlock(int,int,uint64_t,int,int,bool,const std::string&, const std::string&);
    int _idx, _apos;
    uint64_t _fpos;
    int _blen, _clen;
    bool _compressed;
    std::string _checksum;
    std::string _aid;
};
```

## struct DbFpDat
```c++
{
    static DbFpDat make(std::string const &);
    static DbFpDat fromFile(boost::filesystem::path const &);
    DbFpDat() : _blocks(new std::vector<DbFpBlock>()) {}
    ~DbFpDat() { _blocks.reset(); }
    DbFpDat(DbFpDat const & d) { _id = d._id; _len = d._len;
        _osusr = d._osusr; _osgrp = d._osgrp; _osattr = d._osattr;
        _checksum = d._checksum; _blocks = d._blocks; }
    DbFpDat(DbFpDat && d) { _id = d._id; _len = d._len;
        _osusr = d._osusr; _osgrp = d._osgrp; _osattr = d._osattr;
        _checksum = d._checksum; _blocks = d._blocks; }
    DbFpDat& operator=(DbFpDat const && d) { _id = d._id; _len = d._len;
        _osusr = d._osusr; _osgrp = d._osgrp; _osattr = d._osattr;
        _checksum = d._checksum; _blocks = d._blocks;
        return *this; }
    bool operator==(DbFpDat const & d) const {
        return(
        _id == d._id && _len == d._len
        && _osusr == d._osusr && _osgrp == d._osgrp
        && _osattr == d._osattr && _checksum == d._checksum
        && _blocks->size() == d._blocks->size()); }
    DbFpDat& operator=(DbFpDat const & d) = delete;
    std::string _id;
    uint64_t _len;
    std::string _osusr, _osgrp, _osattr;
    std::string _checksum;
    std::shared_ptr<std::vector<DbFpBlock>> _blocks;
};
std::ostream & operator<<(std::ostream &os, lxr::DbFpDat const & dat);
```

# class DbFp : public DbCtrl&lt;struct DbFpDat&gt;

{

>public:

>DbFp() : DbCtrl&lt;struct DbFpDat&gt;() {}

>virtual void [inStream](dbfp_functions.cpp.md)(std::istream &) override;

>virtual void [outStream](dbfp_functions.cpp.md)(std::ostream &) const override;

>protected:

>private:

>DbFp(DbFp const &) = delete;

>DbFp & operator=(DbFp const &) = delete;

};

```cpp
} // namespace
```

declared in [Chunk](chunk.hpp.md)

```cpp

unsigned char Chunk::get(int pos) const
{
  return (*_pimpl->_buffer)[pos];
}

void Chunk::set(int pos, unsigned char val)
{
  (*_pimpl->_buffer)[pos] = val;
}

void Chunk::clear()
{
  _pimpl->_buffer->transform([](int i, unsigned char c)->unsigned char {
    return '\0';
  });
}

bool Chunk::fromFile(boost::filesystem::path const & fpath)
{
  if (! FileCtrl::fileExists(fpath)) { return false; }
  if (auto fsz = FileCtrl::fileSize(fpath); fsz && *fsz < Chunk::size) { return false; }

  if (FILE * fstr = fopen(fpath.c_str(), "rb"); ! fstr) {
    return false;
  } else {
    size_t nread = fread((void*)_pimpl->_buffer->ptr(), 64, Chunk::size / 64, fstr);
    fclose(fstr);
    return (nread == Chunk::size / 64);
  }
}

bool Chunk::toFile(boost::filesystem::path const & fpath) const
{
  if (FileCtrl::fileExists(fpath)) { return false; }

  if (FILE * fstr = fopen(fpath.c_str(), "wb"); ! fstr) {
    return false;
  } else {
    size_t nwritten = fwrite(_pimpl->_buffer->ptr(), 64, Chunk::size / 64, fstr);
    fclose(fstr);
    return (nwritten == Chunk::size / 64);
  }
}

```

compute Md5 checksum over chunk's internal buffer
```cpp
Key128 Chunk::md5() const
{
  return Md5::hash((const char*)_pimpl->_buffer->ptr(), Chunk::size);
}
```

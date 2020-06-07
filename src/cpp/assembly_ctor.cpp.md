declared in [Assembly](assembly.hpp.md)

```cpp

Assembly::Assembly(int n)
  :_pimpl(new pimpl(n))
{
  // initialise first bytes with random data
  Key128 initr;
  sizebounded<unsigned char, datasz> buf;
  const int len = initr.length()/8;
  buf.transform([&initr,len](int i, unsigned char c)->unsigned char {
      if (i < len) {
          return initr.bytes()[i];
      } else {
          return '\\0';
      }
  });
  addData(len, buf);
}

Assembly::Assembly(Key256 const & aid, int n)
  :_pimpl(new pimpl(aid, n))
{ }

Assembly::~Assembly() = default;

```

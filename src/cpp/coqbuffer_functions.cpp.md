```coq
```

```cpp

uint32_t CoqBuffer::len() const
{
    if (_pimpl) {
        return _pimpl->len();
    } else {
        return 0;
    }
}

std::optional<const Key256> CoqBuffer::calc_checksum() const
{
    if (_pimpl) {
        return _pimpl->calc_checksum();
    } else {
        return {};
    }
}

int CoqBuffer::copy_sz_pos(int pos0, int sz, CoqBuffer &target, int pos1) const
{
    if (_pimpl) {
        return _pimpl->copy_sz_pos(pos0, sz, target, pos1);
    } else {
        return -1;
    }
}

int CoqBuffer::fileout_sz_pos(int pos, int sz, FILE *fstr) const
{
    if (_pimpl && fstr) {
        return _pimpl->fileout_sz_pos(pos, sz, fstr);
    } else {
        return -1;
    }
}

int CoqBuffer::filein_sz_pos(int pos, int sz, FILE *fstr)
{
    if (_pimpl && fstr) {
        return _pimpl->filein_sz_pos(pos, sz, fstr);
    } else {
        return -1;
    }
}


```
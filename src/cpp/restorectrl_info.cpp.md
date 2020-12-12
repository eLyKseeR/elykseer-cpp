declared in [RestoreCtrl](restorectrl.hpp.md)

```cpp
uint64_t RestoreCtrl::bytes_in() const
{
    return _pimpl->trx_in;
}

uint64_t RestoreCtrl::bytes_out() const
{
    return _pimpl->trx_out;
}

std::chrono::microseconds RestoreCtrl::time_decrypt() const
{
    return _pimpl->time_decr;
}

std::chrono::microseconds RestoreCtrl::time_decompress() const
{
    return _pimpl->time_decomp;
}

std::chrono::microseconds RestoreCtrl::time_read() const
{
    return _pimpl->time_read;
}

std::chrono::microseconds RestoreCtrl::time_write() const
{
    return _pimpl->time_write;
}

std::chrono::microseconds RestoreCtrl::time() const
{
    return _pimpl->time_restore;
}
```

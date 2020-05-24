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

std::time_t RestoreCtrl::time_decrypt() const
{
    return 0; // TODO
}

std::time_t RestoreCtrl::time_extract() const
{
    return 0; // TODO
}

std::time_t RestoreCtrl::time_read() const
{
    return 0; // TODO
}
```

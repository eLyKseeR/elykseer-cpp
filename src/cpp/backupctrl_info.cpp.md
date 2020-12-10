declared in [BackupCtrl](backupctrl.hpp.md)

```cpp
uint32_t BackupCtrl::free() const
{
    if (_pimpl->_ass) {
        return _pimpl->_ass->free();
    }
    return 0;
}

uint64_t BackupCtrl::bytes_in() const
{
    return _pimpl->trx_in;
}

uint64_t BackupCtrl::bytes_out() const
{
    return _pimpl->trx_out;
}

std::chrono::microseconds BackupCtrl::time_encrypt() const
{
    return _pimpl->time_encr;
}

std::chrono::microseconds BackupCtrl::time_extract() const
{
    return _pimpl->time_decr;
}

std::chrono::microseconds BackupCtrl::time_read() const
{
    return _pimpl->time_read;
}

std::chrono::microseconds BackupCtrl::time_write() const
{
    return _pimpl->time_write;
}

DbFp const & BackupCtrl::getDbFp() const
{
    return _pimpl->_dbfp;
}

DbKey const & BackupCtrl::getDbKey() const
{
    return _pimpl->_dbkey;
}
```

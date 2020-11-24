declared in [BackupCtrl](backupctrl.hpp.md)

```cpp
BackupCtrl::BackupCtrl()
  : _pimpl(new pimpl())
{
  _pimpl->_nChunks = Options::current().nChunks();
}

BackupCtrl::~BackupCtrl()
{
  if (_pimpl) {
    finalize();
    _pimpl.reset();
  }
}
```

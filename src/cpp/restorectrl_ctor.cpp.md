declared in [RestoreCtrl](restorectrl.hpp.md)

```cpp
RestoreCtrl::RestoreCtrl()
  : _pimpl(new pimpl())
{
}

RestoreCtrl::~RestoreCtrl() {
  if (_pimpl) {
    _pimpl.reset();
  }
}
```

```cpp

/*
<fpaste ../../../src/copyright.md>
*/

#pragma once

#include "streamio.hpp"
#include <unordered_map>
#include <functional>
#include <optional>

````

namespace [lxr](namespace.list) {

/*

```fsharp


type IDb<'k, 'e> =

    inherit IStreamIO

    abstract member contains : 'k -> bool
    abstract member get : 'k -> 'e option
    abstract member set : 'k -> 'e -> unit
    abstract member count : int
    abstract member union : IDb<'k,'e> -> unit
    abstract member appKeys : ('k -> unit) -> unit
    abstract member appValues : ('k -> 'e -> unit) -> unit

```

*/

> template &lt;typename T, typename K = std::string&gt;

# class DbCtrl : public [StreamIO](streamio.hpp.md)

{

>public:

>virtual int count() const noexcept {
      return _map.size(); }

>virtual bool contains(K const & k) const noexcept {
```cpp
      try { return (_map.at(k),true); } catch (...) { return false; }; }
```

>virtual std::optional&lt;T&gt; get(K const & k) noexcept {
```cpp
      try { T & r = _map.at(k); return r; } catch (...) { return {}; }; }
```

>virtual void set(K const & k, T const & v) noexcept {
      _map[k]=std::move(v);
    }

>virtual void unionWith(DbCtrl&lt;T,K&gt; const & other) noexcept {
```cpp
      _map.insert(other._map.begin(), other._map.end());
    }
```

>virtual void appKeys(std::function&lt;void(K const &)&gt; f) const noexcept {
```cpp
      for (const auto & p : _map) {
        f(p.first); }
    }
```

>virtual void appValues(std::function&lt;void(K const &, T const &)&gt; f) const noexcept {
```cpp
      for (const auto & p : _map) {
        f(p.first, p.second); }
    }
```

>protected:

>DbCtrl() {}

>std::unordered_map&lt;K,T&gt; _map;

>private:

>DbCtrl(DbCtrl const &) = delete;

>DbCtrl & operator=(DbCtrl const &) = delete;

};

```cpp
} // namespace 1
```


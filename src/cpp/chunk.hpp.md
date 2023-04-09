```cpp

/*
<fpaste ../../../src/copyright.md>
*/

#pragma once

#include "sizebounded/sizebounded.hpp"

#include "lxr/md5.hpp"
#include "lxr/filectrl.hpp"

#include <memory>

````

namespace [lxr](namespace.list) {

/*

```fsharp

module Chunk = 

    type t

    exception NoAccess
    exception AlreadyExists
    exception BadNumber

    val width : int

    val height : int
    (** constant: height of a chunk in bytes *)

    val size : int
    (** constant: size of a chunk in bytes *)

    //val num : t -> int
    //val buf : t -> byte array

    val create : unit -> t
    (** create new chunk with number set *)

    //[<CompiledName("FromFile")>]
     val fromFile : string -> t
    (** restore chunk from file given file path *)

    val toFile : t -> string -> bool
    (** store chunk to file, given file path *)

    val empty : t -> unit
    (** clear content of chunk *)

    val get : t -> int -> byte
    (** get byte at index *)

    val set : t -> int -> byte -> unit
    (** set byte at index *)

#if DEBUG
    val show : t -> unit
    (** output in hex format to console *)
#endif
```

*/

# class Chunk

{

>public:

>static constexpr int width { 256 };

>static constexpr int height { 1024 };

>static constexpr int size { width * height };

>[Chunk](chunk_ctor.cpp.md)();

>explicit [Chunk](chunk_ctor.cpp.md)(std::shared_ptr&lt;sizebounded&lt;unsigned char, Chunk::size&gt;&gt;);

>[~Chunk](chunk_ctor.cpp.md)();

>void [clear](chunk_functions.cpp.md)();

>inline

>unsigned char [get](chunk_functions.cpp.md)(int pos) const {return _buffer->get(pos);}

>inline

>void [set](chunk_functions.cpp.md)(int pos, unsigned char val) {(*_buffer)[pos] = val;}

>bool [toFile](chunk_functions.cpp.md)(filepath const &) const;

>bool [fromFile](chunk_functions.cpp.md)(filepath const &);

>Key128 [md5](chunk_functions.cpp.md)() const;

>private:

>std::shared_ptr<sizebounded<unsigned char, Chunk::size>> _buffer;

>Chunk(Chunk const &) = delete;

>Chunk & operator=(Chunk const &) = delete;

};

```cpp
} // namespace
```

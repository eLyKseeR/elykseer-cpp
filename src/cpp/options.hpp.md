```cpp

/*
<fpaste ../../src/copyright.md>
*/

#pragma once

#include "lxr/streamio.hpp"
#include "boost/filesystem.hpp"
#include "boost/property_tree/ptree.hpp"

#include <memory>
#include <iosfwd>

````

namespace [lxr](namespace.list) {

/*

```fsharp

type Options =
    class

    interface IStreamIO

        //override inStream : TextReader -> unit
        //override outStream : TextWriter -> unit

    (** ctor *)
    new : unit -> Options

    (** getters *)
    member nchunks : int
    member redundancy : int
    member fpath_chunks : string
    member fpath_db : string
    member isCompressed : bool
    member isDeduplicated : int

    (** setters *)
    member setNchunks : v:int -> unit
    member setRedundancy : v:int -> unit
    member setFpathChunks : v:string -> unit
    member setFpathDb : v:string -> unit
    member setCompression : v:bool -> unit
    member setDeduplication : v:int -> unit

    (** cast to the interface *)
    member io : IStreamIO

    end
```

*/

# class Options : public [StreamIO](streamio.hpp.md)

{

>public:

>static Options const & [current](options_functions.cpp.md)() noexcept;

>static Options & [set](options_functions.cpp.md)() noexcept;

>[Options](options_ctor.cpp.md)();

>[Options](options_ctor.cpp.md)(Options const &);

>Options & operator=(Options const &) noexcept;

>[~Options](options_ctor.cpp.md)();

>void [fromXML](options_functions.cpp.md)(boost::property_tree::ptree::iterator &);

>virtual void inStream(std::istream&) override;

>virtual void outStream(std::ostream&) const override;

>int [nChunks](options_functions.cpp.md)() const noexcept;

>void [nChunks](options_functions.cpp.md)(int v) noexcept;

>int [nRedundancy](options_functions.cpp.md)() const noexcept;

>void [nRedundancy](options_functions.cpp.md)(int v) noexcept;

>bool [isCompressed](options_functions.cpp.md)() const noexcept;

>void [isCompressed](options_functions.cpp.md)(bool v) noexcept;

>int [isDeduplicated](options_functions.cpp.md)() const noexcept;

>void [isDeduplicated](options_functions.cpp.md)(int v) noexcept;

>boost::filesystem::path const & [fpathChunks](options_functions.cpp.md)() const noexcept;

>boost::filesystem::path & [fpathChunks](options_functions.cpp.md)() noexcept;

>boost::filesystem::path const & [fpathMeta](options_functions.cpp.md)() const noexcept;

>boost::filesystem::path & [fpathMeta](options_functions.cpp.md)() noexcept;

>protected:

>private:

>friend std::ostream & operator<<(std::ostream &os, Options const & opt);

>struct pimpl;

>std::unique_ptr&lt;pimpl&gt; _pimpl;

};

```cpp
std::ostream & operator<<(std::ostream &os, Options const & opt);

} //namespace

```

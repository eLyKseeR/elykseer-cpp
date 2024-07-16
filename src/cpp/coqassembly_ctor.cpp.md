declared in [CoqAssembly](coqassembly.hpp.md)

```cpp
CoqAssembly::KeyInformation::KeyInformation(const CoqConfiguration & c)
  : _localid(c.my_id), _localnchunks(c.nchunks())
{}
```

```cpp
CoqAssembly::AssemblyInformation::AssemblyInformation()
{}

CoqAssembly::AssemblyInformation::AssemblyInformation(const CoqConfiguration & c)
  : _nchunks(c.nchunks())
{}

CoqAssembly::AssemblyInformation::AssemblyInformation(const CoqConfiguration & c, const std::string & aid)
  : _nchunks(c.nchunks()), _aid(aid)
{}
```

```cpp
CoqAssembly::CoqAssembly(const CoqConfiguration &c)
  : _config(c)
{
  // std::clog << "CoqAssembly::CoqAssembly(const CoqConfiguration &c)" << std::endl;
}

CoqAssembly::CoqAssembly(CoqAssembly *ca)
  : _config(ca->_config), _assemblyinformation(ca->_assemblyinformation)
{
  // std::clog << "CoqAssembly::CoqAssembly(CoqAssembly *ca)" << std::endl;
}

CoqAssembly::CoqAssembly()
{
  // std::clog << "CoqAssembly::CoqAssembly()" << std::endl;
}

CoqAssembly::~CoqAssembly() = default;
```

## create a distinct assembly id (256 bits)

```ml
   function
   x -> Elykseer_crypto.Random.with_rng (fun rng -> Elykseer_crypto.Random.random32 rng) |> string_of_int |>
     String.cat x |>
     String.cat (Unix.gethostname ()) |> String.cat (Unix.gettimeofday () |> string_of_float) |>
     Elykseer_crypto.Sha3_256.string
```
```cpp
void CoqAssembly::mk_assembly_id()
{
  auto now = std::chrono::system_clock::now();
  auto tstamp = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  lxr::Random r;
  ss << r.random() << _config.my_id << OS::hostname() << tstamp;
  const std::string v{ss.str()};
  const auto h = Sha3_256::hash(v).toHex();
  // std::clog << "  mk_assembly_id: " << v << " => " << h << std::endl;
  _assemblyinformation._aid = h;
}
```

## CoqAssemblyPlainWritable

```coq
    Definition create (c : configuration) : H * B :=
        let chunks := config_nchunks c in
        let b := BufferPlain.buffer_create (chunksize_N * Nchunks.to_N chunks) in
        let rb := Buffer.ranbuf128 tt in
        let nb := BufferPlain.copy_sz_pos rb 0 16 b 0 in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) nb, b).
```

```cpp

CoqAssemblyPlainWritable::CoqAssemblyPlainWritable(const CoqConfiguration &c)
  : CoqAssembly(c)
{
  // std::clog << "CoqAssemblyPlainWritable::CoqAssemblyPlainWritable(const CoqConfiguration &c)" << std::endl;
  uint32_t nchunks = c.nchunks();
  _buffer.reset(new CoqBufferPlain(CoqAssembly::chunksize * nchunks));
  Key128 _rand128{false};
  const char *r = (const char*)_rand128.bytes();
  for (int i = 0; i < 16; i++) {
    _buffer->at(idx2apos(i,nchunks), r[i]);
  }
  _assemblyinformation._apos = 16;
  _assemblyinformation._nchunks = nchunks;
  mk_assembly_id();
}

CoqAssemblyPlainWritable::~CoqAssemblyPlainWritable() = default;

```

```coq
    Definition create (c : configuration) :=
        let chunks := config_nchunks c in
        let sz := chunksize_N * Nchunks.to_N chunks in
        let b := BufferPlain.buffer_create sz in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) sz, b).
```

## CoqAssemblyPlainFull

```cpp

CoqAssemblyPlainFull::CoqAssemblyPlainFull(const CoqConfiguration &c)
  : CoqAssembly(c)
{
  // std::clog << "CoqAssemblyPlainFull::CoqAssemblyPlainFull(const CoqConfiguration &c)" << std::endl;
  _buffer.reset(new CoqBufferPlain(CoqAssembly::chunksize * c.nchunks()));
}

CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyEncrypted *ca, std::shared_ptr<CoqBufferPlain> &b)
  : CoqAssembly(ca)
{
  // std::clog << "CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyEncrypted *ca, std::shared_ptr<CoqBufferPlain> &b)" << std::endl;
  _buffer = b;
}

CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyPlainWritable *ca, std::shared_ptr<CoqBufferPlain> &b)
  : CoqAssembly(ca)
{
  // std::clog << "CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyPlainWritable *ca, std::shared_ptr<CoqBufferPlain> &b)" << std::endl;
  _buffer = b;
}

CoqAssemblyPlainFull::~CoqAssemblyPlainFull() = default;

```


```coq
    Definition create (c : configuration) : H * B :=
        let chunks := config_nchunks c in
        let b := BufferEncrypted.buffer_create (chunksize_N * Nchunks.to_N chunks) in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) 0, b).
```

## CoqAssemblyEncrypted

```cpp

CoqAssemblyEncrypted::CoqAssemblyEncrypted(const CoqConfiguration &c)
  : CoqAssembly(c)
{
  // std::clog << "CoqAssemblyEncrypted::CoqAssemblyEncrypted(const CoqConfiguration &c)" << std::endl;
  _buffer.reset(new CoqBufferEncrypted(CoqAssembly::chunksize * c.nchunks()));
}

CoqAssemblyEncrypted::CoqAssemblyEncrypted(CoqAssemblyPlainFull *ca, std::shared_ptr<CoqBufferEncrypted> &b)
  : CoqAssembly(ca)
{
  // std::clog << "CoqAssemblyEncrypted::CoqAssemblyEncrypted(CoqAssemblyPlainFull *ca, std::shared_ptr<CoqBufferEncrypted> &b)" << std::endl;
  _buffer = std::move(std::reinterpret_pointer_cast<CoqBufferEncrypted>(b));
}


CoqAssemblyEncrypted::~CoqAssemblyEncrypted() = default;

```
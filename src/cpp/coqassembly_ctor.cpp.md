declared in [CoqAssembly](coqassembly.hpp.md)

```cpp
CoqAssembly::KeyInformation::KeyInformation(const CoqConfiguration & c)
  : _localid(c.my_id), _localnchunks(c.nchunks())
{}
```

```cpp
CoqAssembly::CoqAssembly(const CoqConfiguration &c)
  : _config(c)
{}

CoqAssembly::CoqAssembly(CoqAssembly *ca)
  : _config(ca->_config), _assemblyinformation(ca->_assemblyinformation)
{}

CoqAssembly::CoqAssembly()
{}

CoqAssembly::~CoqAssembly() = default;
```

## create a distinct assembly id (256 bits)

```ml
   function
   x -> Elykseer_crypto.Random.with_rng (fun rng -> Elykseer_crypto.Random.random32 rng) |> string_of_int |>
     String.cat x |>
     String.cat (Unix.gethostname ()) |> String.cat (Unix.gettimeofday () |> string_of_float) |>
     Elykseer_crypto.Sha256.string
```
```cpp
void CoqAssembly::mk_assembly_id()
{
  auto now = std::chrono::system_clock::now();
  auto tstamp = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  lxr::Random r;
  ss << r.random() << _config.my_id << OS::hostname() << tstamp;
  const auto h = Sha256::hash(ss.str()).toHex();
  _assemblyinformation._aid = h;
}
```

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
  _buffer.reset(new CoqBufferPlain(CoqAssembly::chunksize * c.nchunks()));
  Key128 _rand128{false};
  const char *r = (const char*)_rand128.bytes();
  for (int i = 0; i < 16; i++) {
    _buffer->at(i, r[i]);
  }
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

```cpp

CoqAssemblyPlainFull::CoqAssemblyPlainFull(const CoqConfiguration &c)
  : CoqAssembly(c)
{
  _buffer.reset(new CoqBufferPlain(CoqAssembly::chunksize * c.nchunks()));
}

CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyEncrypted *ca)
  : CoqAssembly(ca)
{
  // TODO decrypt assembly
}

CoqAssemblyPlainFull::CoqAssemblyPlainFull(CoqAssemblyPlainWritable *ca)
  : CoqAssembly(ca)
{
  // TODO finish assembly
}

CoqAssemblyPlainFull::~CoqAssemblyPlainFull() = default;

```


```coq
    Definition create (c : configuration) : H * B :=
        let chunks := config_nchunks c in
        let b := BufferEncrypted.buffer_create (chunksize_N * Nchunks.to_N chunks) in
        (mkassembly chunks (Utilities.rnd256 (my_id c)) 0, b).
```

```cpp

CoqAssemblyEncrypted::CoqAssemblyEncrypted(const CoqConfiguration &c)
  : CoqAssembly(c)
{
  _buffer.reset(new CoqBufferEncrypted(CoqAssembly::chunksize * c.nchunks()));
}

CoqAssemblyEncrypted::CoqAssemblyEncrypted(CoqAssemblyPlainFull *ca)
  : CoqAssembly(ca)
{
  // TODO encrypt assembly
}


CoqAssemblyEncrypted::~CoqAssemblyEncrypted() = default;

```
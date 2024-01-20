declared in [CoqEnvironment](coqenvironment.hpp.md)


```coq
Definition initial_environment (c : configuration) : environment :=
    let (a,b) := AssemblyPlainWritable.create c in
    {| cur_assembly := a
    ;  cur_buffer := b
    ;  config := c
    ;  fblocks := nil
    ;  keys := nil
    |}.
```

```cpp

CoqEnvironment::CoqEnvironment(const CoqConfiguration &c)
  : _config(c)
{
}

CoqEnvironment::~CoqEnvironment() = default;

```

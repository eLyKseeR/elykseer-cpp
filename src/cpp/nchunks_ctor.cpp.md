declared in [Nchunks](nchunks.hpp.md)

/*

it is guaranteed that the value returned by this structure
is within the bounds [min_n .. max_n].

```coq

Definition max_n : positive := 256.
Definition min_n : positive := 16.

Module Private.
    Definition t := positive.
    Definition from_positive : positive -> t := fun n =>
        Pos.min max_n (Pos.max n min_n).
    Definition from_int : int -> t := fun i =>
        from_positive (Conversion.i2p i).
    Definition to_positive : t -> positive := fun x => x.
    Definition to_N : t -> N := fun x => Conversion.pos2N x.
End Private.

```

*/

```cpp

Nchunks::Nchunks(int n)
{
    _n = std::min(max_n, std::max(min_n, n));
}

```

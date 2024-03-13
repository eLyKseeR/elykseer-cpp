declared in [CoqAssemblyCache](coqassemblycache.hpp.md)

## Private implementation

```coq
Record assemblycache : Type :=
    mkassemblycache
        { acenvs : list EnvironmentReadable.E
        ; acsize : nat
            (* ^ read environments of a fixed maximal size *)
        ; acwriteenv : EnvironmentWritable.E
            (* ^ write environment, exactly one *)
        ; acconfig : configuration
            (* ^ configuration needed to create new assemblies *)
        ; acwriteq : writequeue
            (* ^ writer queue *)
        ; acreadq : readqueue
            (* ^ reader queue *)
        ; acfbstore : FBlockListStore.R
            (* ^ file block store *)
        ; ackstore : KeyListStore.R
            (* ^ assembly key store *)
        }.

Definition prepare_assemblycache (c : configuration) (size : positive) : assemblycache :=
    {| acenvs := nil
     ; acsize := Pos.to_nat size
     ; acwriteenv := EnvironmentWritable.initial_environment c
     ; acconfig := c
     ; acwriteq := (mkwritequeue nil qsize)
     ; acreadq := (mkreadqueue nil qsize)
     ; acfbstore := FBlockListStore.init c
     ; ackstore := KeyListStore.init c
    |}.
```

```cpp
struct CoqAssemblyCache::pimpl {
public:
    pimpl(const CoqConfiguration &c, const int n_envs, const int depth)
        : _config(c), n_readenvs(n_envs), n_queuesz(depth)
    {};
    ~pimpl() {};

    std::optional<std::shared_ptr<CoqEnvironmentReadable>> try_restore_assembly(const CoqAssembly::aid_t & sel_aid);
    std::optional<std::shared_ptr<CoqEnvironmentReadable>> ensure_assembly(const CoqAssembly::aid_t & sel_aid);

    std::shared_ptr<CoqFBlockStore> get_fblock_store() const;
    std::shared_ptr<CoqKeyStore> get_key_store() const;

    void register_key_store(std::shared_ptr<CoqKeyStore> &st);
    void register_fblock_store(std::shared_ptr<CoqFBlockStore> &st);

    bool enqueue_read_request(const ReadQueueEntity &);
    bool enqueue_write_request(const WriteQueueEntity &);
    std::vector<ReadQueueResult> run_read_requests(const std::vector<ReadQueueEntity> & rreqs);
    std::vector<ReadQueueResult> iterate_read_queue();
    std::vector<WriteQueueResult> run_write_requests(const std::vector<WriteQueueEntity> & wreqs);
    std::vector<WriteQueueResult> iterate_write_queue();
    void flush();
    void close();
    CoqAssemblyCache::vmetric_t metrics() const;

private:
    const CoqConfiguration _config;
    std::deque<std::shared_ptr<CoqEnvironmentReadable>> _readenvs{};
    const int n_readenvs;
    std::shared_ptr<CoqEnvironmentWritable> _writeenv{nullptr};
    const int n_queuesz;
    std::vector<ReadQueueEntity> _readqueue;
    std::vector<WriteQueueEntity> _writequeue;

    std::shared_ptr<CoqKeyStore> _keystore{nullptr};
    std::shared_ptr<CoqFBlockStore> _fblockstore{nullptr};

    std::mutex _write_mutex;
    std::mutex _read_mutex;

    // stats
    uint64_t backup_bytes{0};
    uint64_t restored_bytes{0};
    uint32_t n_read_requests{0};
    uint32_t n_write_requests{0};
    uint32_t n_iterate_reads{0};
    uint32_t n_iterate_writes{0};
    uint32_t n_assembly_finalised{0};
    uint32_t n_assembly_recalled{0};
};
```

```cpp
CoqAssemblyCache::CoqAssemblyCache(const CoqConfiguration & c, const int n_envs, const int depth)
  : _pimpl(new pimpl(c,n_envs,depth))
{}

CoqAssemblyCache::~CoqAssemblyCache() = default;
// TODO on exit if there is a _writeenv, then finalise it
```

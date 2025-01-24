module;
/*
    eLyKseeR or LXR - cryptographic data archiving software
    https://github.com/eLyKseeR/elykseer-cpp
    Copyright (C) 2018-2025 Alexander Diemand

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <deque>
#include <functional>
#include <memory>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

import lxr_coqassembly;
import lxr_coqassemblycache;
import lxr_coqbuffer;
import lxr_coqconfiguration;
import lxr_coqenvironment;
import lxr_coqstore;


module lxr_coqassemblycache;


namespace lxr {

// Private implementation

/*
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
*/

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
    std::shared_ptr<CoqFInfoStore> get_finfo_store() const;

    void register_key_store(std::shared_ptr<CoqKeyStore> &st);
    void register_fblock_store(std::shared_ptr<CoqFBlockStore> &st);
    void register_finfo_store(std::shared_ptr<CoqFInfoStore> &st);

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
    std::shared_ptr<CoqFInfoStore> _finfostore{nullptr};

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

CoqAssemblyCache::CoqAssemblyCache(const CoqConfiguration & c, const int n_envs, const int depth)
  : _pimpl(new pimpl(c,n_envs,depth))
{}

CoqAssemblyCache::~CoqAssemblyCache() = default;
// TODO on exit if there is a _writeenv, then finalise it

// Store file blocks

void CoqAssemblyCache::register_fblock_store(std::shared_ptr<CoqFBlockStore> &st)
{
    if (_pimpl) {
        _pimpl->register_fblock_store(st);
    }
}

void CoqAssemblyCache::pimpl::register_fblock_store(std::shared_ptr<CoqFBlockStore> &st)
{
    _fblockstore = st;
}

std::shared_ptr<CoqFBlockStore> CoqAssemblyCache::get_fblock_store() const
{
    return _pimpl->get_fblock_store();
}
std::shared_ptr<CoqFBlockStore> CoqAssemblyCache::pimpl::get_fblock_store() const
{
    return _fblockstore;
}

// Store file information

void CoqAssemblyCache::register_finfo_store(std::shared_ptr<CoqFInfoStore> &st)
{
    if (_pimpl) {
        _pimpl->register_finfo_store(st);
    }
}

void CoqAssemblyCache::pimpl::register_finfo_store(std::shared_ptr<CoqFInfoStore> &st)
{
    _finfostore = st;
}

std::shared_ptr<CoqFInfoStore> CoqAssemblyCache::get_finfo_store() const
{
    return _pimpl->get_finfo_store();
}
std::shared_ptr<CoqFInfoStore> CoqAssemblyCache::pimpl::get_finfo_store() const
{
    return _finfostore;
}

// Store encryption keys

void CoqAssemblyCache::register_key_store(std::shared_ptr<CoqKeyStore> &st)
{
    if (_pimpl) {
        _pimpl->register_key_store(st);
    }
}

void CoqAssemblyCache::pimpl::register_key_store(std::shared_ptr<CoqKeyStore> &st)
{
    _keystore = st;
}

std::shared_ptr<CoqKeyStore> CoqAssemblyCache::get_key_store() const
{
    return _pimpl->get_key_store();
}
std::shared_ptr<CoqKeyStore> CoqAssemblyCache::pimpl::get_key_store() const
{
    return _keystore;
}

// Restore assembly

//Given its assembly identifier, restore an assembly from chunks, wrap it in a readable environment and place it in front of the list of environments.

/*
```coq
Program Definition try_restore_assembly (ac : assemblycache) (sel_aid : Assembly.aid_t) : option EnvironmentReadable.E :=
    match KeyListStore.find sel_aid ac.(ackstore) with
    | None => None
    | Some ki =>
        EnvironmentReadable.restore_assembly (EnvironmentReadable.initial_environment ac.(acconfig)) sel_aid ki
    end.

Program Definition set_envs (ac0 : assemblycache) (envs : list EnvironmentReadable.E) : assemblycache :=
    {| acenvs := envs; acsize := ac0.(acsize);
       acwriteenv := ac0.(acwriteenv); acconfig := ac0.(acconfig);
       acreadq := ac0.(acreadq); acwriteq := ac0.(acwriteq);
       acfbstore := ac0.(acfbstore); ackstore := ac0.(ackstore) |}.

(* ensure that an environment with assembly (by aid) is available
   and that it is in the head position of the list of envs *)
Program Definition ensure_assembly (ac0 : assemblycache) (sel_aid : Assembly.aid_t) : option (EnvironmentReadable.E * assemblycache) :=
    match ac0.(acenvs) with
    | nil => (* create first env *)
        match try_restore_assembly ac0 sel_aid with
        | None => None
        | Some env =>
           Some (env, set_envs ac0 (env :: nil))
        end
    | e1 :: nil =>
        if String.eqb (aid e1.(cur_assembly AssemblyPlainFull.B)) sel_aid then
            Some (e1, ac0)
        else
            match try_restore_assembly ac0 sel_aid with
            | None => None
            | Some env =>
                if Nat.eqb ac0.(acsize) 1
                then Some (env, set_envs ac0 (env :: nil))
                else Some (env, set_envs ac0 (env :: e1 :: nil))
            end
    | e1 :: r =>
        if String.eqb (aid e1.(cur_assembly AssemblyPlainFull.B)) sel_aid then
            (* found in first position *)
            Some (e1, ac0)
        else
            let found := List.filter (fun e => String.eqb (aid e.(cur_assembly AssemblyPlainFull.B)) sel_aid) r in
            match found with
            | efound :: _ =>
                (* found further down -> move to first position *)
                let r' := List.filter (fun e => negb (String.eqb (aid e.(cur_assembly AssemblyPlainFull.B)) sel_aid)) r in
                Some (efound, set_envs ac0 (efound :: e1 :: r'))
            | nil =>
                match try_restore_assembly ac0 sel_aid with
                | None => None
                | Some env =>
                    (* check number of envs <= acsize *)
                    let lr := if Nat.leb ac0.(acsize) (List.length ac0.(acenvs))
                                then List.removelast ac0.(acenvs)
                                else ac0.(acenvs) in
                    Some (env, set_envs ac0 (env :: lr))
                end
            end
    end.
```
*/
std::optional<std::shared_ptr<CoqEnvironmentReadable>>
CoqAssemblyCache::pimpl::try_restore_assembly(const CoqAssembly::aid_t & sel_aid)
{
    if (! _keystore) { return {}; }
    std::shared_ptr<CoqEnvironmentReadable> _env(new CoqEnvironmentReadable(_config));
    std::optional<CoqAssembly::KeyInformation> optki = _keystore->find(sel_aid);
    if (optki && _env->restore_assembly(sel_aid, optki.value())) {
        ++n_assembly_recalled;
        return _env;
    }
    return {};
}

std::optional<std::shared_ptr<CoqEnvironmentReadable>>
CoqAssemblyCache::pimpl::ensure_assembly(const CoqAssembly::aid_t & sel_aid)
{
    auto match_aid = [&sel_aid](std::shared_ptr<CoqEnvironmentReadable> const & env) { return env->_assembly && env->_assembly->aid() == sel_aid; };
    auto found = std::ranges::find_if(_readenvs.begin(), _readenvs.end(), match_aid);
    if (found != _readenvs.end()) {
        if (found != _readenvs.begin()) {
            // found the aid, move it to the front if not so already
            auto head = *found;
            _readenvs.erase(std::remove_if(_readenvs.begin(), _readenvs.end(), match_aid));
            _readenvs.push_front(head);
        }
        // std::clog << "    found " << sel_aid << std::endl;
        return *found;
    }

    auto n_envs = _readenvs.size();
    if (n_envs < n_readenvs) {
        // std::clog << "    free positions in _readenvs" << std::endl;
    } else {
        // std::clog << "    replace env in _readenvs" << std::endl;
        // drop last
        _readenvs.pop_back();
    }

    // push new to front
    auto new_env = try_restore_assembly(sel_aid);
    if (new_env) {
        // std::clog << "    new env " << sel_aid << std::endl;
        _readenvs.push_front(new_env.value());
        return new_env.value();
    } else {
        std::clog << "    failed to restore! " << sel_aid << std::endl;
        return {};
    }
    return {};
}

// Read request queue
/*
```coq
Program Fixpoint run_read_requests (ac0 : assemblycache) (reqs : list readqueueentity) (res : list readqueueresult) : (list readqueueresult * assemblycache) :=
    match reqs with
    | nil => (res, ac0)
    | h :: r =>
        let aid := h.(qaid) in
        match ensure_assembly ac0 aid with
        | None => (res, ac0)
        | Some (env, ac1) =>
            let buf := BufferPlain.buffer_create h.(qrlen) in
            let _ := assembly_get_content env.(cur_buffer AssemblyPlainFull.B) h.(qrlen) h.(qapos) buf in
            run_read_requests ac1 r ({| readrequest := h; rresult := buf |} :: res)
        end
    end.
```
*/
std::vector<ReadQueueResult> CoqAssemblyCache::pimpl::run_read_requests(const std::vector<ReadQueueEntity> & rreqs)
{
    std::vector<ReadQueueResult> results{};
    // TODO parallelise
    for (auto const & rreq : rreqs) {
        auto const &_aid = rreq._aid;
        ensure_assembly(rreq._aid);
        lxr::CoqAssembly::BlockInformation bi;
        bi.blockid = 1; bi.blocksize = rreq._rlen; bi.blockaid = rreq._aid; bi.blockapos = rreq._apos;
        if (auto const &_renv = _readenvs.front(); _renv->_assembly) {
            std::shared_ptr<lxr::CoqBufferPlain> restored_block = _renv->_assembly->restore(bi);
            restored_bytes += restored_block->len();
            lxr::ReadQueueResult rres;
            rres._readrequest = rreq;
            rres._rresult = restored_block;
            { // protected:
                results.push_back(rres);
                ++n_read_requests;
            }
        }
    }
    return results;
}

/*
```coq
Program Definition iterate_read_queue (ac0 : assemblycache) : (list readqueueresult * assemblycache) :=
    match rqueue ac0.(acreadq) with
    | nil => (nil, ac0)
    | h :: r =>
        let aid := h.(qaid) in
        let sel := List.filter (fun e => String.eqb e.(qaid) aid) r in
        let ac1 := {| acenvs := ac0.(acenvs); acsize := ac0.(acsize); acwriteenv := ac0.(acwriteenv); acconfig := ac0.(acconfig);
                      acreadq := {| rqueue := List.filter (fun e => negb (String.eqb e.(qaid) aid)) r; rqueuesz := ac0.(acreadq).(rqueuesz) |};
                      acwriteq := ac0.(acwriteq)|} in
        run_read_requests ac1 (h :: sel) nil
    end.
```
*/
std::vector<ReadQueueResult> CoqAssemblyCache::iterate_read_queue()
{
    if (_pimpl) {
        return _pimpl->iterate_read_queue();
    }
    return {};
}
std::vector<ReadQueueResult> CoqAssemblyCache::pimpl::iterate_read_queue()
{
    std::lock_guard<std::mutex> guard(_read_mutex);
    // std::clog << "AssemblyCache::iterate_read_queue  readqueue size = " << _pimpl->_readqueue.size() << std::endl;
    if (! _readqueue.empty()) {
        ++n_iterate_reads;
        std::clog << "CoqAssemblyCache::iterate_read_queue  size of queue: " << _readqueue.size() << std::endl;
        std::vector<lxr::ReadQueueEntity> rmatches = _readqueue;
        _readqueue.clear();
        return run_read_requests(rmatches);
    } else {
        return {};
    }
    return {};
}

/*
```coq
Program Definition enqueue_read_request (ac : assemblycache) (req : readqueueentity) : (bool * assemblycache) :=
    let ln := List.length (rqueue ac.(acreadq)) in
    if N.leb (pos2N qsize) (nat2N ln) then
        (false, ac)
    else
        (true, {| acenvs := ac.(acenvs); acsize := ac.(acsize); acwriteenv := ac.(acwriteenv); acconfig := ac.(acconfig);
                  acreadq := {| rqueue := List.app (rqueue ac.(acreadq)) (req :: nil); rqueuesz := rqueuesz ac.(acreadq) |};
                  acwriteq := ac.(acwriteq)  |}).
```
*/
bool CoqAssemblyCache::enqueue_read_request(const ReadQueueEntity &rqe)
{
    if (_pimpl) {
        return _pimpl->enqueue_read_request(rqe);
    }
    return false;
}

bool CoqAssemblyCache::pimpl::enqueue_read_request(const ReadQueueEntity &rqe)
{
    if (_readqueue.size() >= n_queuesz) {
        return false;
    } else {
        std::lock_guard<std::mutex> guard(_read_mutex);
        ++n_iterate_reads;
        _readqueue.push_back(rqe);
    }
    return true;
}

// Write request queue
/*
```coq
Program Fixpoint run_write_requests (ac0 : assemblycache) (reqs : list writequeueentity) (res : list writequeueresult)
                                  : (list writequeueresult * assemblycache) :=
    match reqs with
    | nil => (res, ac0)
    | h :: r =>
        let env := EnvironmentWritable.backup ac0.(acwriteenv) h.(qfhash) h.(qfpos) h.(qbuffer) in
        let ac1 := {| acenvs := ac0.(acenvs); acsize := ac0.(acsize); acwriteenv := env; acconfig := ac0.(acconfig);
                      acreadq := ac0.(acreadq); acwriteq := {| wqueue := nil; wqueuesz := wqueuesz ac0.(acwriteq) |} |} in
        run_write_requests ac1 r ({| writerequest := h;
                                     wresult := {| qaid := env.(cur_assembly AssemblyPlainWritable.B).(aid); qapos := env.(cur_assembly AssemblyPlainWritable.B).(apos);
                                                   qrlen := buffer_len h.(qbuffer) |} |} :: res)
    end.
```
*/
std::vector<WriteQueueResult> CoqAssemblyCache::pimpl::run_write_requests(const std::vector<WriteQueueEntity> & wreqs)
{
    for (auto const & wreq : wreqs) {
        if (! _writeenv) {
            _writeenv.reset(new CoqEnvironmentWritable(_config));
            _writeenv->recreate_assembly();
        }
        if (wreq._buffer) {
            std::pair<CoqEnvironment::rel_fname_fblocks, CoqEnvironment::rel_aid_keys> backup_result = _writeenv->backup(wreq._fhash.toHex(), wreq._fpos, *wreq._buffer, wreq._buffer->len());
            for (auto const & fb : backup_result.first) {
                _fblockstore->add(fb.first, fb.second);
            }
            for (auto const & ki : backup_result.second) {
                _keystore->add(ki.first, ki.second);
            }
            backup_bytes += wreq._buffer->len();
        }
    }
    return {};
}

/*
```coq
Program Definition iterate_write_queue (ac0 : assemblycache) : (list writequeueresult * assemblycache) :=
    match wqueue ac0.(acwriteq) with
    | nil => (nil, ac0)
    | h :: r =>
        let ac1 := {| acenvs := ac0.(acenvs); acsize := ac0.(acsize); acwriteenv := ac0.(acwriteenv); acconfig := ac0.(acconfig);
                      acreadq := ac0.(acreadq); acwriteq := {| wqueue := nil; wqueuesz := wqueuesz ac0.(acwriteq) |} |} in
        run_write_requests ac1 (h :: r) nil
    end.
```
*/
std::vector<WriteQueueResult> CoqAssemblyCache::iterate_write_queue()
{
    if (_pimpl) {
        return _pimpl->iterate_write_queue();
    }
    return {};
}
std::vector<WriteQueueResult> CoqAssemblyCache::pimpl::iterate_write_queue()
{
    if (! _writequeue.empty()) {
        std::vector<WriteQueueEntity> wq;
        {
            std::lock_guard<std::mutex> guard(_write_mutex);
            ++n_iterate_writes;
            // make a copy of the write queue
            wq = std::move(_writequeue);
            _writequeue.clear();
        }

        // execute write requests
        return run_write_requests(wq);
    }
    return {};
}

/*
```coq
Program Definition enqueue_write_request (ac : assemblycache) (req : writequeueentity) : (bool * assemblycache) :=
    let ln := List.length (wqueue ac.(acwriteq)) in
    if N.leb (pos2N qsize) (nat2N ln) then
        (false, ac)
    else
        (true, {| acenvs := ac.(acenvs); acsize := ac.(acsize); acwriteenv := ac.(acwriteenv); acconfig := ac.(acconfig);
                  acwriteq := {| wqueue := List.app (wqueue ac.(acwriteq)) (req :: nil); wqueuesz := wqueuesz ac.(acwriteq) |};
                  acreadq := ac.(acreadq)  |}).
```
*/
bool CoqAssemblyCache::enqueue_write_request(const WriteQueueEntity &wqe)
{
    if (_pimpl) {
        return _pimpl->enqueue_write_request(wqe);
    }
    return false;
}

bool CoqAssemblyCache::pimpl::enqueue_write_request(const WriteQueueEntity &wqe)
{
    if (_writequeue.size() >= n_queuesz) {
        return false;
    } else {
        std::lock_guard<std::mutex> guard(_write_mutex);
        ++n_write_requests;
        _writequeue.push_back(std::move(wqe));
    }
    return true;
}

// Varia
/*
```coq
Program Definition flush (ac0 : assemblycache) : assemblycache :=
    match EnvironmentWritable.finalise_and_recreate_assembly ac0.(acwriteenv) with
    | None => ac0
    | Some (env', (aid, ki)) =>
        let ackstore' := KeyListStore.add aid ki ac0.(ackstore) in
        {| acenvs := nil; acsize := ac0.(acsize); acwriteenv := env'; acconfig := ac0.(acconfig);
        acreadq := ac0.(acreadq); acwriteq := ac0.(acwriteq);
        acfbstore := ac0.(acfbstore); ackstore := ackstore'; acfistore := ac0.(acfistore) |}
    end.
```
*/
void CoqAssemblyCache::flush()
{
    if (_pimpl) {
        _pimpl->flush();
    }
}

void CoqAssemblyCache::pimpl::flush()
{
    if (_writeenv) {
        ++n_assembly_finalised;
        auto okey = _writeenv->finalise_and_recreate_assembly();
        if (_keystore && okey) {
            _keystore->add(okey.value().first, okey.value().second);
        }
    } else {
        _writeenv->recreate_assembly();
    }
}

/*
```coq
Program Definition close (ac0 : assemblycache) : assemblycache :=
    let env := EnvironmentWritable.finalise_assembly ac0.(acwriteenv) in
    {| acenvs := nil; acsize := ac0.(acsize); acwriteenv := env; acconfig := ac0.(acconfig);
       acreadq := ac0.(acreadq); acwriteq := ac0.(acwriteq) |}.
```
*/
void CoqAssemblyCache::close()
{
    if (_pimpl) {
        _pimpl->close();
    }
}

void CoqAssemblyCache::pimpl::close()
{
    if (_writeenv) {
        ++n_assembly_finalised;
        auto okey = _writeenv->finalise_assembly();
        if (_keystore && okey) {
            _keystore->add(okey.value().first, okey.value().second);
        }
    }
}

CoqAssemblyCache::vmetric_t CoqAssemblyCache::metrics() const
{
    if (_pimpl) {
        return _pimpl->metrics();
    }
    return {};
}

CoqAssemblyCache::vmetric_t CoqAssemblyCache::pimpl::metrics() const
{
    CoqAssemblyCache::vmetric_t _metrics{};
    _metrics.push_back({"backup_bytes", backup_bytes});
    _metrics.push_back({"restored_bytes", restored_bytes});
    _metrics.push_back({"n_read_requests", n_read_requests});
    _metrics.push_back({"n_write_requests", n_write_requests});
    _metrics.push_back({"n_iterate_reads", n_iterate_reads});
    _metrics.push_back({"n_iterate_writes", n_iterate_writes});
    _metrics.push_back({"n_read_environments", (uint32_t)_readenvs.size()});
    _metrics.push_back({"n_write_environments", _writeenv ? uint32_t(1) : uint32_t(0)});
    _metrics.push_back({"n_assembly_finalised", n_assembly_finalised});
    _metrics.push_back({"n_assembly_recalled", n_assembly_recalled});
    _metrics.push_back({"len_read_queue", (uint32_t)_readqueue.size()});
    _metrics.push_back({"len_write_queue", (uint32_t)_writequeue.size()});
    std::ostringstream _ss;
    _ss << "[";
    int idx=0;
    for (auto const &re : _readenvs) {
        if (re && re->_assembly) {
            if (idx > 0) { _ss << ", "; }
            _ss << re->_assembly->aid();
            idx++;
        }
    }
    _ss << "]";
    _metrics.push_back({"read_aid_list", _ss.str()});
    _ss.str("");
    _ss << "[";
    if (_writeenv && _writeenv->_assembly) {
        _ss << _writeenv->_assembly->aid();
    }
    _ss << "]";
    _metrics.push_back({"write_aid_list", _ss.str()});
    return _metrics;
}

} // namespace
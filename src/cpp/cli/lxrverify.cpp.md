```cpp

/*
<fpaste ../../../src/copyright.md>
*/

#include "lxr/coqassemblycache.hpp"
#include "lxr/coqconfiguration.hpp"
#include "lxr/coqprocessor.hpp"
#include "lxr/coqstore.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/gpg.hpp"
#include "lxr/liz.hpp"
#include "lxr/os.hpp"
#include "lxr/sha3.hpp"

#include <chrono>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <getopt.h>

```

## options
```cpp
    static int dry_run = 0;
    static int verbose_out = 0;
    static int use_gpg = 1;
    static struct option longopts[] = {
             { "dry-run",   no_argument,        &dry_run,       1   },
             { "verbose",   no_argument,        &verbose_out,   1   },
             { "help",      no_argument,        NULL,           'h' },
             { "version",   no_argument,        NULL,           'V' },
             { "license",   no_argument,        NULL,           'L' },
             { "copyright", no_argument,        NULL,           'C' },
             { "pChunks",   required_argument,  NULL,           'x' },
             { "pDbFb",     required_argument,  NULL,           'b' },
             { "pKeys",     required_argument,  NULL,           'k' },
             { "file",      required_argument,  NULL,           'f' },
            //  { "dir",       required_argument,  NULL,           'd' },
             { "myid",      required_argument,  NULL,           'y' },
             { "nogpg",     no_argument,        &use_gpg,       0 },
             { NULL,         0,                 NULL,           0 }
    };

```

keep lists of files and directories to restore:
```cpp
constexpr int MAX_RESTORE_FILES = 8192;
// constexpr int MAX_RESTORE_DIRS = 128;

static int counter_files = 0;
// static int counter_dirs = 0;

static std::string list_files[MAX_RESTORE_FILES];
// static std::string list_dirs[MAX_RESTORE_DIRS];
```

## parsing cli arguments
```cpp
void output_error() {
  std::clog << "error!" << std::endl;
  exit(1);
}

void output_help() {
  std::cout << "usage: " << std::endl;
  std::cout << "-h --help      shows this help" << std::endl;
  std::cout << "-V --version   shows version information" << std::endl;
  std::cout << "-L --license   shows license" << std::endl;
  std::cout << "-C --copyright shows copyright" << std::endl;
  std::cout << "--verbose      verbose output" << std::endl;
  std::cout << "--dry-run      just validate arguments" << std::endl;
  std::cout << "--nogpg        turn off decrypion of meta data using GnuPG" << std::endl;
  std::cout << "-x --pChunks   sets path for encrypted chunks" << std::endl;
  std::cout << "-b --pDbFb     adds backup meta data dbfb (*)" << std::endl;
  std::cout << "-k --pKeys     adds backup encryption keys dbks (*)" << std::endl;
  std::cout << "-y --myid      sets unique identifier for this local instance" << std::endl;
  std::cout << "-f --file      file to be restored (*)" << std::endl;
  std::cout << "-d --dir       files in directory to be restored (*)" << std::endl;
  std::cout << "options labelled with (*) can be provided multiple times on the command line." << std::endl;
}

void output_version() {
  std::cout << lxr::Liz::version() << std::endl;
}

void output_license() {
  std::cout << lxr::Liz::license() << std::endl;
}

void output_copyright() {
  std::cout << lxr::Liz::copyright() << std::endl;
}

void set_chunk_path(std::optional<std::string> & chunkpath, const std::string & p) {
  if (lxr::FileCtrl::dirExists(p)) {
    chunkpath = p;
  } else {
    std::clog << "chunk directory does not exist: " << p << std::endl;
    output_error();
  }
}

void add_dbfb_path(std::shared_ptr<lxr::CoqFBlockStore> & db, const std::string & fp) {
  if (lxr::FileCtrl::fileExists(fp)) {
    int sz0 = db->size();
    if (use_gpg == 1) {
      lxr::Gpg gpg;
      gpg.decrypt_from_file(fp);
      db->inStream(gpg.istream());
    } else {
      std::ifstream _if; _if.open(fp, std::ios::binary);
      db->inStream(_if); _if.close();
    }
    int sz1 = db->size();
    std::clog << "       ### read " << sz1 - sz0 << " blocks from FBlockStore" << std::endl;
  } else {
    std::clog << "blocks database file does not exist: " << fp << std::endl;
    output_error();
  }
}

void add_dbkey_path(std::shared_ptr<lxr::CoqKeyStore> & db, const std::string & fp) {
  if (lxr::FileCtrl::fileExists(fp)) {
    int sz0 = db->size();
    if (use_gpg == 1) {
      lxr::Gpg gpg;
      gpg.decrypt_from_file(fp);
      db->inStream(gpg.istream());
    } else {
      std::ifstream _if; _if.open(fp, std::ios::binary);
      db->inStream(_if); _if.close();
    }
    int sz1 = db->size();
    std::clog << "       ### read " << sz1 - sz0 << " keys from KeyStore" << std::endl;
  } else {
    std::clog << "encryption keys database file does not exist: " << fp << std::endl;
    output_error();
  }
}

void restore_file(std::string const p) {
  if (counter_files >= MAX_RESTORE_FILES -1 ) {
    std::clog << "maximum number of files reached: " << MAX_RESTORE_FILES << std::endl;
    output_error();
  }

  list_files[counter_files++] = p;
}

void set_myid(std::optional<std::string> &myid, std::string const p) {
  if (p.length() > 5) {
    myid = p;
  } else {
    std::clog << "myid needs to be at least 6 characters: " << p << std::endl;
    output_error();
  }
}

// void restore_dir(std::string const p) {
//   if (counter_dirs >= MAX_RESTORE_DIRS -1 ) {
//     std::clog << "maximum number of directories reached: " << MAX_RESTORE_DIRS << std::endl;
//     output_error();
//   }

//   list_dirs[counter_dirs++] = p;
// }

// bool hasPrefix(std::string_view prefix, std::string_view str)
// {
//     return prefix == str.substr(0, prefix.size());
// }
```

## main
```cpp
int main (int argc, char * const argv[]) {
  std::optional<int> nchunks{16};
  std::optional<std::string> myid{};
  std::optional<std::string> chunkpath{};

  lxr::CoqConfiguration _config;
  _config.nchunks(16);

  std::shared_ptr<lxr::CoqKeyStore> _keystore{new lxr::CoqKeyStore(_config)};
  std::shared_ptr<lxr::CoqFBlockStore> _fblockstore{new lxr::CoqFBlockStore(_config)};
  std::shared_ptr<lxr::CoqFInfoStore> _finfostore{new lxr::CoqFInfoStore(_config)};

  int ch;
  while ((ch = getopt_long(argc, argv, "hVLCx:b:k:f:y:", longopts, NULL)) != -1) {
    //std::cout << "argument '" << ch << "'" << std::endl;
    switch (ch) {
      case 'h': output_help(); break;
      case 'V': output_version(); break;
      case 'L': output_license(); break;
      case 'C': output_copyright(); break;
      case 'x': set_chunk_path(chunkpath, optarg); break;
      case 'b': add_dbfb_path(_fblockstore, optarg); break;
      case 'k': add_dbkey_path(_keystore, optarg); break;
      case 'f': restore_file(optarg); break;
      // case 'd': restore_dir(optarg); break;
      case 'y': set_myid(myid, optarg); break;
      default : break; //output_error(); return 1;
    }
  }
  argc -= optind;
  argv += optind;
  if (optind < 2) {
    std::clog << "no arguments; use '-h' for help" << std::endl;
    output_error(); return 1;
  }

  // check arguments
  if (! myid) { std::clog << "missing: myid" << std::endl; output_error(); }
  if (! chunkpath) { std::clog << "missing: chunk path" << std::endl; output_error(); }

  {
    _config.nchunks(nchunks.value());
    _config.my_id = myid.value();
    _config.path_chunks = chunkpath.value();
  }

  auto t0 = std::chrono::system_clock::now();

  std::shared_ptr<lxr::CoqAssemblyCache> qac(new lxr::CoqAssemblyCache(_config, 3, 12));
  qac->register_key_store(_keystore);
  qac->register_fblock_store(_fblockstore);
  qac->register_finfo_store(_finfostore);
  lxr::CoqProcessor qproc(_config, qac);

  // prepare files to restore
  int corr_files = 0;
  for (int i = 0; i < counter_files; i++) {
    //std::cout << "checking " << i << "  " << list_files[i] << std::endl;
    auto const fhash = lxr::Sha3_256::hash(list_files[i]).toHex();
    if (! _fblockstore->contains(fhash)) {
      list_files[i] = "";
      corr_files++;
    }
  }
  // for (int i = 0; i < counter_dirs; i++) {
  //   db_fp.appKeys([i](std::string const & fp) {
  //     //std::cout << fp << std::endl;
  //     if (hasPrefix(list_dirs[i], fp)) {
  //       if (counter_files >= MAX_RESTORE_FILES - 1) {
  //         std::clog << "maximum number of files reached: " << MAX_RESTORE_FILES << std::endl;
  //         output_error();
  //       } else {
  //         list_files[counter_files++] = fp;
  //       }
  //     }
  //   });
  // }

  // organise files by assembly
  // TODO

  // output count of files
  std::cout << "going to verify " << (counter_files - corr_files) << " files." << std::endl;
  {
    for (int i = 0; i < counter_files; i++) {
      std::cout << "    " << (i+1) << "  " << list_files[i] << std::endl;
    }
  }

  if (dry_run != 0) {
    std::cout << "bye." << std::endl;
    return 0;
  }

  // run program
  bool res = true;
  for (int i = 0; i < counter_files; i++) {
    if (list_files[i] != "") {
      auto const fhash = lxr::Sha3_256::hash(list_files[i]).toHex();
      if (auto const fbs = _fblockstore->find(fhash); fbs) {
          auto process_read_blocks = [&res](const std::vector<lxr::ReadQueueResult> & rqrs) {
            for (auto const &rqr : rqrs) {
              if (rqr._rresult) {
                // rqr._rresult->fileout_sz_pos(0, rqr._rresult->len(), tgt);
                auto const optchksum = rqr._rresult->calc_checksum();
                if (optchksum && optchksum.value().toHex() == rqr._readrequest._chksum.toHex()) {
                  res &= true;
                } else {
                  res &= false;
                }
              }
            }
          };
          for (auto const bi : fbs.value()) {
            lxr::ReadQueueEntity rqe;
            rqe._aid = bi.blockaid; rqe._apos = bi.blockapos;
            rqe._rlen = bi.blocksize; rqe._fpos = bi.filepos;
            rqe._chksum = bi.bchecksum;
            std::clog << "   read request @" << bi.filepos << " in aid = " << bi.blockaid << std::endl;
            if (! qac->enqueue_read_request(rqe)) {
              process_read_blocks(qac->iterate_read_queue());
              qac->enqueue_read_request(rqe);
            }
          }
          process_read_blocks(qac->iterate_read_queue());
      }
    }
  }

  auto t1 = std::chrono::system_clock::now();
  auto tdiff = std::chrono::round<std::chrono::microseconds>(t1 - t0).count();

  // show statistics
  if (verbose_out > 0) {
    auto metrics = qac->metrics();
    std::map<std::string, lxr::CoqAssemblyCache::mvalue_t> map_metrics{};
    map_metrics.insert(metrics.begin(), metrics.end());
    for (auto const &m : metrics) {
        std::clog << "  " << m.first << ": ";
        if (std::holds_alternative<std::string>(m.second))
            std::clog << std::get<std::string>(m.second) << std::endl;
        if (std::holds_alternative<uint32_t>(m.second))
            std::clog << std::get<uint32_t>(m.second) << std::endl;
        if (std::holds_alternative<uint64_t>(m.second))
            std::clog << std::get<uint64_t>(m.second) << std::endl;
    }
    auto bytes_read = std::get<uint64_t>(map_metrics["restored_bytes"]);
    std::clog << "bps: " << bytes_read * 1e6 / tdiff << std::endl;
  }

  // exit with status code 1 on failure; success = 0
  return (res ? 0 : 1);
}
```

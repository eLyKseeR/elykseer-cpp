```cpp

/*
<fpaste ../../../src/copyright.md>
*/

#include "lxr/coqassemblycache.hpp"
#include "lxr/coqconfiguration.hpp"
#include "lxr/coqprocessor.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/gpg.hpp"
#include "lxr/liz.hpp"
#include "lxr/os.hpp"

#include <ctime>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

```

## options
```cpp
    static int dry_run = 0;
    static int verbose_out = 0;
    static struct option longopts[] = {
             { "dry-run",   no_argument,        &dry_run,       1   },
             { "verbose",   no_argument,        &verbose_out,   1   },
             { "help",      no_argument,        NULL,           'h' },
             { "version",   no_argument,        NULL,           'V' },
             { "license",   no_argument,        NULL,           'L' },
             { "copyright", no_argument,        NULL,           'C' },
             { "pX",        required_argument,  NULL,           'x' },
             { "pD",        required_argument,  NULL,           'o' },
             { "pR",        required_argument,  NULL,           'r' },
            //  { "ref",       required_argument,  NULL,           'r' },
             { "f",         required_argument,  NULL,           'f' },
             { "d1",        required_argument,  NULL,           'd' },
             { "dr",        required_argument,  NULL,           'D' },
             { "nchunks",   required_argument,  NULL,           'n' },
             { "myid",      required_argument,  NULL,           'y' },
             { "owner-key", required_argument,  NULL,           'k' },
            //  { "compression", required_argument, NULL,          'c' },
            //  { "deduplication", required_argument, NULL,        'u' },
             { NULL,         0,                 NULL,           0 }
    };

```
keep lists of files and directories to backup:
```cpp
constexpr int MAX_BACKUP_FILES = 8192;
constexpr int MAX_BACKUP_DIRS = 128;

static int counter_files = 0;
static int counter_dir1 = 0;
static int counter_dirs = 0;

static std::string list_files[MAX_BACKUP_FILES];
static std::string list_dir1[MAX_BACKUP_DIRS];
static std::string list_dir[MAX_BACKUP_DIRS];
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
  std::cout << "-x --pX        sets output path for encrypted chunks" << std::endl;
  std::cout << "-o --pD        sets output path for meta data" << std::endl;
  std::cout << "-n --nchunks   sets number of chunks (16-256) per assembly" << std::endl;
  // std::cout << "-c --compression  sets compression (0 or 1)" << std::endl;
  // std::cout << "-u --deduplication sets duplication mode (0, 1 or 2)" << std::endl;
  // std::cout << "-r --ref       adds reference dbfp for deduplication (*)" << std::endl;
  std::cout << "-f             backups a file (*)" << std::endl;
  std::cout << "-d --d1        backups files in a directory (*)" << std::endl;
  std::cout << "-D --dr        recursively backups a directory and all its content (*)" << std::endl;
  std::cout << "-y --myid      sets unique identifier for this local instance" << std::endl;
  std::cout << "-k --owner-key looks up the PGP key and encrypts meta data with it" << std::endl;
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

void set_chunk_path(std::optional<std::string> &chunkpath, std::string const p) {
  if (lxr::FileCtrl::dirExists(p)) {
    chunkpath = p;
  } else {
    std::clog << "chunk output directory does not exist: " << p << std::endl;
    output_error();
  }
}

void set_meta_path(std::optional<std::string> &metapath, std::string const p) {
  if (lxr::FileCtrl::dirExists(p)) {
    metapath = p;
  } else {
    std::clog << "meta data output directory does not exist: " << p << std::endl;
    output_error();
  }
}

// void add_ref_path(lxr::BackupCtrl & ctrl, std::string const p) {
//   if (lxr::FileCtrl::fileExists(p)) {
//     lxr::DbFp _dbfp;
//     { std::ifstream _if; _if.open(p);
//       _dbfp.inStream(_if); _if.close(); }
//     ctrl.addReference(_dbfp);
//   } else {
//     std::clog << "reference DbFp does not exist: " << p << std::endl;
//     output_error();
//   }
// }

void set_backup_file(std::string const fp) {
  if (counter_files >= MAX_BACKUP_FILES -1 ) {
    std::clog << "maximum number of files reached: " << MAX_BACKUP_FILES << std::endl;
    output_error();
  }

  if (lxr::FileCtrl::fileExists(fp)) {
    list_files[counter_files++] = fp;
  } else {
    std::clog << "file to backup does not exist: " << fp << std::endl;
    output_error();
  }
}

void set_backup_dir1(std::string const fp) {
  if (counter_dir1 >= MAX_BACKUP_DIRS -1 ) {
    std::clog << "maximum number of directories reached: " << MAX_BACKUP_DIRS << std::endl;
    output_error();
  }

  if (lxr::FileCtrl::dirExists(fp)) {
    list_dir1[counter_dir1++] = fp;
  } else {
    std::clog << "dir to backup does not exist: " << fp << std::endl;
    output_error();
  }
}

void set_backup_dir_rec(std::string const fp) {
  if (counter_dirs >= MAX_BACKUP_DIRS -1 ) {
    std::clog << "maximum number of directories reached: " << MAX_BACKUP_DIRS << std::endl;
    output_error();
  }

  if (lxr::FileCtrl::dirExists(fp)) {
    list_dir[counter_dirs++] = fp;
  } else {
    std::clog << "dir to recursively backup does not exist: " << fp << std::endl;
    output_error();
  }
}

void set_n_chunks(std::optional<int> &nchunks, std::string const p) {
  int n = atoi(p.c_str());
  if (n >= 16 && n <= 256) {
    nchunks = n;
  } else {
    std::clog << "number of chunks, out of range (16-256): " << p << std::endl;
    output_error();
  }
}

// void set_compression(std::string const p) {
//   int n = atoi(p.c_str());
//   if (n >= 0 && n <= 1) {
//     lxr::Options::set().isCompressed(n==1);
//   } else {
//     std::clog << "compression, out of range (0-1): " << p << std::endl;
//     output_error();
//   }
// }

// void set_deduplication(std::string const p) {
//   int n = atoi(p.c_str());
//   if (n >= 0 && n <= 2) {
//     lxr::Options::set().isDeduplicated(n);
//   } else {
//     std::clog << "deduplication, out of range (0-2): " << p << std::endl;
//     output_error();
//   }
// }

void set_myid(std::optional<std::string> &myid, std::string const p) {
  if (p.length() > 5) {
    myid = p;
  } else {
    std::clog << "myid needs to be at least 6 characters: " << p << std::endl;
    output_error();
  }
}

void set_ownerkey(std::optional<std::string> &okey, std::string const p) {
  lxr::Gpg gpg;
  if (gpg.has_public_key(p)) {
    okey = p;
  } else {
    std::clog << "owner key does not exist: " << p << std::endl;
    output_error();
  }
}
```

## main
```cpp
int main (int argc, char * const argv[]) {
  std::optional<int> nchunks{16};
  std::optional<std::string> myid{};
  std::optional<std::string> okey{};
  std::optional<std::string> metapath{};
  std::optional<std::string> chunkpath{};
  int ch;
  while ((ch = getopt_long(argc, argv, "hVLCx:o:f:d:D:n:k:y:", longopts, NULL)) != -1) {
    switch (ch) {
      case 'h': output_help(); break;
      case 'V': output_version(); break;
      case 'L': output_license(); break;
      case 'C': output_copyright(); break;
      case 'x': set_chunk_path(chunkpath, optarg); break;
      case 'o': set_meta_path(metapath, optarg); break;
      // case 'r': add_ref_path(ctrl, optarg); break;
      case 'f': set_backup_file(optarg); break;
      case 'd': set_backup_dir1(optarg); break;
      case 'D': set_backup_dir_rec(optarg); break;
      case 'n': set_n_chunks(nchunks, optarg); break;
      case 'y': set_myid(myid, optarg); break;
      // case 'c': set_compression(optarg); break;
      // case 'u': set_deduplication(optarg); break;
      case 'k': set_ownerkey(okey, optarg); break;
      default : break; //output_error(); return 1;
    }
  }
  argc -= optind;
  argv += optind;

  // check arguments
  if (! myid) { std::clog << "missing: myid" << std::endl; output_error(); }
  if (! okey) { std::clog << "missing: GPG owner key" << std::endl; output_error(); }
  if (! metapath) { std::clog << "missing: metapath" << std::endl; output_error(); }
  if (! chunkpath) { std::clog << "missing: chunkpath" << std::endl; output_error(); }

  lxr::CoqConfiguration _config;
  {
      _config.my_id = myid.value();
      _config.nchunks(nchunks.value());
      _config.path_chunks = chunkpath.value();
      _config.path_db = metapath.value();
  }

  // output count of files
  if (verbose_out > 0) {
    std::cout << "going to backup " << counter_files << " files." << std::endl;
    std::cout << "going to backup " << counter_dir1 << " directories." << std::endl;
    std::cout << "going to backup " << counter_dirs << " directories recursively." << std::endl;
    // for (int i = 0; i < counter_files; i++) {
    //   std::cout << "    " << (i+1) << "  " << list_files[i] << std::endl;
    // }
  }

  if (dry_run != 0) {
    std::cout << "bye." << std::endl;
    return 0;
  }

  // run program
  std::shared_ptr<lxr::CoqKeyStore> _keystore{new lxr::CoqKeyStore(_config)};
  std::shared_ptr<lxr::CoqFBlockStore> _fblockstore{new lxr::CoqFBlockStore(_config)};
  std::shared_ptr<lxr::CoqAssemblyCache> qac(new lxr::CoqAssemblyCache(_config, 1, 12));
  qac->register_key_store(_keystore);
  qac->register_fblock_store(_fblockstore);
  lxr::CoqProcessor qproc(_config, qac);

  for (int i = 0; i < counter_dir1; i++) {
    qproc.directory_backup_0(list_dir1[i]);
  }

  for (int i = 0; i < counter_dirs; i++) {
    qproc.recursive_backup_0(list_dir[i]);
  }

  for (int i = 0; i < counter_files; i++) {
    qproc.file_backup(list_files[i]);
  }
  qproc.close();

  // output databases with timestamp
  std::string timestamp = lxr::OS::timestamp();
  std::filesystem::path pdb{_config.path_db};
  std::string _frp = okey.value();
  auto const fp_dbk = pdb / ("LXRbackup-" + timestamp + "-dbks.xml");
  _keystore->encrypted_output(fp_dbk, _frp);
  auto const fp_dbfp = pdb / ("LXRbackup-" + timestamp + "-dbfp.xml");
  _fblockstore->encrypted_output(fp_dbfp, _frp);

  // show statistics
  if (verbose_out > 0) {
    std::unordered_map<std::string, lxr::CoqAssemblyCache::mvalue_t> map_metrics1{};
    const auto vmetric = qac->metrics();
    for (const auto & m : vmetric) {
        std::clog << "  " << m.first << ": ";
        if (std::holds_alternative<std::string>(m.second))
            std::clog << std::get<std::string>(m.second) << std::endl;
        if (std::holds_alternative<uint32_t>(m.second))
            std::clog << std::get<uint32_t>(m.second) << std::endl;
        if (std::holds_alternative<uint64_t>(m.second))
            std::clog << std::get<uint64_t>(m.second) << std::endl;
    }
  }

  return 0;
}
```

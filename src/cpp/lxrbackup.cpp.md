```cpp

/*
<fpaste ../../src/copyright.md>
*/

#include "lxr/backupctrl.hpp"
#include "lxr/dbfp.hpp"
#include "lxr/dbkey.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/liz.hpp"
#include "lxr/options.hpp"
#include "lxr/os.hpp"

#include "boost/filesystem.hpp"

#include <iostream>
#include <string>
#include <ctime>
#include <getopt.h>

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
             { "ref",       required_argument,  NULL,           'r' },
             { "f",         required_argument,  NULL,           'f' },
             { "d1",        required_argument,  NULL,           'd' },
             { "dr",        required_argument,  NULL,           'D' },
             { "nchunks",   required_argument,  NULL,           'n' },
             { "compression", required_argument, NULL,          'c' },
             { "deduplication", required_argument, NULL,        'u' },
             { NULL,         0,                 NULL,           0 }
    };

```
keep lists of files and directories to backup:
```cpp
constexpr int MAX_BACKUP_FILES = 32768;
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
  std::cout << "-c --compression  sets compression (0 or 1)" << std::endl;
  std::cout << "-u --deduplication sets duplication mode (0, 1 or 2)" << std::endl;
  std::cout << "-r --ref --pR  adds reference dbfp for deduplication (*)" << std::endl;
  std::cout << "-f             backups a file (*)" << std::endl;
  std::cout << "-d --d1        backups files in a directory (*)" << std::endl;
  std::cout << "-D --dr        recursively backups a directory and all its content (*)" << std::endl;
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

void set_chunk_path(std::string const p) {
  if (lxr::FileCtrl::dirExists(p)) {
    lxr::Options::set().fpathChunks() = p;
  } else {
    std::clog << "chunk output directory does not exist: " << p << std::endl;
    output_error();
  }
}

void set_meta_path(std::string const p) {
  if (lxr::FileCtrl::dirExists(p)) {
    lxr::Options::set().fpathMeta() = p;
  } else {
    std::clog << "meta data output directory does not exist: " << p << std::endl;
    output_error();
  }
}

void add_ref_path(lxr::BackupCtrl & ctrl, std::string const p) {
  std::clog << "reference dbfp ignored; not yet iplemented" << std::endl;
  output_error();

  if (lxr::FileCtrl::fileExists(p)) {
    //ctrl.setReference(p);
  } else {
    std::clog << "reference dbfp does not exist: " << p << std::endl;
    output_error();
  }
}

void backup_file(std::string const fp) {
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

void backup_dir1(std::string const fp) {
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

void backup_dir_rec(std::string const fp) {
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

void set_n_chunks(std::string const p) {
  int n = atoi(p.c_str());
  if (n >= 16 && n <= 256) {
    lxr::Options::set().nChunks(n);
  } else {
    std::clog << "number of chunks, out of range (16-256): " << p << std::endl;
    output_error();
  }
}

void set_compression(std::string const p) {
  int n = atoi(p.c_str());
  if (n >= 0 && n <= 1) {
    lxr::Options::set().isCompressed(n==1);
  } else {
    std::clog << "compression, out of range (0-1): " << p << std::endl;
    output_error();
  }
}

void set_deduplication(std::string const p) {
  int n = atoi(p.c_str());
  if (n >= 0 && n <= 2) {
    lxr::Options::set().isDeduplicated(n);
  } else {
    std::clog << "deduplication, out of range (0-2): " << p << std::endl;
    output_error();
  }
}

```

## main
```cpp
int main (int argc, char * const argv[]) {
  lxr::BackupCtrl ctrl;

  // options
  auto const tmpd = boost::filesystem::temp_directory_path();
  lxr::Options::set().fpathChunks() = tmpd / "LXR";
  lxr::Options::set().fpathMeta() = tmpd / "meta";
  lxr::Options::set().nChunks(16);
  lxr::Options::set().isCompressed(true);

  int ch;
  while ((ch = getopt_long(argc, argv, "hVLCx:o:r:f:d:D:n:c:u:", longopts, NULL)) != -1) {
    switch (ch) {
      case 'h': output_help(); break;
      case 'V': output_version(); break;
      case 'L': output_license(); break;
      case 'C': output_copyright(); break;
      case 'x': set_chunk_path(optarg); break;
      case 'o': set_meta_path(optarg); break;
      case 'r': add_ref_path(ctrl, optarg); break;
      case 'f': backup_file(optarg); break;
      case 'd': backup_dir1(optarg); break;
      case 'D': backup_dir_rec(optarg); break;
      case 'n': set_n_chunks(optarg); break;
      case 'c': set_compression(optarg); break;
      case 'u': set_deduplication(optarg); break;
      default : break; //output_error(); return 1;
    }
  }
  argc -= optind;
  argv += optind;

  // prepare file list
  for (int i = 0; i < counter_dir1; i++) {
    if (lxr::FileCtrl::dirExists(list_dir1[i])) {
      for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(list_dir1[i])) {
        if (lxr::FileCtrl::isFileReadable(x)) {
          if (counter_files >= MAX_BACKUP_FILES -1 ) {
            std::clog << "maximum number of files reached: " << MAX_BACKUP_FILES << std::endl;
            output_error();
          }
          list_files[counter_files++]=x.path().native();
        }
      }
    }
  }
  for (int i = 0; i < counter_dirs; i++) {
    if (lxr::FileCtrl::dirExists(list_dir[i])) {
      auto vfiles = lxr::FileCtrl::fileListRecursive(list_dir[i]);
      for (auto const & x : vfiles) {
        if (lxr::FileCtrl::isFileReadable(x)) {
          if (counter_files >= MAX_BACKUP_FILES -1 ) {
            std::clog << "maximum number of files reached: " << MAX_BACKUP_FILES << std::endl;
            output_error();
          }
          list_files[counter_files++]=x.native();
        }
      }
    }
  }

  // output count of files
  std::cout << "going to backup " << counter_files << " files." << std::endl;
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
  for (int i = 0; i < counter_files; i++) {
    ctrl.backup(list_files[i]);
  }
  ctrl.finalize();

  // output databases with timestamp
  std::string timestamp = lxr::OS::timestamp();
  auto const fp_dbfp = lxr::Options::current().fpathMeta() / ("LXRbackup-" + timestamp + "-dbfp.xml");
  auto const fp_dbk = lxr::Options::current().fpathMeta() / ("LXRbackup-" + timestamp + "-dbks.xml");
  { std::ofstream _out1; _out1.open(fp_dbfp.native());
    ctrl.getDbFp().outStream(_out1);
    _out1.close();
  }
  { std::ofstream _out1; _out1.open(fp_dbk.native());
    ctrl.getDbKey().outStream(_out1);
    _out1.close();
  }

  // show statistics
  if (verbose_out > 0) {
    std::clog << "bytes read: " << ctrl.bytes_in() << std::endl;
    std::clog << "bytes written: " << ctrl.bytes_out() << std::endl;
    //std::clog << "encryption time: " << ctrl.time_encrypt() << std::endl;
    //std::clog << "reading time: " << ctrl.time_extract() << std::endl;
    //std::clog << "writting time: " << ctrl.time_write() << std::endl;
  }


  return 0;
}
```

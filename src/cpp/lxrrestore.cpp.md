```cpp

/*
<fpaste ../../src/copyright.md>
*/

#include "lxr/dbfp.hpp"
#include "lxr/dbkey.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/liz.hpp"
#include "lxr/options.hpp"
#include "lxr/os.hpp"
#include "lxr/restorectrl.hpp"

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
    static std::string output_path = "";
    static lxr::DbFp db_fp;
    static struct option longopts[] = {
             { "dry-run",   no_argument,        &dry_run,       1   },
             { "verbose",   no_argument,        &verbose_out,   1   },
             { "help",      no_argument,        NULL,           'h' },
             { "version",   no_argument,        NULL,           'V' },
             { "license",   no_argument,        NULL,           'L' },
             { "copyright", no_argument,        NULL,           'C' },
             { "pChunks",   required_argument,  NULL,           'x' },
             { "pData",     required_argument,  NULL,           'o' },
             { "pDbFp",     required_argument,  NULL,           'p' },
             { "pKeys",     required_argument,  NULL,           'k' },
             { "file",      required_argument,  NULL,           'f' },
             { "dir",       required_argument,  NULL,           'd' },
             { NULL,         0,                 NULL,           0 }
    };

```

keep lists of files and directories to restore:
```cpp
constexpr int MAX_RESTORE_FILES = 32768;
constexpr int MAX_RESTORE_DIRS = 128;

static int counter_files = 0;
static int counter_dirs = 0;

static std::string list_files[MAX_RESTORE_FILES];
static std::string list_dirs[MAX_RESTORE_DIRS];
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
  std::cout << "-x --pChunks   sets path for encrypted chunks" << std::endl;
  std::cout << "-o --pData     sets output path for decrypted data" << std::endl;
  std::cout << "-p --pDbFp     adds backup meta data dbfp (*)" << std::endl;
  std::cout << "-k --pKeys     adds backup encryption keys dbks (*)" << std::endl;
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

void set_chunk_path(std::string const p) {
  if (lxr::FileCtrl::dirExists(p)) {
    lxr::Options::set().fpathChunks() = p;
  } else {
    std::clog << "chunk output directory does not exist: " << p << std::endl;
    output_error();
  }
}

void set_output_path(std::string const p) {
  if (lxr::FileCtrl::dirExists(p)) {
    output_path = p;
  } else {
    std::clog << "data output directory does not exist: " << p << std::endl;
    output_error();
  }
}

void add_dbfp_path(lxr::RestoreCtrl & ctrl, std::string const p) {
  if (lxr::FileCtrl::fileExists(p)) {
    //std::cout << "add_dbfp_path 0" << std::endl;
    lxr::DbFp _dbfp;
    //std::cout << "add_dbfp_path 1" << std::endl;
    { std::ifstream _if; _if.open(p);
      _dbfp.inStream(_if); _if.close(); }
    //std::cout << "add_dbfp_path 2" << std::endl;
    ctrl.addDbFp(_dbfp);
    //std::cout << "add_dbfp_path 3" << std::endl;
    db_fp.unionWith(_dbfp);
    //std::cout << "add_dbfp_path 4" << std::endl;
  } else {
    std::clog << "meta data dbfp file does not exist: " << p << std::endl;
    output_error();
  }
}

void add_dbkey_path(lxr::RestoreCtrl & ctrl, std::string const p) {
  if (lxr::FileCtrl::fileExists(p)) {
    lxr::DbKey _dbks;
    { std::ifstream _if; _if.open(p);
      _dbks.inStream(_if); _if.close(); }
    ctrl.addDbKey(_dbks);
  } else {
    std::clog << "encryption keys dbks file does not exist: " << p << std::endl;
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

void restore_dir(std::string const p) {
  if (counter_dirs >= MAX_RESTORE_DIRS -1 ) {
    std::clog << "maximum number of directories reached: " << MAX_RESTORE_DIRS << std::endl;
    output_error();
  }

  list_dirs[counter_dirs++] = p;
}

bool hasPrefix(std::string_view prefix, std::string_view str)
{
    return prefix == str.substr(0, prefix.size());
}
```

## main
```cpp
int main (int argc, char * const argv[]) {
  lxr::RestoreCtrl ctrl;

  // options
  auto const tmpd = boost::filesystem::temp_directory_path();
  lxr::Options::set().fpathChunks() = tmpd / "LXR";

  int ch;
  while ((ch = getopt_long(argc, argv, "hVLCx:o:p:k:f:d:", longopts, NULL)) != -1) {
    //std::cout << "argument '" << ch << "'" << std::endl;
    switch (ch) {
      case 'h': output_help(); break;
      case 'V': output_version(); break;
      case 'L': output_license(); break;
      case 'C': output_copyright(); break;
      case 'x': set_chunk_path(optarg); break;
      case 'o': set_output_path(optarg); break;
      case 'p': add_dbfp_path(ctrl, optarg); break;
      case 'k': add_dbkey_path(ctrl, optarg); break;
      case 'f': restore_file(optarg); break;
      case 'd': restore_dir(optarg); break;
      default : break; //output_error(); return 1;
    }
  }
  argc -= optind;
  argv += optind;
  if (optind < 2) {
    std::clog << "no arguments; use '-h' for help" << std::endl;
    output_error(); return 1;
  }

  // check output path
  if (output_path == "") {
    std::clog << "no output path for decrypted data given." << std::endl;
    output_error(); return 1;
  }

  // prepare files to restore
  int corr_files = 0;
  for (int i = 0; i < counter_files; i++) {
    //std::cout << "checking " << i << "  " << list_files[i] << std::endl;
    if (! db_fp.contains(list_files[i])) {
      list_files[i] = "";
      corr_files++;
    }
  }
  for (int i = 0; i < counter_dirs; i++) {
    db_fp.appKeys([i](std::string const & fp) {
      //std::cout << fp << std::endl;
      if (hasPrefix(list_dirs[i], fp)) {
        if (counter_files >= MAX_RESTORE_FILES - 1) {
          std::clog << "maximum number of files reached: " << MAX_RESTORE_FILES << std::endl;
          output_error();
        } else {
          list_files[counter_files++] = fp;
        }
      }
    });
  }

  // organise files by assembly
  // TODO

  // output count of files
  std::cout << "going to restore " << (counter_files - corr_files) << " files." << std::endl;
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
    if (list_files[i] != "") {
      ctrl.restore(output_path, list_files[i]);
    }
  }

  // show statistics
  if (verbose_out > 0) {
    std::clog << "bytes read: " << ctrl.bytes_in() << std::endl;
    std::clog << "bytes written: " << ctrl.bytes_out() << std::endl;
    std::clog << "encryption time: " << ctrl.time_decrypt() << std::endl;
    std::clog << "extracting time: " << ctrl.time_extract() << std::endl;
    std::clog << "reading time: " << ctrl.time_read() << std::endl;
  }


  return 0;
}
```

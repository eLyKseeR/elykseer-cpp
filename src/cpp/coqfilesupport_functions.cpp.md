declared in [CoqStore](coqstore.hpp.md)

```cpp
std::optional<CoqFilesupport::FileInformation> CoqFilesupport::get_file_information(const filename &fn) noexcept
{
    FileInformation fi;

    std::filesystem::path fp{fn};
    if (! std::filesystem::exists(fp)) { return {}; }

    fi.fname = fn;
    fi.fchecksum = lxr::Sha256::hash(fp).toHex();

  #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    struct stat _fst;
    if (stat(fp.string().c_str(), &_fst) == 0) {
        fi.fsize = _fst.st_size;
        char _buf[65];
        snprintf(_buf, 65, "%d", _fst.st_uid);
        fi.fowner = std::string(_buf);
    //   snprintf(_buf, 65, "%d", _fst.st_gid);
    //   fi.fgroup = _buf;
  #if defined(__APPLE__) || defined(__FreeBSD__)
          fi.fmodified = OS::time2string(_fst.st_mtimespec.tv_sec);
  #elif defined(__linux__)
          fi.fmodified = OS::time2string(_fst.st_mtim.tv_sec);
  #endif
    } else {
        return {};
    }
  #else
    //#error Such a platform is not yet a good host for this software
    auto ftm = std::filesystem::last_write_time(fp);
    fi.fmodified = std::format("{}",ftm);
  #endif

    return std::move(fi);
}

```

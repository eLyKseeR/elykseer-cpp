declared in [FsUtils](fsutils.hpp.md)

## implementations

```cpp
std::string FsUtils::sep() noexcept
{
#ifdef _WIN32
    return "\\\\";
#else
    return "/";
#endif
}

const filepath FsUtils::cleanfp(filepath const & _fp) noexcept
{
#ifdef _WIN32
    std::string fp = _fp.string();
    fp.replace(fp.find(":"), 0, ",drive");
    return fp;
#else
    return _fp;
#endif
}

const std::string FsUtils::fstem() noexcept
{
    const std::string _machine = OS::hostname();
    const std::string _user = OS::username();
    const std::string _ts = OS::timestamp();
    return "lxr_" + _machine + "_" + _user + "_" + _ts;
}

const filepath FsUtils::tempdir() noexcept
{
    return std::filesystem::temp_directory_path();
}

const filepath FsUtils::tempfile() noexcept
{
    const filepath td = std::filesystem::temp_directory_path();
    const auto now = time(NULL);
    const filepath fname = filepath(std::string("_") + std::to_string(now) + ".temporary");
    return td / fname;
}

std::pair<const std::string, const std::string> FsUtils::osusrgrp(filepath const & fp)
{
#ifdef _WIN32
    std::string accname = "error";
    std::string domname = "error";
    HANDLE hFile;
    hFile = CreateFile(fp.string().c_str(),
                       GENERIC_READ, FILE_SHARE_READ,
                       NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
      PSID sidowner = nullptr;
      PSECURITY_DESCRIPTOR sd = nullptr;
      if (GetSecurityInfo(hFile, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION,
                          &sidowner, NULL, NULL, NULL, &sd) != ERROR_SUCCESS) {
        DWORD dwaccnm = 1; DWORD dwdomnm = 1;
        SID_NAME_USE siduse = SidTypeUnknown;
        LPTSTR _accountname = nullptr;
        LPTSTR _domainname = nullptr;
        if (LookupAccountSid(NULL, sidowner, _accountname, &dwaccnm,
                             _domainname, &dwdomnm, &siduse)) {
          _accountname = (LPTSTR)GlobalAlloc(GMEM_FIXED, dwaccnm * sizeof(wchar_t));
          _domainname = (LPTSTR)GlobalAlloc(GMEM_FIXED, dwdomnm * sizeof(wchar_t));
          if (LookupAccountSid(NULL, sidowner, _accountname, &dwaccnm,
                               _domainname, &dwdomnm, &siduse)) {
            if (_accountname) { accname = std::string(_accountname,dwaccnm); }
            if (_domainname) { domname = std::string(_domainname,dwdomnm); }
          }
        }
      }
    }
    return std::make_pair(accname, domname);
#else
    std::string _osusr = "error";
    std::string _osgrp = "error";
    struct stat _fi;
    stat(fp.string().c_str(), &_fi);  // Error check omitted
    constexpr int _blen = 512;
    char _buf[_blen];
    struct passwd _pw, *_retpw = NULL;
    if (getpwuid_r(_fi.st_uid, &_pw, _buf, _blen, &_retpw) == 0) {
        _osusr = _pw.pw_name;
    }
    struct group _gr, *_retgr = NULL;
    if (getgrgid_r(_fi.st_gid, &_gr, _buf, _blen, &_retgr) == 0) {
        _osgrp = _gr.gr_name;
    }
    return std::make_pair(_osusr, _osgrp);
#endif
}
```

## operators to concat file paths

```cpp
const filepath operator/(std::string const &a, std::string const &b) noexcept
{
    filepath fp{a};
    return fp /= b;
}

const filepath operator/(filepath const &a, std::string const &b) noexcept
{
    filepath fp{a};
    return fp /= b;
}

```

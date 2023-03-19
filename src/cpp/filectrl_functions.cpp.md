declared in [FileCtrl](filectrl.hpp.md)

```cpp

std::optional<std::string> FileCtrl::fileDate(std::filesystem::path const & fp) noexcept
{
    return {};
}

std::optional<std::filesystem::file_time_type> FileCtrl::fileLastWriteTime(std::filesystem::path const & fp) noexcept
{
    try {
        return std::filesystem::last_write_time(fp);
    } catch (...) { return {}; }
}

std::optional<uint64_t> FileCtrl::fileSize(std::filesystem::path const & fp) noexcept
{
    try {
        return std::filesystem::file_size(fp);
    } catch (...) { return {}; }
}

bool FileCtrl::fileExists(std::filesystem::path const & fp) noexcept
{
    try {
        return std::filesystem::exists(fp);
    } catch (...) { return {}; }
}

bool FileCtrl::isFileReadable(std::filesystem::path const & fp) noexcept
{
    try {
        auto s = std::filesystem::status(fp);
        return std::filesystem::is_regular_file(s);
    } catch (...) {}
    return false;
}

bool FileCtrl::dirExists(std::filesystem::path const & fp) noexcept
{
    try {
        auto s = std::filesystem::status(fp);
        return std::filesystem::is_directory(s);
    } catch (...) {}
    return false;
}

std::vector<std::filesystem::path> FileCtrl::fileListRecursive(std::filesystem::path const & fp)
{
    std::vector<std::filesystem::path> res;
    std::filesystem::directory_iterator _pit{fp};
    while (_pit != std::filesystem::directory_iterator{}) {
        if (auto fp2 = *_pit++; dirExists(fp2)) {
            auto dsub = fileListRecursive(fp2);
            res.reserve( res.size() + dsub.size() );
            res.insert( res.end(), dsub.begin(), dsub.end() );
        } else {
            res.push_back(fp2);
        }
    }
    return res;
}

```
declared in [Options](options.hpp.md)

```cpp

static Options _options;

Options const & Options::current() noexcept
{
  return _options;
}

Options & Options::set() noexcept
{
  return _options;
}

int Options::nChunks() const noexcept
{
  return _pimpl->_nchunks;
}

auto Options::nChunks(int v) noexcept -> Options&
{
  _pimpl->_nchunks = std::min(256, std::max(16, v));
  return *this;
}

int Options::nRedundancy() const noexcept
{
  return _pimpl->_nredundancy;
}

auto Options::nRedundancy(int v) noexcept -> Options&
{
  _pimpl->_nredundancy = v;
  return *this;
}

bool Options::isCompressed() const noexcept
{
  return _pimpl->_iscompressed;
}

auto Options::isCompressed(bool v) noexcept -> Options&
{
  _pimpl->_iscompressed = v;
  return *this;
}

int Options::isDeduplicated() const noexcept
{
  return _pimpl->_isdeduplicated;
}

auto Options::isDeduplicated(int v) noexcept -> Options&
{
  _pimpl->_isdeduplicated = v;
  return *this;
}

boost::filesystem::path const & Options::fpathChunks() const noexcept
{
  return _pimpl->_fpathchunks;
}

auto Options::fpathChunks(boost::filesystem::path const &p) noexcept -> Options&
{
  _pimpl->_fpathchunks = p;
  return *this;
}

boost::filesystem::path const & Options::fpathMeta() const noexcept
{
  return _pimpl->_fpathmeta;
}

auto Options::fpathMeta(boost::filesystem::path const &p) noexcept -> Options&
{
  _pimpl->_fpathmeta = p;
  return *this;
}

void Options::inStream(std::istream & ins)
{
    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(ins, pt,
      boost::property_tree::xml_parser::no_comments |
      boost::property_tree::xml_parser::trim_whitespace);
    for (auto root = pt.begin(); root != pt.end(); root++) {
        if (root->first == "Options") {
          fromXML(root);
          break;
        }
    }
}

void Options::fromXML(boost::property_tree::ptree::iterator &root)
{
  if (root->first == "Options") {
    for (auto ops = root->second.begin(); ops != root->second.end(); ops++) {
      if (ops->first == "memory") {
        for (auto mem = ops->second.begin(); mem != ops->second.end(); mem++) {
          if (mem->first == "<xmlattr>") {
            nChunks(mem->second.get<int>("nchunks")).
            nRedundancy(mem->second.get<int>("redundancy"));
          }
        }
      } else if (ops->first == "compression") {
        std::string o = ops->second.data();
        isCompressed(o == "on");
      } else if (ops->first == "deduplication") {
        for (auto ded = ops->second.begin(); ded != ops->second.end(); ded++) {
          if (ded->first == "<xmlattr>") {
            isDeduplicated(ded->second.get<int>("level"));
          }
        }
      } else if (ops->first == "fpaths") {
        fpathChunks(ops->second.get<std::string>("chunks")).
        fpathMeta(ops->second.get<std::string>("meta"));
      }
    }
  }
}

void Options::outStream(std::ostream & os) const
{
  os << "<Options>" << std::endl;
  os << "  <memory nchunks=\\"" << _pimpl->_nchunks << "\\" redundancy=\\"" << _pimpl->_nredundancy << "\\" />" << std::endl;
  os << "  <compression>" << (_pimpl->_iscompressed?"on":"off") << "</compression>" << std::endl;
  os << "  <deduplication level=\\"" << _pimpl->_isdeduplicated << "\\" />" << std::endl;
  os << "  <fpaths>" << std::endl;
  os << "    <meta>" << _pimpl->_fpathmeta.native() << "</meta>" << std::endl;
  os << "    <chunks>" << _pimpl->_fpathchunks.native() << "</chunks>" << std::endl;
  os << "  </fpaths>" << std::endl;
  os << "</Options>" << std::endl;
}

```

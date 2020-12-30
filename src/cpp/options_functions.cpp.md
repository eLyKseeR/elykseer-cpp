declared in [Options](options.hpp.md)

```cpp

static Options _options;

Options const & Options::current()
{
  return _options;
}

Options & Options::set()
{
  return _options;
}

int Options::nChunks() const
{
  return _pimpl->_nchunks;
}

void Options::nChunks(int v)
{
  _pimpl->_nchunks = std::min(256, std::max(16, v));
}

int Options::nRedundancy() const
{
  return _pimpl->_nredundancy;
}

void Options::nRedundancy(int v)
{
  _pimpl->_nredundancy = v;
}

bool Options::isCompressed() const
{
  return _pimpl->_iscompressed;
}

void Options::isCompressed(bool v)
{
  _pimpl->_iscompressed = v;
}

int Options::isDeduplicated() const
{
  return _pimpl->_isdeduplicated;
}

void Options::isDeduplicated(int v)
{
  _pimpl->_isdeduplicated = v;
}

boost::filesystem::path const & Options::fpathChunks() const
{
  return _pimpl->_fpathchunks;
}

boost::filesystem::path & Options::fpathChunks()
{
  return _pimpl->_fpathchunks;
}

boost::filesystem::path const & Options::fpathMeta() const
{
  return _pimpl->_fpathmeta;
}

boost::filesystem::path & Options::fpathMeta()
{
  return _pimpl->_fpathmeta;
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
            nChunks(mem->second.get<int>("nchunks"));
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
        fpathChunks() = std::move(ops->second.get<std::string>("chunks"));
        fpathMeta() = std::move(ops->second.get<std::string>("meta"));
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

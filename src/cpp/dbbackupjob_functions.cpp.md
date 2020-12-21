declared in [DbBackupJob](dbbackupjob.hpp.md)

```c++
DbJobDat::DbJobDat()
{
    //_regexincl.push_back(std::regex(".*"));
}

void DbJobDat::include(std::string const & pat)
{
    _regexincl.push_back(std::regex(pat));
    _strincl.push_back(pat);
}

void DbJobDat::exclude(std::string const & pat)
{
    _regexexcl.push_back(std::regex(pat));
    _strexcl.push_back(pat);
}

void DbJobDat::addFile(std::string const & fp)
{
    _paths.push_back(std::make_pair("file", fp));
}

void DbJobDat::addDirectory(std::string const & fp)
{
    _paths.push_back(std::make_pair("directory", fp));
}

void DbJobDat::addRecursive(std::string const & fp)
{
    _paths.push_back(std::make_pair("recursive", fp));
}

```

```c++
void DbBackupJob::inStream(std::istream & ins)
{
    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(ins, pt,
      boost::property_tree::xml_parser::no_comments |
      boost::property_tree::xml_parser::trim_whitespace);
    for (auto root = pt.begin(); root != pt.end(); root++) {
        if (root->first == "DbBackupJob") {
            for (auto dbj = root->second.begin(); dbj != root->second.end(); dbj++) {
                if (dbj->first == "Job") {
                    DbJobDat dat;
                    std::string _name;
                    for (auto job = dbj->second.begin(); job != dbj->second.end(); job++) {
                        if (job->first == "<xmlattr>") {
                            _name = job->second.get<std::string>("name");
                        } else if (job->first == "Options") {
                            dat._options = Options();
                            dat._options.fromXML(job);
                        } else if (job->first == "Paths") {
                            for (auto p = job->second.begin(); p != job->second.end(); p++) {
                                if (p->first == "path") {
                                    std::string _type("file");
                                    for (auto attr = p->second.begin(); attr != p->second.end(); attr++) {
                                        if (attr->first == "<xmlattr>") {
                                            _type = attr->second.get<std::string>("type");
                                        }
                                    }
                                    dat._paths.push_back(std::make_pair(_type, p->second.data()));
                                }
                            }
                        } else if (job->first == "Filters") {
                            for (auto fl = job->second.begin(); fl != job->second.end(); fl++) {
                                if (fl->first == "include") {
                                    auto s = fl->second.data();
                                    dat._strincl.push_back(s);
                                    dat._regexincl.push_back(std::regex(s));
                                } else if (fl->first == "exclude") {
                                    auto s = fl->second.data();
                                    dat._strexcl.push_back(s);
                                    dat._regexexcl.push_back(std::regex(s));
                                }
                            }
                        }
                    }
                    set(_name, dat);
                }
            }
        }
        break;
    }  
}
```

```fsharp
    member this.outStream (s : TextWriter) =
        //let refl1 = Reflection.Assembly.GetCallingAssembly()
        let refl2 = Reflection.Assembly.GetExecutingAssembly()
        //let xname = refl1.GetName()
        let aname = refl2.GetName()
        s.WriteLine("<?xml version=\"1.0\"?>")
        s.WriteLine("<DbBackupJob xmlns=\"http://spec.sbclab.com/lxr/v1.0\">")
        s.WriteLine("<library><name>{0}</name><version>{1}</version></library>", aname.Name, aname.Version.ToString())
        //s.WriteLine("<program><name>{0}</name><version>{1}</version></program>", xname.Name, xname.Version.ToString())
        s.WriteLine("<host>{0}</host>", System.Environment.MachineName)
        s.WriteLine("<user>{0}</user>", System.Environment.UserName)
        s.WriteLine("<date>{0}</date>", System.DateTime.Now.ToString("s"))
        this.idb.appValues (fun k (v : DbJobDat) ->
             s.WriteLine(@"  <Job name=""{0}"">", k)
             s.WriteLine("    <Paths>")
             v.paths |> Seq.iter(fun x ->
                 s.WriteLine(@"      <path type=""file"">{0}</path>", x) )
             s.WriteLine("    </Paths>")
             v.options.io.outStream s
             s.WriteLine("    <Filters>")
             v.regexexcl |> Seq.iter(fun x ->
                 s.WriteLine("    <exclude>{0}</exclude>", x) )
             v.regexincl |> Seq.iter(fun x ->
                 s.WriteLine("    <include>{0}</include>", x) )
             s.WriteLine("    </Filters>")
             s.WriteLine("  </Job>")
             )
        s.WriteLine("</DbBackupJob>")
        s.Flush()
```

```c++
void DbBackupJob::outStream(std::ostream & os) const
{
    os << "<?xml version=\\"1.0\\"?>" << std::endl;
    os << "<DbBackupJob xmlns=\\"http://spec.sbclab.com/lxr/v1.0\\">" << std::endl;
    os << "<library><name>" << Liz::name() << "</name><version>" << Liz::version() << "</version></library>" << std::endl;
    os << "<host>" << OS::hostname() << "</host>" << std::endl;
    os << "<user>" << OS::username() << "</user>" << std::endl;
    os << "<date>" << OS::timestamp() << "</date>" << std::endl;
    appValues([&os](std::string const & k, struct DbJobDat const & v) {
        os << "  <Job name=\\"" << k << "\\">" << std::endl;
        v._options.outStream(os);
        os << "    <Paths>" << std::endl;
        for (auto const & p : v._paths) {
            os << "    <path type=\\"" << p.first << "\\">" << p.second << "</path>" << std::endl;
        }
        os << "    </Paths>" << std::endl;
        os << "    <Filters>" << std::endl;
        for (auto const & p : v._strexcl) {
            os << "    <exclude>" << p << "</exclude>" << std::endl;
        }
        for (auto const & p : v._strincl) {
            os << "    <include>" << p << "</include>" << std::endl;
        }
        os << "    </Filters>" << std::endl;
        os << "  </Job>" << std::endl;
    });
    os << "</DbBackupJob>" << std::endl;
}
```

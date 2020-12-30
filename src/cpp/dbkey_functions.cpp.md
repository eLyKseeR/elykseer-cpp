declared in [DbKey](dbkey.hpp.md)

```xml
<?xml version="1.0"?>
<DbKey xmlns="http://spec.sbclab.com/lxr/v1.0">
<library><name>LXR</name><version>Version 1.5.1.1 TR5 - do not use for production</version></library>
<host>AlexBook.fritz.box</host>
<user>axeld</user>
<date>20201221T192020</date>
  <Key aid="a31d3b8015e00894ffed38da8acee6f14be5af8f9b30c7d12a58bfed57ba8d12" n="16" iv="5e945f0036e2d926bcf25a96d4a4333e">f1fdd850cb786a9be605fea3ab9da3b495ced136d30f7b9c2d35225797bd8ffd</Key>
  <Key aid="94ffed38da8acee6f14be5af8a31d3b8015e008f9b30c7d12a58bfed57ba8d12" n="64" iv="5708895eca21865e18269476c8491bed">7de1def001d68a95ae23bced73c8dcfa3a190a9971a2317f9a5d3de357c8b3fa</Key>
</DbKey>

```

```cpp
void DbKey::inStream(std::istream & ins)
{
    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(ins, pt,
      boost::property_tree::xml_parser::no_comments |
      boost::property_tree::xml_parser::trim_whitespace);
    for (auto root = pt.begin(); root != pt.end(); root++) {
        if (root->first == "DbKey") {
            for (auto k = root->second.begin(); k != root->second.end(); k++) {
                if (k->first == "Key") {
                    DbKeyBlock block;
                    block._key.fromHex(k->second.data());
                    std::string _aid;
                    for (auto attr = k->second.begin(); attr != k->second.end(); attr++) {
                        if (attr->first == "<xmlattr>") {
                            block._n = attr->second.get<int>("n");
                            _aid = attr->second.get<std::string>("aid");
                            block._iv.fromHex(attr->second.get<std::string>("iv"));
                        }
                    }
                    set(_aid, block);
                }
            }
        }
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
        s.WriteLine("<DbKey xmlns=\"http://spec.sbclab.com/lxr/v1.0\">")
        s.WriteLine("<library><name>{0}</name><version>{1}</version></library>", aname.Name, aname.Version.ToString())
        //s.WriteLine("<program><name>{0}</name><version>{1}</version></program>", xname.Name, xname.Version.ToString())
        s.WriteLine("<host>{0}</host>", System.Environment.MachineName)
        s.WriteLine("<user>{0}</user>", System.Environment.UserName)
        s.WriteLine("<date>{0}</date>", System.DateTime.Now.ToString("s"))
        this.idb.appValues (fun k v ->
             let l = sprintf "  <Key aid=\"%s\" n=\"%d\" iv=\"%s\">%s</Key>" k v.n (Key.toHex v.iv.Length v.iv) (Key256.toHex v.key) in
             s.WriteLine(l))
        s.WriteLine("</DbKey>")
        s.Flush()
```

```cpp

void DbKey::outStream(std::ostream & os) const
{
    os << "<?xml version=\\"1.0\\"?>" << std::endl;
    os << "<DbKey xmlns=\\"http://spec.sbclab.com/lxr/v1.0\\">" << std::endl;
    os << "<library><name>" << Liz::name() << "</name><version>" << Liz::version() << "</version></library>" << std::endl;
    os << "<host>" << OS::hostname() << "</host>" << std::endl;
    os << "<user>" << OS::username() << "</user>" << std::endl;
    os << "<date>" << OS::timestamp() << "</date>" << std::endl;
    appValues([&os](std::string const & k, struct DbKeyBlock const & v) {
        os << "  <Key aid=\\"" << k << "\\" n=\\"" << v._n << "\\" iv=\\"" << v._iv.toHex() << "\\">" << v._key.toHex() << "</Key>" << std::endl;
    });
    os << "</DbKey>" << std::endl;
}
```
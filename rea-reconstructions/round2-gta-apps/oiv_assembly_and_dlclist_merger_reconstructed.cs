// Reconstructed C# implementation of OpenIV .oiv (assembly.xml) Parser & dlclist.xml Merger
// Evidence source:
//   - OpenIV-PackageFormat/specification & gameconfig/install.xml
//   - MagicOGK-OIV-Builder & vMenu Newtonsoft.Json (11.0.0.0) managed artifact

using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Xml.Linq;

namespace Round2Reconstruction.Oiv
{
    public sealed class OivManifest
    {
        public string Name { get; set; } = string.Empty;
        public string Version { get; set; } = "1.0";
        public string Author { get; set; } = string.Empty;
        public List<(string ArchivePath, string SourceEntry)> AddOperations { get; } = new();
        public List<(string TargetXml, string XPath, string InnerXml)> XmlInjections { get; } = new();
    }

    public static class OivPackageProcessor
    {
        public static OivManifest ParseOivPackage(string oivPath)
        {
            using var zip = ZipFile.OpenRead(oivPath);
            var asmEntry = zip.GetEntry("assembly.xml")
                ?? throw new InvalidDataException("Missing assembly.xml in .oiv archive.");

            using var stream = asmEntry.Open();
            var doc = XDocument.Load(stream);
            var root = doc.Root ?? throw new InvalidDataException("Empty assembly.xml");

            var manifest = new OivManifest
            {
                Name = root.Element("metadata")?.Element("name")?.Value ?? "Unnamed",
                Version = root.Element("metadata")?.Element("version")?.Element("major")?.Value ?? "1",
                Author = root.Element("metadata")?.Element("author")?.Element("displayName")?.Value ?? ""
            };

            foreach (var arch in root.Descendants("archive"))
            {
                string archPath = arch.Attribute("path")?.Value ?? "";
                foreach (var add in arch.Elements("add"))
                {
                    string src = add.Attribute("source")?.Value ?? "";
                    manifest.AddOperations.Add((archPath, src));
                }
                foreach (var xml in arch.Elements("xml"))
                {
                    string target = xml.Attribute("path")?.Value ?? "";
                    foreach (var ins in xml.Elements("add"))
                    {
                        string xpath = ins.Attribute("xpath")?.Value ?? "";
                        manifest.XmlInjections.Add((target, xpath, ins.Value));
                    }
                }
            }
            return manifest;
        }

        public static void MergeDlcListEntry(string dlcListXmlPath, string dlcName)
        {
            var doc = XDocument.Load(dlcListXmlPath);
            var pathsNode = doc.Root?.Element("Paths")
                ?? throw new InvalidDataException("Missing <Paths> in dlclist.xml");

            string normalized = $"dlcpacks:/{dlcName.Trim('/')}/";
            bool exists = pathsNode.Elements("Item")
                .Any(e => string.Equals(e.Value.Trim(), normalized, StringComparison.OrdinalIgnoreCase));

            if (!exists)
            {
                pathsNode.Add(new XElement("Item", normalized));
                doc.Save(dlcListXmlPath);
            }
        }
    }
}

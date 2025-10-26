// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "OutputFormatter.h"
#include "Resources.h"
#include "Command.h"
#include <algorithm>

namespace AppInstaller::CLI::Execution
{
    OutputFormat ParseOutputFormat(std::string_view format)
    {
        std::string lowerFormat;
        lowerFormat.resize(format.size());
        std::transform(format.begin(), format.end(), lowerFormat.begin(),
            [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });

        if (lowerFormat == "json")
        {
            return OutputFormat::Json;
        }
        else if (lowerFormat == "xml")
        {
            return OutputFormat::Xml;
        }
        else if (lowerFormat == "text" || lowerFormat.empty())
        {
            return OutputFormat::Text;
        }
        else
        {
            throw CommandException(Resource::String::FormatUnsupportedError(Utility::LocIndView{ format }));
        }
    }

    OutputFormat GetOutputFormatFromContext(const Context& context)
    {
        if (context.Args.Contains(Args::Type::OutputFormat))
        {
            return ParseOutputFormat(context.Args.GetArg(Args::Type::OutputFormat));
        }
        return OutputFormat::Text;
    }

    // JsonOutputFormatter implementation
    void JsonOutputFormatter::StartOutput()
    {
        m_root = Json::Value(Json::objectValue);
        m_packagesArray = Json::Value(Json::arrayValue);
        m_featuresArray = Json::Value(Json::arrayValue);
        m_sourcesArray = Json::Value(Json::arrayValue);
    }

    void JsonOutputFormatter::EndOutput()
    {
        if (!m_packagesArray.empty())
        {
            m_root["packages"] = m_packagesArray;
        }
        if (!m_featuresArray.empty())
        {
            m_root["features"] = m_featuresArray;
        }
        if (!m_sourcesArray.empty())
        {
            m_root["sources"] = m_sourcesArray;
        }
        if (m_truncated)
        {
            m_root["truncated"] = true;
        }
        if (!m_errors.empty())
        {
            Json::Value errorsArray(Json::arrayValue);
            for (const auto& error : m_errors)
            {
                errorsArray.append(error);
            }
            m_root["errors"] = errorsArray;
        }
    }

    std::string JsonOutputFormatter::GetOutput() const
    {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        return Json::writeString(builder, m_root);
    }

    void JsonOutputFormatter::AddPackageEntry(const std::string& name, const std::string& id,
                                              const std::string& version, const std::string& match,
                                              const std::string& source)
    {
        Json::Value package(Json::objectValue);
        package["name"] = name;
        package["id"] = id;
        if (!version.empty())
        {
            package["version"] = version;
        }
        if (!match.empty())
        {
            package["match"] = match;
        }
        if (!source.empty())
        {
            package["source"] = source;
        }
        m_packagesArray.append(package);
    }

    void JsonOutputFormatter::AddListEntry(const std::string& name, const std::string& id,
                                          const std::string& version, const std::string& availableVersion,
                                          const std::string& source)
    {
        Json::Value package(Json::objectValue);
        package["name"] = name;
        package["id"] = id;
        if (!version.empty())
        {
            package["version"] = version;
        }
        if (!availableVersion.empty())
        {
            package["availableVersion"] = availableVersion;
        }
        if (!source.empty())
        {
            package["source"] = source;
        }
        m_packagesArray.append(package);
    }

    void JsonOutputFormatter::AddFeatureEntry(const std::string& name, const std::string& status,
                                             const std::string& property, const std::string& link)
    {
        Json::Value feature(Json::objectValue);
        feature["name"] = name;
        feature["status"] = status;
        feature["property"] = property;
        feature["link"] = link;
        m_featuresArray.append(feature);
    }

    void JsonOutputFormatter::AddSourceEntry(const std::string& name, const std::string& type,
                                            const std::string& arg, const std::string& data,
                                            const std::string& updated)
    {
        Json::Value source(Json::objectValue);
        source["name"] = name;
        if (!type.empty())
        {
            source["type"] = type;
        }
        if (!arg.empty())
        {
            source["arg"] = arg;
        }
        if (!data.empty())
        {
            source["data"] = data;
        }
        if (!updated.empty())
        {
            source["updated"] = updated;
        }
        m_sourcesArray.append(source);
    }

    void JsonOutputFormatter::SetTruncated(bool truncated)
    {
        m_truncated = truncated;
    }

    void JsonOutputFormatter::AddError(const std::string& message)
    {
        m_errors.push_back(message);
    }

    // XmlOutputFormatter implementation
    void XmlOutputFormatter::StartOutput()
    {
        m_output << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
        m_output << "<root>\n";
    }

    void XmlOutputFormatter::EndOutput()
    {
        if (m_truncated)
        {
            m_output << "  <truncated>true</truncated>\n";
        }
        if (!m_errors.empty())
        {
            m_output << "  <errors>\n";
            for (const auto& error : m_errors)
            {
                m_output << "    <error>" << EscapeXml(error) << "</error>\n";
            }
            m_output << "  </errors>\n";
        }
        m_output << "</root>\n";
    }

    std::string XmlOutputFormatter::GetOutput() const
    {
        return m_output.str();
    }

    void XmlOutputFormatter::AddPackageEntry(const std::string& name, const std::string& id,
                                            const std::string& version, const std::string& match,
                                            const std::string& source)
    {
        m_output << "  <package>\n";
        m_output << "    <name>" << EscapeXml(name) << "</name>\n";
        m_output << "    <id>" << EscapeXml(id) << "</id>\n";
        if (!version.empty())
        {
            m_output << "    <version>" << EscapeXml(version) << "</version>\n";
        }
        if (!match.empty())
        {
            m_output << "    <match>" << EscapeXml(match) << "</match>\n";
        }
        if (!source.empty())
        {
            m_output << "    <source>" << EscapeXml(source) << "</source>\n";
        }
        m_output << "  </package>\n";
    }

    void XmlOutputFormatter::AddListEntry(const std::string& name, const std::string& id,
                                         const std::string& version, const std::string& availableVersion,
                                         const std::string& source)
    {
        m_output << "  <package>\n";
        m_output << "    <name>" << EscapeXml(name) << "</name>\n";
        m_output << "    <id>" << EscapeXml(id) << "</id>\n";
        if (!version.empty())
        {
            m_output << "    <version>" << EscapeXml(version) << "</version>\n";
        }
        if (!availableVersion.empty())
        {
            m_output << "    <availableVersion>" << EscapeXml(availableVersion) << "</availableVersion>\n";
        }
        if (!source.empty())
        {
            m_output << "    <source>" << EscapeXml(source) << "</source>\n";
        }
        m_output << "  </package>\n";
    }

    void XmlOutputFormatter::AddFeatureEntry(const std::string& name, const std::string& status,
                                            const std::string& property, const std::string& link)
    {
        m_output << "  <feature>\n";
        m_output << "    <name>" << EscapeXml(name) << "</name>\n";
        m_output << "    <status>" << EscapeXml(status) << "</status>\n";
        m_output << "    <property>" << EscapeXml(property) << "</property>\n";
        m_output << "    <link>" << EscapeXml(link) << "</link>\n";
        m_output << "  </feature>\n";
    }

    void XmlOutputFormatter::AddSourceEntry(const std::string& name, const std::string& type,
                                           const std::string& arg, const std::string& data,
                                           const std::string& updated)
    {
        m_output << "  <source>\n";
        m_output << "    <name>" << EscapeXml(name) << "</name>\n";
        if (!type.empty())
        {
            m_output << "    <type>" << EscapeXml(type) << "</type>\n";
        }
        if (!arg.empty())
        {
            m_output << "    <arg>" << EscapeXml(arg) << "</arg>\n";
        }
        if (!data.empty())
        {
            m_output << "    <data>" << EscapeXml(data) << "</data>\n";
        }
        if (!updated.empty())
        {
            m_output << "    <updated>" << EscapeXml(updated) << "</updated>\n";
        }
        m_output << "  </source>\n";
    }

    void XmlOutputFormatter::SetTruncated(bool truncated)
    {
        m_truncated = truncated;
    }

    void XmlOutputFormatter::AddError(const std::string& message)
    {
        m_errors.push_back(message);
    }

    std::string XmlOutputFormatter::EscapeXml(const std::string& str)
    {
        std::string result;
        result.reserve(str.size());

        for (char c : str)
        {
            switch (c)
            {
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            case '&':
                result += "&amp;";
                break;
            case '\"':
                result += "&quot;";
                break;
            case '\'':
                result += "&apos;";
                break;
            default:
                result += c;
                break;
            }
        }

        return result;
    }
}

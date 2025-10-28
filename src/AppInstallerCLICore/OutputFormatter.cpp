// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "OutputFormatter.h"
#include "ExecutionContext.h"
#include "Resources.h"
#include "Command.h"
#include "VTSupport.h"
#include <json/json.h>
#include <algorithm>

namespace AppInstaller::CLI::Execution
{
    // PIMPL implementation structure for JsonOutputFormatter
    struct JsonOutputFormatter::Impl
    {
        Json::Value m_root;
        Json::Value m_packagesArray;
        Json::Value m_featuresArray;
        Json::Value m_sourcesArray;
        bool m_truncated = false;
        std::vector<std::string> m_errors;
    };
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

    std::string FormatTimePointAsISO8601(const std::chrono::system_clock::time_point& timePoint)
    {
        // Check if this is epoch (never updated)
        if (timePoint == Utility::ConvertUnixEpochToSystemClock(0))
        {
            return "";
        }

        // Convert to time_t for formatting
        auto time = std::chrono::system_clock::to_time_t(timePoint);

        // Convert to UTC
        std::tm tm{};
#ifdef _WIN32
        gmtime_s(&tm, &time);
#else
        gmtime_r(&time, &tm);
#endif

        // Format as ISO-8601 (YYYY-MM-DDTHH:MM:SSZ)
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);

        return std::string(buffer);
    }

    // JsonOutputFormatter implementation
    JsonOutputFormatter::JsonOutputFormatter()
        : m_impl(std::make_unique<Impl>())
    {
    }

    JsonOutputFormatter::~JsonOutputFormatter() = default;

    JsonOutputFormatter::JsonOutputFormatter(JsonOutputFormatter&&) noexcept = default;

    JsonOutputFormatter& JsonOutputFormatter::operator=(JsonOutputFormatter&&) noexcept = default;

    void JsonOutputFormatter::StartOutput()
    {
        m_impl->m_root = Json::Value(Json::objectValue);
        m_impl->m_packagesArray = Json::Value(Json::arrayValue);
        m_impl->m_featuresArray = Json::Value(Json::arrayValue);
        m_impl->m_sourcesArray = Json::Value(Json::arrayValue);
    }

    void JsonOutputFormatter::EndOutput()
    {
        if (!m_impl->m_packagesArray.empty())
        {
            m_impl->m_root["packages"] = m_impl->m_packagesArray;
        }
        if (!m_impl->m_featuresArray.empty())
        {
            m_impl->m_root["features"] = m_impl->m_featuresArray;
        }
        if (!m_impl->m_sourcesArray.empty())
        {
            m_impl->m_root["sources"] = m_impl->m_sourcesArray;
        }
        if (m_impl->m_truncated)
        {
            m_impl->m_root["truncated"] = true;
        }
        if (!m_impl->m_errors.empty())
        {
            Json::Value errorsArray(Json::arrayValue);
            for (const auto& error : m_impl->m_errors)
            {
                errorsArray.append(error);
            }
            m_impl->m_root["errors"] = errorsArray;
        }
    }

    std::string JsonOutputFormatter::GetOutput() const
    {
        Json::StreamWriterBuilder builder;
        // Pretty-print for console, compact for redirected/piped output
        if (VirtualTerminal::IsConsoleOutput())
        {
            builder["indentation"] = "  ";
        }
        return Json::writeString(builder, m_impl->m_root);
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
        m_impl->m_packagesArray.append(package);
    }

    void JsonOutputFormatter::AddListEntry(const std::string& name, const std::string& id,
                                          const std::string& version, const std::string& availableVersion,
                                          const std::string& source, const std::string& category)
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
        if (!category.empty())
        {
            package["category"] = category;
        }
        m_impl->m_packagesArray.append(package);
    }

    void JsonOutputFormatter::AddFeatureEntry(const std::string& name, bool enabled,
                                             const std::string& property, const std::string& link)
    {
        Json::Value feature(Json::objectValue);
        feature["name"] = name;
        feature["enabled"] = enabled;
        feature["property"] = property;
        feature["link"] = link;
        m_impl->m_featuresArray.append(feature);
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
        m_impl->m_sourcesArray.append(source);
    }

    void JsonOutputFormatter::SetTruncated(bool truncated)
    {
        m_impl->m_truncated = truncated;
    }

    void JsonOutputFormatter::AddError(const std::string& message)
    {
        m_impl->m_errors.push_back(message);
    }

    // XmlOutputFormatter implementation
    void XmlOutputFormatter::StartOutput()
    {
        // Use compact mode when output is redirected/piped
        m_compact = !VirtualTerminal::IsConsoleOutput();

        m_output << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
        if (!m_compact) m_output << "\n";
        m_output << "<root>";
        if (!m_compact) m_output << "\n";
    }

    void XmlOutputFormatter::EndOutput()
    {
        if (m_truncated)
        {
            if (!m_compact) m_output << "  ";
            m_output << "<truncated>true</truncated>";
            if (!m_compact) m_output << "\n";
        }
        if (!m_errors.empty())
        {
            if (!m_compact) m_output << "  ";
            m_output << "<errors>";
            if (!m_compact) m_output << "\n";
            for (const auto& error : m_errors)
            {
                if (!m_compact) m_output << "    ";
                m_output << "<error>" << EscapeXml(error) << "</error>";
                if (!m_compact) m_output << "\n";
            }
            if (!m_compact) m_output << "  ";
            m_output << "</errors>";
            if (!m_compact) m_output << "\n";
        }
        m_output << "</root>";
        if (!m_compact) m_output << "\n";
    }

    std::string XmlOutputFormatter::GetOutput() const
    {
        return m_output.str();
    }

    void XmlOutputFormatter::AddPackageEntry(const std::string& name, const std::string& id,
                                            const std::string& version, const std::string& match,
                                            const std::string& source)
    {
        if (!m_compact) m_output << "  ";
        m_output << "<package>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "    ";
        m_output << "<name>" << EscapeXml(name) << "</name>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "    ";
        m_output << "<id>" << EscapeXml(id) << "</id>";
        if (!m_compact) m_output << "\n";
        if (!version.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<version>" << EscapeXml(version) << "</version>";
            if (!m_compact) m_output << "\n";
        }
        if (!match.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<match>" << EscapeXml(match) << "</match>";
            if (!m_compact) m_output << "\n";
        }
        if (!source.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<source>" << EscapeXml(source) << "</source>";
            if (!m_compact) m_output << "\n";
        }
        if (!m_compact) m_output << "  ";
        m_output << "</package>";
        if (!m_compact) m_output << "\n";
    }

    void XmlOutputFormatter::AddListEntry(const std::string& name, const std::string& id,
                                         const std::string& version, const std::string& availableVersion,
                                         const std::string& source, const std::string& category)
    {
        if (!m_compact) m_output << "  ";
        m_output << "<package>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "    ";
        m_output << "<name>" << EscapeXml(name) << "</name>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "    ";
        m_output << "<id>" << EscapeXml(id) << "</id>";
        if (!m_compact) m_output << "\n";
        if (!version.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<version>" << EscapeXml(version) << "</version>";
            if (!m_compact) m_output << "\n";
        }
        if (!availableVersion.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<availableVersion>" << EscapeXml(availableVersion) << "</availableVersion>";
            if (!m_compact) m_output << "\n";
        }
        if (!source.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<source>" << EscapeXml(source) << "</source>";
            if (!m_compact) m_output << "\n";
        }
        if (!category.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<category>" << EscapeXml(category) << "</category>";
            if (!m_compact) m_output << "\n";
        }
        if (!m_compact) m_output << "  ";
        m_output << "</package>";
        if (!m_compact) m_output << "\n";
    }

    void XmlOutputFormatter::AddFeatureEntry(const std::string& name, bool enabled,
                                            const std::string& property, const std::string& link)
    {
        if (!m_compact) m_output << "  ";
        m_output << "<feature>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "    ";
        m_output << "<name>" << EscapeXml(name) << "</name>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "    ";
        m_output << "<enabled>" << (enabled ? "true" : "false") << "</enabled>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "    ";
        m_output << "<property>" << EscapeXml(property) << "</property>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "    ";
        m_output << "<link>" << EscapeXml(link) << "</link>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "  ";
        m_output << "</feature>";
        if (!m_compact) m_output << "\n";
    }

    void XmlOutputFormatter::AddSourceEntry(const std::string& name, const std::string& type,
                                           const std::string& arg, const std::string& data,
                                           const std::string& updated)
    {
        if (!m_compact) m_output << "  ";
        m_output << "<source>";
        if (!m_compact) m_output << "\n";
        if (!m_compact) m_output << "    ";
        m_output << "<name>" << EscapeXml(name) << "</name>";
        if (!m_compact) m_output << "\n";
        if (!type.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<type>" << EscapeXml(type) << "</type>";
            if (!m_compact) m_output << "\n";
        }
        if (!arg.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<arg>" << EscapeXml(arg) << "</arg>";
            if (!m_compact) m_output << "\n";
        }
        if (!data.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<data>" << EscapeXml(data) << "</data>";
            if (!m_compact) m_output << "\n";
        }
        if (!updated.empty())
        {
            if (!m_compact) m_output << "    ";
            m_output << "<updated>" << EscapeXml(updated) << "</updated>";
            if (!m_compact) m_output << "\n";
        }
        if (!m_compact) m_output << "  ";
        m_output << "</source>";
        if (!m_compact) m_output << "\n";
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

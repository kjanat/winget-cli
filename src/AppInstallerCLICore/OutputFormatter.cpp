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
    /**
     * @brief Parses a case-insensitive output format string into an OutputFormat enum.
     *
     * @param format The format name to parse; recognized values are "json", "xml", and "text".
     *               Comparison is case-insensitive and an empty string is treated as "text".
     * @return OutputFormat The parsed output format: `OutputFormat::Json`, `OutputFormat::Xml`, or `OutputFormat::Text`.
     * @throws CommandException If `format` is not one of the recognized values.
     */
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

    /**
     * @brief Determines the desired output format from the execution context.
     *
     * @param context Execution context containing parsed command-line arguments.
     * @return OutputFormat `OutputFormat::Json` when the context's OutputFormat argument is "json", `OutputFormat::Xml` when it is "xml", and `OutputFormat::Text` when no OutputFormat argument is present.
     */
    OutputFormat GetOutputFormatFromContext(const Context& context)
    {
        if (context.Args.Contains(Args::Type::OutputFormat))
        {
            return ParseOutputFormat(context.Args.GetArg(Args::Type::OutputFormat));
        }
        return OutputFormat::Text;
    }

    /**
     * @brief Formats a time point as an ISO-8601 UTC timestamp.
     *
     * If the provided time point equals the Unix epoch (used to indicate "never updated"),
     * an empty string is returned.
     *
     * @param timePoint The time point to format (interpreted as system clock time).
     * @return std::string The formatted timestamp in "YYYY-MM-DDTHH:MM:SSZ" UTC form, or an empty string if `timePoint` is the epoch.
     */
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

    /**
     * @brief Constructs a JsonOutputFormatter and initializes its internal implementation.
     *
     * Initializes the private implementation (PIMPL) used to accumulate JSON output state.
     */
    JsonOutputFormatter::JsonOutputFormatter()
        : m_impl(std::make_unique<Impl>())
    {
    }

    /**
 * @brief Destroys the JsonOutputFormatter and releases its resources.
 *
 * Performs cleanup for the formatter's internal implementation.
 */
JsonOutputFormatter::~JsonOutputFormatter() = default;

    /**
 * @brief Move-constructs a JsonOutputFormatter by transferring ownership of its internal state.
 *
 * The source object's resources are moved into the new instance; the source is left in a valid but unspecified state.
 */
JsonOutputFormatter::JsonOutputFormatter(JsonOutputFormatter&&) noexcept = default;

    JsonOutputFormatter& JsonOutputFormatter::operator=(JsonOutputFormatter&&) noexcept = default;

    /**
     * @brief Initializes the internal JSON root and clears the package, feature, and source arrays.
     *
     * Prepares the formatter to start a new JSON output by resetting the root object and
     * replacing the packages, features, and sources members with empty JSON arrays.
     */
    void JsonOutputFormatter::StartOutput()
    {
        m_impl->m_root = Json::Value(Json::objectValue);
        m_impl->m_packagesArray = Json::Value(Json::arrayValue);
        m_impl->m_featuresArray = Json::Value(Json::arrayValue);
        m_impl->m_sourcesArray = Json::Value(Json::arrayValue);
    }

    /**
     * @brief Finalizes the JSON output by attaching accumulated sections to the root object.
     *
     * Adds the packages, features, and sources arrays to the root when they contain entries,
     * sets the `truncated` flag if truncation was recorded, and adds an `errors` array when errors were collected.
     */
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

    /**
     * @brief Serializes the accumulated JSON output to a string.
     *
     * The returned JSON is pretty-printed when output is a console and compact when redirected or piped.
     *
     * @return std::string The serialized JSON representation of the current output state.
     */
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

    /**
     * @brief Appends a package object to the JSON packages array.
     *
     * Creates a JSON object with the package's name and id and appends it to the internal
     * packages array. The `version`, `match`, and `source` fields are added only when their
     * corresponding string arguments are non-empty.
     *
     * @param name Package display name.
     * @param id   Package identifier.
     * @param version Optional package version; omitted if empty.
     * @param match   Optional match information; omitted if empty.
     * @param source  Optional source identifier; omitted if empty.
     */
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

    /**
     * @brief Adds a package entry to the JSON packages array using list-specific fields.
     *
     * Creates a JSON object with the provided values and appends it to the internal
     * packages array. `name` and `id` are always included; `version`, `availableVersion`,
     * `source`, and `category` are added only when non-empty.
     *
     * @param name Display name of the package.
     * @param id Unique identifier of the package.
     * @param version Currently installed or referenced version of the package (if any).
     * @param availableVersion Latest available version of the package from the source (if any).
     * @param source Identifier or name of the package source (if any).
     * @param category Category or classification of the package (if any).
     */
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

    /**
     * @brief Adds a feature entry to the JSON features array.
     *
     * Creates a JSON object with fields "name", "enabled", "property", and "link"
     * and appends it to the internal features array.
     *
     * @param name Human-readable feature name.
     * @param enabled `true` if the feature is enabled, `false` otherwise.
     * @param property Additional feature property or metadata string (may be empty).
     * @param link Related link or identifier for the feature (may be empty).
     */
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

    /**
     * @brief Appends a source entry to the JSON output's sources array.
     *
     * Creates a JSON object with the mandatory field `name` and optional fields
     * `type`, `arg`, `data`, and `updated` when their corresponding arguments are
     * non-empty, then appends it to the internal sources array.
     *
     * @param name Source name.
     * @param type Optional source type; omitted if empty.
     * @param arg Optional source argument; omitted if empty.
     * @param data Optional source data; omitted if empty.
     * @param updated Optional update timestamp string; omitted if empty.
     */
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

    /**
     * @brief Mark the output as truncated or not.
     *
     * @param truncated When `true`, indicates the produced output was truncated; when `false`, indicates it was not.
     */
    void JsonOutputFormatter::SetTruncated(bool truncated)
    {
        m_impl->m_truncated = truncated;
    }

    /**
     * @brief Adds an error message to the formatter's list of errors for JSON output.
     *
     * @param message Error message to include in the output's "errors" array.
     */
    void JsonOutputFormatter::AddError(const std::string& message)
    {
        m_impl->m_errors.push_back(message);
    }

    /**
     * @brief Initializes XML output state and writes the XML prolog and opening root element.
     *
     * Sets compact mode when output is redirected or piped, then writes the XML declaration
     * and the opening <root> element to the internal output stream, adding line breaks
     * only when not in compact mode.
     */
    void XmlOutputFormatter::StartOutput()
    {
        // Use compact mode when output is redirected/piped
        m_compact = !VirtualTerminal::IsConsoleOutput();

        m_output << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
        if (!m_compact) m_output << "\n";
        m_output << "<root>";
        if (!m_compact) m_output << "\n";
    }

    /**
     * @brief Finalizes XML output by emitting optional truncation and error elements and closing the root element.
     *
     * Emits a <truncated>true</truncated> element when the formatter is marked truncated.
     * Emits an <errors> block containing one <error> element per recorded error (each message is XML-escaped) when errors are present.
     * Closes the root element. Formatting (indentation and newlines) is controlled by the formatter's compact mode.
     */
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

    /**
     * @brief Retrieve the accumulated XML output from the formatter.
     *
     * @return std::string The complete XML document produced by the formatter. 
     */
    std::string XmlOutputFormatter::GetOutput() const
    {
        return m_output.str();
    }

    /**
     * @brief Appends a package element to the XML output.
     *
     * Adds a <package> element containing <name> and <id>, and includes optional
     * child elements <version>, <match>, and <source> when the corresponding
     * arguments are non-empty.
     *
     * @param name Package display name.
     * @param id Package identifier.
     * @param version Package version string; omitted if empty.
     * @param match Match type or criteria for the package; omitted if empty.
     * @param source Source identifier or location for the package; omitted if empty.
     */
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

    /**
     * @brief Appends a <package> element describing a package to the XML output.
     *
     * The element always contains <name> and <id> children. The <version>, <availableVersion>,
     * <source>, and <category> children are included only when their corresponding arguments
     * are non-empty.
     *
     * @param name Package display name.
     * @param id Package identifier.
     * @param version Installed package version; omitted if empty.
     * @param availableVersion Available/upstream package version; omitted if empty.
     * @param source Source identifier or argument for the package; omitted if empty.
     * @param category Package category or classification; omitted if empty.
     */
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

    /**
     * @brief Appends a <feature> element to the XML output with the specified fields.
     *
     * Writes a <feature> element containing child elements for name, enabled, property, and link
     * into the formatter's accumulated XML output, using compact or pretty formatting based on
     * the formatter's mode.
     *
     * @param name The feature's display name.
     * @param enabled `true` if the feature is enabled, `false` otherwise.
     * @param property Additional property information for the feature.
     * @param link A related link or reference for the feature.
     */
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

    /**
     * @brief Appends a <source> element to the XML output using the provided field values.
     *
     * Writes a <source> element containing a required <name> child and optional <type>, <arg>, <data>,
     * and <updated> children. Optional children are omitted when their corresponding string is empty.
     *
     * @param name The source name to include in the <name> element.
     * @param type The source type to include in the <type> element; omitted if empty.
     * @param arg The source argument to include in the <arg> element; omitted if empty.
     * @param data The source data to include in the <data> element; omitted if empty.
     * @param updated The source updated timestamp to include in the <updated> element; omitted if empty.
     */
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

    /**
     * @brief Marks the formatter's output as truncated.
     *
     * When set to `true`, the formatter will include a <truncated> element in the final XML output.
     *
     * @param truncated `true` to mark the output as truncated, `false` to clear the truncation flag.
     */
    void XmlOutputFormatter::SetTruncated(bool truncated)
    {
        m_truncated = truncated;
    }

    /**
     * @brief Record an error message for inclusion in the XML output.
     *
     * @param message The error text to append to the formatter's internal error list; these messages will be included in the output's <errors> section.
     */
    void XmlOutputFormatter::AddError(const std::string& message)
    {
        m_errors.push_back(message);
    }

    /**
     * @brief Escapes characters in a string to their XML entity equivalents.
     *
     * Replaces the characters '<', '>', '&', '\"', and '\'' with their XML entities
     * "&lt;", "&gt;", "&amp;", "&quot;", and "&apos;" respectively.
     *
     * @param str Input string to escape.
     * @return std::string The escaped string safe for inclusion in XML content.
     */
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
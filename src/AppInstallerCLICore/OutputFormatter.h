// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <string_view>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

/**
 * Available output formats.
 */
 
/**
 * Parse a human-readable format string into an OutputFormat value.
 * @param format Format string to parse (e.g., "text", "json", "xml").
 * @returns The corresponding OutputFormat value; behavior for unrecognized strings is implementation-defined.
 */
 
/**
 * Determine and validate the output format configured in the provided Context.
 * @param context Execution context containing output-related arguments/settings.
 * @returns The resolved OutputFormat based on the context.
 */

/**
 * Format a time_point as an ISO-8601 UTC timestamp.
 * @param timePoint Time point to format.
 * @returns ISO-8601 UTC string representation of timePoint; empty string if timePoint is epoch (represents "never updated").
 */

/**
 * Abstract interface for producing output in a specific format.
 */

/**
 * Begin generating output. Implementations should perform any initialization required before entries are added.
 */

/**
 * Finish generating output. Implementations should perform any finalization required after entries are added.
 */

/**
 * Retrieve the complete formatted output produced by this formatter.
 * @returns The formatted output as a string.
 */

/**
 * JSON formatter for structured output.
 */

/**
 * Destructor.
 */

/**
 * Begin JSON output generation.
 */

/**
 * Finalize JSON output generation.
 */

/**
 * Get the generated JSON output.
 * @returns The formatted JSON output as a string.
 */

/**
 * Add a package entry to the output.
 * @param name Package display name.
 * @param id Package identifier.
 * @param version Package version string.
 * @param match Matching criterion or reason for selection.
 * @param source Source where the package was found.
 */

/**
 * Add a list entry to the output.
 * @param name Package display name.
 * @param id Package identifier.
 * @param version Installed or listed version.
 * @param availableVersion Available version (if any).
 * @param source Source where the package is listed.
 * @param category Optional category for the package.
 */

/**
 * Add a feature entry to the output.
 * @param name Feature name.
 * @param enabled Whether the feature is enabled.
 * @param property Additional property or qualifier for the feature.
 * @param link Associated link or metadata for the feature.
 */

/**
 * Add a source entry to the output.
 * @param name Source name.
 * @param type Source type.
 * @param arg Source argument or identifier.
 * @param data Additional source-specific data.
 * @param updated Last-updated timestamp string for the source.
 */

/**
 * Mark whether the output has been truncated.
 * @param truncated True if the output was truncated, false otherwise.
 */

/**
 * Record an error message to include in the output.
 * @param message Error message to include.
 */

/**
 * XML formatter for structured output.
 */

/**
 * Begin XML output generation.
 */

/**
 * Finalize XML output generation.
 */

/**
 * Get the generated XML output.
 * @returns The formatted XML output as a string.
 */

/**
 * Add a package entry to the output.
 * @param name Package display name.
 * @param id Package identifier.
 * @param version Package version string.
 * @param match Matching criterion or reason for selection.
 * @param source Source where the package was found.
 */

/**
 * Add a list entry to the output.
 * @param name Package display name.
 * @param id Package identifier.
 * @param version Installed or listed version.
 * @param availableVersion Available version (if any).
 * @param source Source where the package is listed.
 * @param category Optional category for the package.
 */

/**
 * Add a feature entry to the output.
 * @param name Feature name.
 * @param enabled Whether the feature is enabled.
 * @param property Additional property or qualifier for the feature.
 * @param link Associated link or metadata for the feature.
 */

/**
 * Add a source entry to the output.
 * @param name Source name.
 * @param type Source type.
 * @param arg Source argument or identifier.
 * @param data Additional source-specific data.
 * @param updated Last-updated timestamp string for the source.
 */

/**
 * Mark whether the output has been truncated.
 * @param truncated True if the output was truncated, false otherwise.
 */

/**
 * Record an error message to include in the output.
 * @param message Error message to include.
 */

/**
 * Escape XML special characters in the provided string.
 * @param str Input string potentially containing XML special characters.
 * @returns A string with XML special characters escaped.
 */
namespace AppInstaller::CLI::Execution
{
    // Forward declaration to reduce header coupling
    struct Context;
    // Output format types
    enum class OutputFormat
    {
        Text,
        Json,
        Xml
    };

    // Parses format string to OutputFormat enum
    OutputFormat ParseOutputFormat(std::string_view format);

    // Validates and returns the output format from context arguments
    OutputFormat GetOutputFormatFromContext(const Context& context);

    // Formats a system_clock time_point as ISO-8601 UTC timestamp
    // Returns empty string for epoch (never updated)
    std::string FormatTimePointAsISO8601(const std::chrono::system_clock::time_point& timePoint);

    // Base formatter interface
    class IOutputFormatter
    {
    public:
        virtual ~IOutputFormatter() = default;
        virtual void StartOutput() = 0;
        virtual void EndOutput() = 0;
        virtual std::string GetOutput() const = 0;
    };

    // JSON formatter for structured output
    class JsonOutputFormatter : public IOutputFormatter
    {
    public:
        JsonOutputFormatter();
        ~JsonOutputFormatter() override;

        // Disable copy (PIMPL typically non-copyable without explicit implementation)
        JsonOutputFormatter(const JsonOutputFormatter&) = delete;
        JsonOutputFormatter& operator=(const JsonOutputFormatter&) = delete;

        // Enable move
        JsonOutputFormatter(JsonOutputFormatter&&) noexcept;
        JsonOutputFormatter& operator=(JsonOutputFormatter&&) noexcept;

        void StartOutput() override;
        void EndOutput() override;
        std::string GetOutput() const override;

        // Structured output methods
        void AddPackageEntry(const std::string& name, const std::string& id,
                           const std::string& version, const std::string& match,
                           const std::string& source);

        void AddListEntry(const std::string& name, const std::string& id,
                        const std::string& version, const std::string& availableVersion,
                        const std::string& source, const std::string& category = "");

        void AddFeatureEntry(const std::string& name, bool enabled,
                           const std::string& property, const std::string& link);

        void AddSourceEntry(const std::string& name, const std::string& type,
                          const std::string& arg, const std::string& data,
                          const std::string& updated);

        void SetTruncated(bool truncated);
        void AddError(const std::string& message);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

    // XML formatter for structured output
    class XmlOutputFormatter : public IOutputFormatter
    {
    public:
        XmlOutputFormatter() = default;

        void StartOutput() override;
        void EndOutput() override;
        std::string GetOutput() const override;

        // Structured output methods
        void AddPackageEntry(const std::string& name, const std::string& id,
                           const std::string& version, const std::string& match,
                           const std::string& source);

        void AddListEntry(const std::string& name, const std::string& id,
                        const std::string& version, const std::string& availableVersion,
                        const std::string& source, const std::string& category = "");

        void AddFeatureEntry(const std::string& name, bool enabled,
                           const std::string& property, const std::string& link);

        void AddSourceEntry(const std::string& name, const std::string& type,
                          const std::string& arg, const std::string& data,
                          const std::string& updated);

        void SetTruncated(bool truncated);
        void AddError(const std::string& message);

        // Public utility method for XML escaping
        static std::string EscapeXml(const std::string& str);

    private:
        std::ostringstream m_output;
        bool m_truncated = false;
        bool m_compact = false;
        std::vector<std::string> m_errors;
    };
}
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <string_view>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

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

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "ExecutionContext.h"
#include <json/json.h>
#include <string>
#include <vector>

namespace AppInstaller::CLI::Execution
{
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
        JsonOutputFormatter() = default;

        void StartOutput() override;
        void EndOutput() override;
        std::string GetOutput() const override;

        // Structured output methods
        void AddPackageEntry(const std::string& name, const std::string& id,
                           const std::string& version, const std::string& match,
                           const std::string& source);

        void AddListEntry(const std::string& name, const std::string& id,
                        const std::string& version, const std::string& availableVersion,
                        const std::string& source);

        void AddFeatureEntry(const std::string& name, const std::string& status,
                           const std::string& property, const std::string& link);

        void AddSourceEntry(const std::string& name, const std::string& type,
                          const std::string& arg, const std::string& data,
                          const std::string& updated);

        void SetTruncated(bool truncated);
        void AddError(const std::string& message);

    private:
        Json::Value m_root;
        Json::Value m_packagesArray;
        Json::Value m_featuresArray;
        Json::Value m_sourcesArray;
        bool m_truncated = false;
        std::vector<std::string> m_errors;
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
                        const std::string& source);

        void AddFeatureEntry(const std::string& name, const std::string& status,
                           const std::string& property, const std::string& link);

        void AddSourceEntry(const std::string& name, const std::string& type,
                          const std::string& arg, const std::string& data,
                          const std::string& updated);

        void SetTruncated(bool truncated);
        void AddError(const std::string& message);

    private:
        std::ostringstream m_output;
        bool m_truncated = false;
        std::vector<std::string> m_errors;

        static std::string EscapeXml(const std::string& str);
    };
}

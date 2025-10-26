// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <string>
#include <string_view>

namespace AppInstaller::CLI::Execution
{
    struct Context;
}

namespace AppInstaller::CLI::Workflow
{
    // Enumeration for supported output formats
    enum class OutputFormat
    {
        Text,   // Default text output
        Json,   // JSON structured output
        Xml     // XML structured output
    };

    // Parses a format string into an OutputFormat enum
    // Throws if the format is not supported
    OutputFormat ParseOutputFormat(std::string_view formatStr);

    // Validates that the format string is supported
    // Returns true if valid, false otherwise
    bool IsValidOutputFormat(std::string_view formatStr);

    // Gets the OutputFormat from the execution context
    // Returns OutputFormat::Text if no format argument is specified
    OutputFormat GetOutputFormatFromContext(const Execution::Context& context);
}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "OutputFormatter.h"
#include "ExecutionContext.h"
#include <winget/Utility.h>

namespace AppInstaller::CLI::Workflow
{
    OutputFormat ParseOutputFormat(std::string_view formatStr)
    {
        std::string lowerFormat = Utility::ToLower(formatStr);

        if (lowerFormat == "json")
        {
            return OutputFormat::Json;
        }
        else if (lowerFormat == "xml")
        {
            return OutputFormat::Xml;
        }
        else if (lowerFormat == "text")
        {
            return OutputFormat::Text;
        }
        else
        {
            throw std::invalid_argument("Unsupported output format: " + std::string(formatStr));
        }
    }

    bool IsValidOutputFormat(std::string_view formatStr)
    {
        std::string lowerFormat = Utility::ToLower(formatStr);
        return (lowerFormat == "json" || lowerFormat == "xml" || lowerFormat == "text");
    }

    OutputFormat GetOutputFormatFromContext(const Execution::Context& context)
    {
        if (context.Args.Contains(Execution::Args::Type::OutputFormat))
        {
            auto formatStr = context.Args.GetArg(Execution::Args::Type::OutputFormat);
            return ParseOutputFormat(formatStr);
        }

        return OutputFormat::Text;
    }
}

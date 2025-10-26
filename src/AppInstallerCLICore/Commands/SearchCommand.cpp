// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "SearchCommand.h"
#include "Workflows/CompletionFlow.h"
#include "Workflows/WorkflowBase.h"
#include "Resources.h"

namespace AppInstaller::CLI
{
    using namespace AppInstaller::CLI::Execution;
    using namespace AppInstaller::CLI::Workflow;
    using namespace std::string_view_literals;

    std::vector<Argument> SearchCommand::GetArguments() const
    {
        return {
            Argument::ForType(Execution::Args::Type::Query),
            Argument::ForType(Execution::Args::Type::Id),
            Argument::ForType(Execution::Args::Type::Name),
            Argument::ForType(Execution::Args::Type::Moniker),
            Argument::ForType(Execution::Args::Type::Tag),
            Argument::ForType(Execution::Args::Type::Command),
            Argument::ForType(Execution::Args::Type::Source),
            Argument::ForType(Execution::Args::Type::Count),
            Argument::ForType(Execution::Args::Type::Exact),
            Argument::ForType(Execution::Args::Type::CustomHeader),
            Argument::ForType(Execution::Args::Type::AuthenticationMode),
            Argument::ForType(Execution::Args::Type::AuthenticationAccount),
            Argument::ForType(Execution::Args::Type::AcceptSourceAgreements),
            Argument::ForType(Execution::Args::Type::ListVersions),
            Argument::ForType(Execution::Args::Type::OutputFormat),
        };
    }

    Resource::LocString SearchCommand::ShortDescription() const
    {
        return { Resource::String::SearchCommandShortDescription };
    }

    Resource::LocString SearchCommand::LongDescription() const
    {
        return { Resource::String::SearchCommandLongDescription };
    }

    void SearchCommand::Complete(Execution::Context& context, Execution::Args::Type valueType) const
    {
        switch (valueType)
        {
        case Execution::Args::Type::Query:
            context <<
            Workflow::OpenSource() <<
            Workflow::RequireCompletionWordNonEmpty <<
            Workflow::SearchSourceForManyCompletion <<
            Workflow::CompleteWithMatchedField;
            break;
        case Execution::Args::Type::Id:
        case Execution::Args::Type::Name:
        case Execution::Args::Type::Moniker:
        case Execution::Args::Type::Tag:
        case Execution::Args::Type::Command:
        case Execution::Args::Type::Source:
        case Execution::Args::Type::Count:
        case Execution::Args::Type::Exact:
        case Execution::Args::Type::CustomHeader:
        case Execution::Args::Type::AuthenticationMode:
        case Execution::Args::Type::AuthenticationAccount:
        case Execution::Args::Type::AcceptSourceAgreements:
        case Execution::Args::Type::ListVersions:
            context <<
            Workflow::CompleteWithSingleSemanticsForValue(valueType);
            break;
        case Execution::Args::Type::OutputFormat:
            // Provide tab completion for format values
            context.Reporter.Completion() << "json"_liv << std::endl;
            context.Reporter.Completion() << "table"_liv << std::endl;
            break;
        }
    }

    Utility::LocIndView SearchCommand::HelpLink() const
    {
        return "https://aka.ms/winget-command-search"_liv;
    }

    void SearchCommand::ValidateArgumentsInternal(Args& execArgs) const
    {
        Argument::ValidateCommonArguments(execArgs);

        if (execArgs.Contains(Execution::Args::Type::OutputFormat))
        {
            std::string_view formatView = execArgs.GetArg(Execution::Args::Type::OutputFormat);
            std::string format = Utility::Trim(std::string{ formatView });
            format = Utility::ToLower(format);

            if (!format.empty() && format != "json" && format != "table")
            {
                throw CommandException(Resource::String::InvalidArgumentValueError,
                    Utility::LocIndString{ "--format must be 'json' or 'table'" });
            }
        }
    }

    void SearchCommand::ExecuteInternal(Context& context) const
    {
        context.SetFlags(Execution::ContextFlag::TreatSourceFailuresAsWarning);

        context <<
        Workflow::OpenSource() <<
        Workflow::SearchSourceForMany <<
        Workflow::HandleSearchResultFailures;

        if (context.Args.Contains(Execution::Args::Type::ListVersions))
        {
            context <<
            Workflow::EnsureOneMatchFromSearchResult(OperationType::Search) <<
            Workflow::ReportPackageIdentity <<
            Workflow::ShowAppVersions;
        }
        else
        {
            context <<
            Workflow::EnsureMatchesFromSearchResult(OperationType::Search) <<
            Workflow::ReportSearchResult;
        }

    }
}

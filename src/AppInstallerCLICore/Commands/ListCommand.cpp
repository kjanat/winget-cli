// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "ListCommand.h"
#include "Workflows/CompletionFlow.h"
#include "Workflows/WorkflowBase.h"
#include "Resources.h"

namespace AppInstaller::CLI
{
    using namespace AppInstaller::CLI::Workflow;
    using namespace std::string_view_literals;

    std::vector<Argument> ListCommand::GetArguments() const
    {
        return {
            Argument::ForType(Execution::Args::Type::Query),
            Argument::ForType(Execution::Args::Type::Id),
            Argument::ForType(Execution::Args::Type::Name),
            Argument::ForType(Execution::Args::Type::Moniker),
            Argument::ForType(Execution::Args::Type::Source),
            Argument::ForType(Execution::Args::Type::Tag),
            Argument::ForType(Execution::Args::Type::Command),
            Argument::ForType(Execution::Args::Type::Count),
            Argument::ForType(Execution::Args::Type::Exact),
            Argument{ Execution::Args::Type::InstallScope, Resource::String::InstalledScopeArgumentDescription, ArgumentType::Standard, Argument::Visibility::Help },
            Argument::ForType(Execution::Args::Type::CustomHeader),
            Argument::ForType(Execution::Args::Type::AuthenticationMode),
            Argument::ForType(Execution::Args::Type::AuthenticationAccount),
            Argument::ForType(Execution::Args::Type::AcceptSourceAgreements),
            Argument{ Execution::Args::Type::Upgrade, Resource::String::UpgradeArgumentDescription, ArgumentType::Flag, Argument::Visibility::Help },
            Argument{ Execution::Args::Type::IncludeUnknown, Resource::String::IncludeUnknownInListArgumentDescription, ArgumentType::Flag },
            Argument{ Execution::Args::Type::IncludePinned, Resource::String::IncludePinnedInListArgumentDescription, ArgumentType::Flag },
            Argument::ForType(Execution::Args::Type::OutputFormat),
        };
    }

    Resource::LocString ListCommand::ShortDescription() const
    {
        return { Resource::String::ListCommandShortDescription };
    }

    Resource::LocString ListCommand::LongDescription() const
    {
        return { Resource::String::ListCommandLongDescription };
    }

    void ListCommand::Complete(Execution::Context& context, Execution::Args::Type valueType) const
    {
        switch (valueType)
        {
        case Execution::Args::Type::OutputFormat:
            // Provide tab completion for format values
            context.Reporter.Completion() << "json"_liv << std::endl;
            context.Reporter.Completion() << "table"_liv << std::endl;
            break;
        default:
            context <<
            Workflow::OpenSource() <<
            Workflow::OpenCompositeSource(Repository::PredefinedSource::Installed);

            switch (valueType)
            {
            case Execution::Args::Type::Query:
                context <<
                Workflow::RequireCompletionWordNonEmpty <<
                Workflow::SearchSourceForManyCompletion <<
                Workflow::CompleteWithMatchedField;
                break;
            case Execution::Args::Type::Id:
            case Execution::Args::Type::Name:
            case Execution::Args::Type::Moniker:
            case Execution::Args::Type::Source:
            case Execution::Args::Type::Tag:
            case Execution::Args::Type::Command:
                context <<
                Workflow::CompleteWithSingleSemanticsForValueUsingExistingSource(valueType);
                break;
            default:
                context <<
                Workflow::CompleteWithSingleSemanticsForValue(valueType);
                break;
            }
            break;
        }
    }

    Utility::LocIndView ListCommand::HelpLink() const
    {
        return "https://aka.ms/winget-command-list"_liv;
    }

    void ListCommand::ValidateArgumentsInternal(Execution::Args& execArgs) const
    {
        Argument::ValidateArgumentDependency(execArgs, Execution::Args::Type::IncludeUnknown, Execution::Args::Type::Upgrade);
        Argument::ValidateArgumentDependency(execArgs, Execution::Args::Type::IncludePinned, Execution::Args::Type::Upgrade);

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

    void ListCommand::ExecuteInternal(Execution::Context& context) const
    {
        context.SetFlags(Execution::ContextFlag::TreatSourceFailuresAsWarning);

        context <<
        Workflow::OpenSource() <<
        Workflow::OpenCompositeSource(Workflow::DetermineInstalledSource(context)) <<
        Workflow::SearchSourceForMany <<
        Workflow::HandleSearchResultFailures <<
        Workflow::EnsureMatchesFromSearchResult(OperationType::List) <<
        Workflow::ReportListResult(context.Args.Contains(Execution::Args::Type::Upgrade));
    }
}S

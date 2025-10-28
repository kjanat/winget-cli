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

    /**
     * @brief Returns the set of command-line arguments supported by the list command.
     *
     * The returned vector lists the recognized arguments and flags (including query filters,
     * source/authentication options, upgrade-related flags, and output format selection)
     * that the ListCommand accepts.
     *
     * @return std::vector<Argument> Collection of Argument entries for:
     * Query, Id, Name, Moniker, Source, Tag, Command, Count, Exact, InstallScope,
     * CustomHeader, AuthenticationMode, AuthenticationAccount, AcceptSourceAgreements,
     * Upgrade, IncludeUnknown, IncludePinned, and OutputFormat.
     */
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
            Argument{ Execution::Args::Type::IncludePinned, Resource::String::IncludePinnedInListArgumentDescription, ArgumentType::Flag},
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

    /**
     * @brief Populates shell completion actions for the list command's argument value.
     *
     * Sets up completion workflows based on which argument value is being completed.
     * For query values, requires a non-empty word and offers completions from search matches.
     * For identifier-like values (Id, Name, Moniker, Source, Tag, Command), provides single-valued
     * completions using the currently opened source. For OutputFormat, provides completion options
     * such as "json" and "xml".
     *
     * @param context Execution context used to append completion workflows.
     * @param valueType The argument value type for which completions should be produced.
     */
    void ListCommand::Complete(Execution::Context& context, Execution::Args::Type valueType) const
    {
        if (valueType != Execution::Args::Type::OutputFormat)
        {
            context <<
                Workflow::OpenSource() <<
                Workflow::OpenCompositeSource(Repository::PredefinedSource::Installed);
        }

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
        case Execution::Args::Type::OutputFormat:
            context << Workflow::CompleteOutputFormat;
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
}
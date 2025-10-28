// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "ShowCommand.h"
#include "Workflows/ShowFlow.h"
#include "Workflows/CompletionFlow.h"
#include "Workflows/WorkflowBase.h"
#include "Resources.h"

namespace AppInstaller::CLI
{
    using namespace AppInstaller::CLI::Execution;
    using namespace AppInstaller::CLI::Workflow;

    /**
     * @brief Constructs the list of command-line arguments supported by the `show` command.
     *
     * The returned list contains Argument entries for: Query, Manifest (constructed explicitly to avoid Group Policy blocking and shown in help), Id, Name, Moniker, Version, Channel, Source, Exact, InstallScope (shown in help), InstallerArchitecture, InstallerType, Locale, ListVersions, CustomHeader, AuthenticationMode, AuthenticationAccount, AcceptSourceAgreements, and OutputFormat.
     *
     * @return std::vector<Argument> Vector of Argument objects defining the `show` command's accepted arguments and their help visibility.
     */
    std::vector<Argument> ShowCommand::GetArguments() const
    {
        return {
            Argument::ForType(Execution::Args::Type::Query),
            // The manifest argument from Argument::ForType can be blocked by Group Policy but we don't want that here
            Argument{ Execution::Args::Type::Manifest, Resource::String::ManifestArgumentDescription, ArgumentType::Standard, Argument::Visibility::Help },
            Argument::ForType(Execution::Args::Type::Id),
            Argument::ForType(Execution::Args::Type::Name),
            Argument::ForType(Execution::Args::Type::Moniker),
            Argument::ForType(Execution::Args::Type::Version),
            Argument::ForType(Execution::Args::Type::Channel),
            Argument::ForType(Execution::Args::Type::Source),
            Argument::ForType(Execution::Args::Type::Exact),
            Argument{ Args::Type::InstallScope, Resource::String::InstallScopeDescription, ArgumentType::Standard, Argument::Visibility::Help },
            Argument::ForType(Execution::Args::Type::InstallerArchitecture),
            Argument::ForType(Execution::Args::Type::InstallerType),
            Argument::ForType(Execution::Args::Type::Locale),
            Argument::ForType(Execution::Args::Type::ListVersions),
            Argument::ForType(Execution::Args::Type::CustomHeader),
            Argument::ForType(Execution::Args::Type::AuthenticationMode),
            Argument::ForType(Execution::Args::Type::AuthenticationAccount),
            Argument::ForType(Execution::Args::Type::AcceptSourceAgreements),
            Argument::ForType(Execution::Args::Type::OutputFormat),
        };
    }

    Resource::LocString ShowCommand::ShortDescription() const
    {
        return { Resource::String::ShowCommandShortDescription };
    }

    Resource::LocString ShowCommand::LongDescription() const
    {
        return { Resource::String::ShowCommandLongDescription };
    }

    /**
     * @brief Provides completion suggestions for a given argument type.
     *
     * Writes completion data into the provided execution context based on the requested argument value type:
     * - For InstallerArchitecture and Locale, produces an empty completion set.
     * - For OutputFormat, produces the set of supported output formats.
     * - For all other types, delegates to the single-value completion semantics.
     *
     * @param valueType The argument type for which completion suggestions are requested.
     */
    void ShowCommand::Complete(Execution::Context& context, Execution::Args::Type valueType) const
    {
        switch (valueType)
        {
        case Args::Type::InstallerArchitecture:
        case Args::Type::Locale:
            // May well move to CompleteWithSingleSemanticsForValue,
            // but for now output nothing.
            context <<
                Workflow::CompleteWithEmptySet;
            break;
        case Execution::Args::Type::OutputFormat:
            context << Workflow::CompleteOutputFormat;
            break;
        default:
            context <<
                Workflow::CompleteWithSingleSemanticsForValue(valueType);
        }
    }

    Utility::LocIndView ShowCommand::HelpLink() const
    {
        return "https://aka.ms/winget-command-show"_liv;
    }

    void ShowCommand::ValidateArgumentsInternal(Args& execArgs) const
    {
        Argument::ValidateCommonArguments(execArgs);
    }

    void ShowCommand::ExecuteInternal(Execution::Context& context) const
    {
        context.SetFlags(Execution::ContextFlag::TreatSourceFailuresAsWarning);

        if (context.Args.Contains(Execution::Args::Type::ListVersions))
        {
            if (context.Args.Contains(Execution::Args::Type::Manifest))
            {
                context <<
                    Workflow::GetManifestFromArg <<
                    Workflow::ReportManifestIdentity <<
                    Workflow::ShowManifestVersion;
            }
            else
            {
                context <<
                    Workflow::OpenSource() <<
                    Workflow::SearchSourceForSingle <<
                    Workflow::HandleSearchResultFailures <<
                    Workflow::EnsureOneMatchFromSearchResult(OperationType::Show) <<
                    Workflow::ReportPackageIdentity <<
                    Workflow::ShowAppVersions;
            }
        }
        else
        {
            context <<
                GetManifest( /* considerPins */ false) <<
                Workflow::ReportManifestIdentity <<
                Workflow::SelectInstaller <<
                Workflow::ShowManifestInfo;
        }
    }
}
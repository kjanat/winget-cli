// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "FeaturesCommand.h"
#include "TableOutput.h"
#include "OutputFormatter.h"
#include "Workflows/CompletionFlow.h"
#include <winget/UserSettings.h>

namespace AppInstaller::CLI
{
    using namespace Utility::literals;
    using namespace AppInstaller::Settings;

    /**
     * @brief Declares the command-line arguments accepted by the Features command.
     *
     * @return std::vector<Argument> Containing a single Argument for selecting the output format (`Execution::Args::Type::OutputFormat`).
     */
    std::vector<Argument> FeaturesCommand::GetArguments() const
    {
        return {
            Argument::ForType(Execution::Args::Type::OutputFormat),
        };
    }

    /**
     * @brief Returns the localized short description for the features command.
     *
     * @return Resource::LocString Localized short description for the features command.
     */
    Resource::LocString FeaturesCommand::ShortDescription() const
    {
        return { Resource::String::FeaturesCommandShortDescription };
    }

    Resource::LocString FeaturesCommand::LongDescription() const
    {
        return { Resource::String::FeaturesCommandLongDescription };
    }

    /**
     * @brief Provides the localized help URL for the experimental features command.
     *
     * @return Utility::LocIndView Localized help URL: "https://aka.ms/winget-experimentalfeatures".
     */
    Utility::LocIndView FeaturesCommand::HelpLink() const
    {
        return "https://aka.ms/winget-experimentalfeatures"_liv;
    }

    /**
     * @brief Registers the output-format completer into the execution context when completion is requested for the output format.
     *
     * @param context Execution context that will receive the completer.
     * @param valueType The argument type for which completion is being performed; if this equals `Execution::Args::Type::OutputFormat`, the output-format completer is added to the context.
     */
    void FeaturesCommand::Complete(Execution::Context& context, Execution::Args::Type valueType) const
    {
        if (valueType == Execution::Args::Type::OutputFormat)
        {
            context << Workflow::CompleteOutputFormat;
        }
    }

    /**
     * @brief Outputs the status and metadata of experimental features to the current execution reporter.
     *
     * Depending on the configured output format, emits feature data as JSON, XML, or a human-readable table.
     * When JSON or XML is selected, produces a list of feature entries containing name, enabled state, JSON name, and link.
     * When text output is selected, emits a policy-informed header message and either a 4-column table of features
     * (Feature, Status, Property, Link) or a message indicating that no experimental features are available.
     * If experimental features are disabled at build time, reports that experimental features are disabled by build.
     *
     * @param context The execution context providing reporter, output format selection, and environment information.
     */
    void FeaturesCommand::ExecuteInternal(Execution::Context& context) const
    {
#ifdef WINGET_DISABLE_EXPERIMENTAL_FEATURES
        context.Reporter.Info() << Resource::String::FeaturesMessageDisabledByBuild << std::endl;
#else
        auto outputFormat = Execution::GetOutputFormatFromContext(context);
        auto features = ExperimentalFeature::GetAllFeatures();

        if (outputFormat == Execution::OutputFormat::Json)
        {
            Execution::JsonOutputFormatter formatter;
            formatter.StartOutput();

            for (const auto& feature : features)
            {
                formatter.AddFeatureEntry(
                    static_cast<std::string>(feature.Name()),
                    ExperimentalFeature::IsEnabled(feature.GetFeature()),
                    static_cast<std::string>(feature.JsonName()),
                    static_cast<std::string>(feature.Link())
                );
            }

            formatter.EndOutput();
            context.Reporter.Json() << formatter.GetOutput() << std::endl;
        }
        else if (outputFormat == Execution::OutputFormat::Xml)
        {
            Execution::XmlOutputFormatter formatter;
            formatter.StartOutput();

            for (const auto& feature : features)
            {
                formatter.AddFeatureEntry(
                    static_cast<std::string>(feature.Name()),
                    ExperimentalFeature::IsEnabled(feature.GetFeature()),
                    static_cast<std::string>(feature.JsonName()),
                    static_cast<std::string>(feature.Link())
                );
            }

            formatter.EndOutput();
            context.Reporter.Json() << formatter.GetOutput() << std::endl;
        }
        else
        {
            // Text format (existing implementation)
            if (GroupPolicies().IsEnabled(TogglePolicy::Policy::ExperimentalFeatures) &&
                GroupPolicies().IsEnabled(TogglePolicy::Policy::Settings))
            {
                context.Reporter.Info() << Resource::String::FeaturesMessage << std::endl << std::endl;
            }
            else
            {
                context.Reporter.Info() << Resource::String::FeaturesMessageDisabledByPolicy << std::endl << std::endl;
            }

            if (!features.empty())
            {
                Execution::TableOutput<4> table(context.Reporter, {
                    Resource::String::FeaturesFeature,
                    Resource::String::FeaturesStatus,
                    Resource::String::FeaturesProperty,
                    Resource::String::FeaturesLink });
                for (const auto& feature : features)
                {
                    table.OutputLine({
                        std::string{ feature.Name() },
                        Resource::LocString{ ExperimentalFeature::IsEnabled(feature.GetFeature()) ? Resource::String::FeaturesEnabled : Resource::String::FeaturesDisabled},
                        std::string { feature.JsonName() },
                        std::string{ feature.Link() } });
                }
                table.Complete();
            }
            else
            {
                // Better work hard to get some out there!
                context.Reporter.Info() << Resource::String::NoExperimentalFeaturesMessage << std::endl;
            }
        }
#endif
    }
}
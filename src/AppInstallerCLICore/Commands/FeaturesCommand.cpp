// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "FeaturesCommand.h"
#include "TableOutput.h"
#include "Workflows/OutputFormatter.h"
#include <winget/UserSettings.h>
#include <json/json.h>

namespace AppInstaller::CLI
{
    using namespace Utility::literals;
    using namespace AppInstaller::Settings;

    std::vector<Argument> FeaturesCommand::GetArguments() const
    {
        return {
            Argument::ForType(Execution::Args::Type::OutputFormat),
        };
    }

    Resource::LocString FeaturesCommand::ShortDescription() const
    {
        return { Resource::String::FeaturesCommandShortDescription };
    }

    Resource::LocString FeaturesCommand::LongDescription() const
    {
        return { Resource::String::FeaturesCommandLongDescription };
    }

    Utility::LocIndView FeaturesCommand::HelpLink() const
    {
        return "https://aka.ms/winget-experimentalfeatures"_liv;
    }

    void FeaturesCommand::ExecuteInternal(Execution::Context& context) const
    {
#ifdef WINGET_DISABLE_EXPERIMENTAL_FEATURES
        context.Reporter.Info() << Resource::String::FeaturesMessageDisabledByBuild << std::endl;
#else
        auto features = ExperimentalFeature::GetAllFeatures();
        Workflow::OutputFormat format = Workflow::GetOutputFormatFromContext(context);

        if (format == Workflow::OutputFormat::Json)
        {
            // JSON output
            Json::Value root;
            Json::Value featuresArray(Json::arrayValue);

            for (const auto& feature : features)
            {
                Json::Value featureObj;
                featureObj["name"] = std::string{ feature.Name() };
                featureObj["status"] = ExperimentalFeature::IsEnabled(feature.GetFeature()) ? "enabled" : "disabled";
                featureObj["property"] = std::string{ feature.JsonName() };
                featureObj["link"] = std::string{ feature.Link() };
                featuresArray.append(featureObj);
            }

            root["features"] = featuresArray;

            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            std::string output = Json::writeString(builder, root);
            context.Reporter.Info() << output << std::endl;
        }
        else if (format == Workflow::OutputFormat::Xml)
        {
            // XML output
            context.Reporter.Info() << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
            context.Reporter.Info() << "<features>" << std::endl;

            for (const auto& feature : features)
            {
                context.Reporter.Info() << "  <feature>" << std::endl;
                context.Reporter.Info() << "    <name>" << std::string{ feature.Name() } << "</name>" << std::endl;
                context.Reporter.Info() << "    <status>" << (ExperimentalFeature::IsEnabled(feature.GetFeature()) ? "enabled" : "disabled") << "</status>" << std::endl;
                context.Reporter.Info() << "    <property>" << std::string{ feature.JsonName() } << "</property>" << std::endl;
                context.Reporter.Info() << "    <link>" << std::string{ feature.Link() } << "</link>" << std::endl;
                context.Reporter.Info() << "  </feature>" << std::endl;
            }

            context.Reporter.Info() << "</features>" << std::endl;
        }
        else
        {
            // Default text output
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

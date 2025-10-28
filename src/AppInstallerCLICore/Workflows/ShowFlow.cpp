// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "ShowFlow.h"
#include <winget/ManifestComparator.h>
#include "TableOutput.h"
#include "OutputFormatter.h"

using namespace AppInstaller::Repository;
using namespace AppInstaller::CLI;
using namespace AppInstaller::Utility;
using namespace AppInstaller::Utility::literals;

namespace {

    template <typename String>
    void ShowSingleLineField(Execution::OutputStream outputStream, AppInstaller::StringResource::StringId label, const String& value, bool indent = false)
    {
        if (value.empty())
        {
            return;
        }
        if (indent)
        {
            outputStream << "  "_liv;
        }
        outputStream << Execution::ManifestInfoEmphasis << label << ' ' << value << std::endl;
    }

    template <typename String>
    void ShowMultiLineField(Execution::OutputStream outputStream, AppInstaller::StringResource::StringId label, const String& value)
    {
        if (value.empty())
        {
            return;
        }
        /*
            We need to be able to find and replace within the string.However, we don't want to own the original string
            Therefore, a copy is created here so we can manipulate it. The memory should be freed again once this method
            returns and the string is no longer in scope.
        */
        std::string shownValue = value;
        bool isMultiLine = FindAndReplace(shownValue, "\n", "\n  ");
        outputStream << Execution::ManifestInfoEmphasis << label;
        if (isMultiLine)
        {
            outputStream << std::endl << "  "_liv << shownValue << std::endl;
        }
        else
        {
            outputStream << ' ' << shownValue << std::endl;
        }
    }

    template <typename Enumerable>
    void ShowMultiValueField(Execution::OutputStream outputStream, AppInstaller::StringResource::StringId label, const Enumerable& values)
    {
        if (values.empty())
        {
            return;
        }
        outputStream << Execution::ManifestInfoEmphasis << label << std::endl;
        for (const auto& value : values)
        {
            outputStream << "  "_liv << value << std::endl;
        }
    }

    void ShowAgreements(Execution::OutputStream outputStream, const std::vector<AppInstaller::Manifest::Agreement>& agreements) {

        if (agreements.empty()) {
            return;
        }

        outputStream << Execution::ManifestInfoEmphasis << Resource::String::ShowLabelAgreements << std::endl;
        for (const auto& agreement : agreements) {

            if (!agreement.Label.empty())
            {
                outputStream << "  "_liv << Execution::ManifestInfoEmphasis << agreement.Label << ": "_liv;
            }

            if (!agreement.AgreementText.empty())
            {
                outputStream << agreement.AgreementText << std::endl;
            }

            if (!agreement.AgreementUrl.empty())
            {
                outputStream << agreement.AgreementUrl << std::endl;
            }
        }
    }
}

namespace AppInstaller::CLI::Workflow
{
    /**
     * @brief Displays agreement-related metadata from the active manifest to the output reporter.
     *
     * Outputs manifest fields such as version, publisher, publisher/support URLs, author,
     * package and license information, privacy and copyright information, purchase URL,
     * and the list of agreements to the context's reporter.
     *
     * @param context Execution context that provides the manifest data and the reporter to write to.
     */
    void ShowAgreementsInfo(Execution::Context& context)
    {
        const auto& manifest = context.Get<Execution::Data::Manifest>();
        auto info = context.Reporter.Info();

        ShowSingleLineField(info, Resource::String::ShowLabelVersion, manifest.Version);
        ShowSingleLineField(info, Resource::String::ShowLabelPublisher, manifest.CurrentLocalization.Get<Manifest::Localization::Publisher>());
        ShowSingleLineField(info, Resource::String::ShowLabelPublisherUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PublisherUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelPublisherSupportUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PublisherSupportUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelAuthor, manifest.CurrentLocalization.Get<Manifest::Localization::Author>());
        ShowSingleLineField(info, Resource::String::ShowLabelPackageUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PackageUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelLicense, manifest.CurrentLocalization.Get<Manifest::Localization::License>());
        ShowSingleLineField(info, Resource::String::ShowLabelLicenseUrl, manifest.CurrentLocalization.Get<Manifest::Localization::LicenseUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelPrivacyUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PrivacyUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelCopyright, manifest.CurrentLocalization.Get<Manifest::Localization::Copyright>());
        ShowSingleLineField(info, Resource::String::ShowLabelCopyrightUrl, manifest.CurrentLocalization.Get<Manifest::Localization::CopyrightUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelPurchaseUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PurchaseUrl>());
        ShowAgreements(info, manifest.CurrentLocalization.Get<Manifest::Localization::Agreements>());
        
    }

    /**
     * @brief Emit the manifest and its installer information to the execution reporter in the requested structured format.
     *
     * Serializes package-level fields (id, name, version, publisher, optional description, packageUrl, license,
     * moniker, and tags) and, when an installer is present, installer fields (type, locale, url, sha256,
     * productId, releaseDate, and offlineDistributionSupported) into either JSON or XML and writes the result
     * to the context's reporter.
     *
     * @param context Execution context that provides the manifest, installer, and reporter used for output.
     * @param format The structured output format to produce (JSON or XML).
     */
    void OutputManifestAsStructured(Execution::Context& context, Execution::OutputFormat format)
    {
        const auto& manifest = context.Get<Execution::Data::Manifest>();
        const auto& installer = context.Get<Execution::Data::Installer>();

        if (format == Execution::OutputFormat::Json)
        {
            Json::Value root(Json::objectValue);

            // Package info
            root["id"] = static_cast<std::string>(manifest.Id);
            root["name"] = static_cast<std::string>(manifest.CurrentLocalization.Get<Manifest::Localization::PackageName>());
            root["version"] = manifest.Version;
            root["publisher"] = static_cast<std::string>(manifest.CurrentLocalization.Get<Manifest::Localization::Publisher>());

            auto description = manifest.CurrentLocalization.Get<Manifest::Localization::Description>();
            if (!description.empty())
            {
                root["description"] = static_cast<std::string>(description);
            }

            auto packageUrl = manifest.CurrentLocalization.Get<Manifest::Localization::PackageUrl>();
            if (!packageUrl.empty())
            {
                root["packageUrl"] = static_cast<std::string>(packageUrl);
            }

            auto license = manifest.CurrentLocalization.Get<Manifest::Localization::License>();
            if (!license.empty())
            {
                root["license"] = static_cast<std::string>(license);
            }

            auto moniker = manifest.Moniker;
            if (!moniker.empty())
            {
                root["moniker"] = static_cast<std::string>(moniker);
            }

            const auto& tags = manifest.CurrentLocalization.Get<Manifest::Localization::Tags>();
            if (!tags.empty())
            {
                Json::Value tagsArray(Json::arrayValue);
                for (const auto& tag : tags)
                {
                    tagsArray.append(static_cast<std::string>(tag));
                }
                root["tags"] = tagsArray;
            }

            // Installer info
            if (installer)
            {
                Json::Value installerObj(Json::objectValue);
                auto typeStr = Manifest::InstallerTypeToString(installer->EffectiveInstallerType());
                installerObj["type"] = std::string(typeStr.data(), typeStr.size());

                if (!installer->Locale.empty())
                {
                    installerObj["locale"] = installer->Locale;
                }

                if (!installer->Url.empty())
                {
                    installerObj["url"] = installer->Url;
                }

                if (!installer->Sha256.empty())
                {
                    installerObj["sha256"] = Utility::SHA256::ConvertToString(installer->Sha256);
                }

                if (!installer->ProductId.empty())
                {
                    installerObj["productId"] = installer->ProductId;
                }

                if (!installer->ReleaseDate.empty())
                {
                    installerObj["releaseDate"] = installer->ReleaseDate;
                }

                installerObj["offlineDistributionSupported"] = !installer->DownloadCommandProhibited;

                root["installer"] = installerObj;
            }

            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            context.Reporter.Json() << Json::writeString(builder, root) << std::endl;
        }
        else if (format == Execution::OutputFormat::Xml)
        {
            std::ostringstream output;
            output << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<package>\n";

            output << "  <id>" << Execution::XmlOutputFormatter::EscapeXml(static_cast<std::string>(manifest.Id)) << "</id>\n";
            output << "  <name>" << Execution::XmlOutputFormatter::EscapeXml(static_cast<std::string>(manifest.CurrentLocalization.Get<Manifest::Localization::PackageName>())) << "</name>\n";
            output << "  <version>" << Execution::XmlOutputFormatter::EscapeXml(manifest.Version) << "</version>\n";
            output << "  <publisher>" << Execution::XmlOutputFormatter::EscapeXml(static_cast<std::string>(manifest.CurrentLocalization.Get<Manifest::Localization::Publisher>())) << "</publisher>\n";

            auto description = manifest.CurrentLocalization.Get<Manifest::Localization::Description>();
            if (!description.empty())
            {
                output << "  <description>" << Execution::XmlOutputFormatter::EscapeXml(static_cast<std::string>(description)) << "</description>\n";
            }

            auto packageUrl = manifest.CurrentLocalization.Get<Manifest::Localization::PackageUrl>();
            if (!packageUrl.empty())
            {
                output << "  <packageUrl>" << Execution::XmlOutputFormatter::EscapeXml(static_cast<std::string>(packageUrl)) << "</packageUrl>\n";
            }

            auto license = manifest.CurrentLocalization.Get<Manifest::Localization::License>();
            if (!license.empty())
            {
                output << "  <license>" << Execution::XmlOutputFormatter::EscapeXml(static_cast<std::string>(license)) << "</license>\n";
            }

            auto moniker = manifest.Moniker;
            if (!moniker.empty())
            {
                output << "  <moniker>" << Execution::XmlOutputFormatter::EscapeXml(static_cast<std::string>(moniker)) << "</moniker>\n";
            }

            const auto& tags = manifest.CurrentLocalization.Get<Manifest::Localization::Tags>();
            if (!tags.empty())
            {
                output << "  <tags>\n";
                for (const auto& tag : tags)
                {
                    output << "    <tag>" << Execution::XmlOutputFormatter::EscapeXml(static_cast<std::string>(tag)) << "</tag>\n";
                }
                output << "  </tags>\n";
            }

            if (installer)
            {
                output << "  <installer>\n";
                auto typeStr = Manifest::InstallerTypeToString(installer->EffectiveInstallerType());
                output << "    <type>" << Execution::XmlOutputFormatter::EscapeXml(std::string(typeStr.data(), typeStr.size())) << "</type>\n";
                if (!installer->Locale.empty())
                {
                    output << "    <locale>" << Execution::XmlOutputFormatter::EscapeXml(installer->Locale) << "</locale>\n";
                }
                if (!installer->Url.empty())
                {
                    output << "    <url>" << Execution::XmlOutputFormatter::EscapeXml(installer->Url) << "</url>\n";
                }
                if (!installer->Sha256.empty())
                {
                    output << "    <sha256>" << Execution::XmlOutputFormatter::EscapeXml(Utility::SHA256::ConvertToString(installer->Sha256)) << "</sha256>\n";
                }
                if (!installer->ProductId.empty())
                {
                    output << "    <productId>" << Execution::XmlOutputFormatter::EscapeXml(installer->ProductId) << "</productId>\n";
                }
                if (!installer->ReleaseDate.empty())
                {
                    output << "    <releaseDate>" << Execution::XmlOutputFormatter::EscapeXml(installer->ReleaseDate) << "</releaseDate>\n";
                }
                output << "    <offlineDistributionSupported>" << (installer->DownloadCommandProhibited ? "false" : "true") << "</offlineDistributionSupported>\n";
                output << "  </installer>\n";
            }

            output << "</package>\n";
            context.Reporter.Json() << output.str();
        }
    }

    /**
     * @brief Displays manifest information using the context's selected output format.
     *
     * Chooses between structured output (JSON or XML) and plain text output and emits
     * the manifest and installer information through the provided execution context.
     *
     * @param context Execution context used to determine output format and to emit output.
     */
    void ShowManifestInfo(Execution::Context& context)
    {
        auto outputFormat = Execution::GetOutputFormatFromContext(context);

        if (outputFormat == Execution::OutputFormat::Json || outputFormat == Execution::OutputFormat::Xml)
        {
            OutputManifestAsStructured(context, outputFormat);
        }
        else
        {
            context << ShowPackageInfo << ShowInstallerInfo;
        }
    }

    /**
     * @brief Renders package-level metadata from the active manifest to the reporter.
     *
     * Outputs human-readable package information such as version, publisher, author, moniker,
     * description (prefers full description and falls back to short description), package URL,
     * license and license URL, privacy URL, copyright and copyright URL, release notes and
     * release notes URL, purchase URL, installation notes, documentation entries (label and URL),
     * tags, and agreements.
     */
    void ShowPackageInfo(Execution::Context& context)
    {
        const auto& manifest = context.Get<Execution::Data::Manifest>();
        auto info = context.Reporter.Info();
        // Get description from manifest so we can see if it is empty later
        auto description = manifest.CurrentLocalization.Get<Manifest::Localization::Description>();

        // TODO: Come up with a prettier format
        ShowSingleLineField(info, Resource::String::ShowLabelVersion, manifest.Version);
        ShowSingleLineField(info, Resource::String::ShowLabelPublisher, manifest.CurrentLocalization.Get<Manifest::Localization::Publisher>());
        ShowSingleLineField(info, Resource::String::ShowLabelPublisherUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PublisherUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelPublisherSupportUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PublisherSupportUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelAuthor, manifest.CurrentLocalization.Get<Manifest::Localization::Author>());
        ShowSingleLineField(info, Resource::String::ShowLabelMoniker, manifest.Moniker);
        ShowMultiLineField(info,  Resource::String::ShowLabelDescription, description.empty() ? manifest.CurrentLocalization.Get<Manifest::Localization::ShortDescription>() : description);
        ShowSingleLineField(info, Resource::String::ShowLabelPackageUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PackageUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelLicense, manifest.CurrentLocalization.Get<Manifest::Localization::License>());
        ShowSingleLineField(info, Resource::String::ShowLabelLicenseUrl, manifest.CurrentLocalization.Get<Manifest::Localization::LicenseUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelPrivacyUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PrivacyUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelCopyright, manifest.CurrentLocalization.Get<Manifest::Localization::Copyright>());
        ShowSingleLineField(info, Resource::String::ShowLabelCopyrightUrl, manifest.CurrentLocalization.Get<Manifest::Localization::CopyrightUrl>());
        ShowMultiLineField(info,  Resource::String::ShowLabelReleaseNotes, manifest.CurrentLocalization.Get<Manifest::Localization::ReleaseNotes>());
        ShowSingleLineField(info, Resource::String::ShowLabelReleaseNotesUrl, manifest.CurrentLocalization.Get<Manifest::Localization::ReleaseNotesUrl>());
        ShowSingleLineField(info, Resource::String::ShowLabelPurchaseUrl, manifest.CurrentLocalization.Get<Manifest::Localization::PurchaseUrl>());
        ShowMultiLineField(info, Resource::String::ShowLabelInstallationNotes, manifest.CurrentLocalization.Get<Manifest::Localization::InstallationNotes>());
        const auto& documentations = manifest.CurrentLocalization.Get<Manifest::Localization::Documentations>();
        if (!documentations.empty())
        {
            context.Reporter.Info() << Execution::ManifestInfoEmphasis << Resource::String::ShowLabelDocumentation << std::endl;
            for (const auto& documentation : documentations)
            {
                if (!documentation.DocumentUrl.empty())
                {
                    info << "  "_liv;
                    if (!documentation.DocumentLabel.empty())
                    {
                        info << Execution::ManifestInfoEmphasis << documentation.DocumentLabel << ": "_liv;
                    }

                    info << documentation.DocumentUrl << std::endl;
                }
            }
        }
        ShowMultiValueField(info, Resource::String::ShowLabelTags, manifest.CurrentLocalization.Get<Manifest::Localization::Tags>());
        ShowAgreements(info, manifest.CurrentLocalization.Get<Manifest::Localization::Agreements>());
    }

    /**
     * @brief Displays installer details from the execution context to the reporter.
     *
     * Outputs an "Installer" header and, if an installer is present, prints labeled fields for:
     * installer type (appending the base installer type in parentheses when different), locale,
     * URL, SHA256 (hex string when available), product ID, release date, and whether offline distribution
     * is supported. If the installer declares dependencies, they are listed grouped by type (Windows Features,
     * Windows Libraries, Packages, External); package dependencies include a minimum-version annotation shown as
     * "[>= <version>]" when provided. If no applicable installer is found, emits a warning with the
     * NoApplicableInstallers message.
     */
    void ShowInstallerInfo(Execution::Context& context)
    {
        const auto& installer = context.Get<Execution::Data::Installer>();
        auto info = context.Reporter.Info();

        info << Execution::ManifestInfoEmphasis << Resource::String::ShowLabelInstaller << std::endl;
        if (installer)
        {
            Manifest::InstallerTypeEnum effectiveInstallerType = installer->EffectiveInstallerType();
            Manifest::InstallerTypeEnum baseInstallerType = installer->BaseInstallerType;
            std::string shownInstallerType;
            auto typeStr = Manifest::InstallerTypeToString(effectiveInstallerType);
            shownInstallerType = std::string(typeStr.data(), typeStr.size());
            if (effectiveInstallerType != baseInstallerType)
            {
                shownInstallerType += " ("_liv;
                auto baseTypeStr = Manifest::InstallerTypeToString(baseInstallerType);
                shownInstallerType += std::string(baseTypeStr.data(), baseTypeStr.size());
                shownInstallerType += ')';
            }
            ShowSingleLineField(info, Resource::String::ShowLabelInstallerType, shownInstallerType, true);
            ShowSingleLineField(info, Resource::String::ShowLabelInstallerLocale, installer->Locale, true);
            ShowSingleLineField(info, Resource::String::ShowLabelInstallerUrl, installer->Url, true);
            ShowSingleLineField(info, Resource::String::ShowLabelInstallerSha256, (installer->Sha256.empty()) ? "" : Utility::SHA256::ConvertToString(installer->Sha256), true);
            ShowSingleLineField(info, Resource::String::ShowLabelInstallerProductId, installer->ProductId, true);
            ShowSingleLineField(info, Resource::String::ShowLabelInstallerReleaseDate, installer->ReleaseDate, true);
            ShowSingleLineField(info, Resource::String::ShowLabelInstallerOfflineDistributionSupported, Utility::ConvertBoolToString(!installer->DownloadCommandProhibited), true);

            const auto& dependencies = installer->Dependencies;

            if (dependencies.HasAny())
            {
                info << Execution::ManifestInfoEmphasis << "  "_liv << Resource::String::ShowLabelDependencies << ' ' << std::endl;

                if (dependencies.HasAnyOf(Manifest::DependencyType::WindowsFeature))
                {
                    info << "    - "_liv << Resource::String::ShowLabelWindowsFeaturesDependencies << ' ' << std::endl;
                    dependencies.ApplyToType(Manifest::DependencyType::WindowsFeature, [&info](Manifest::Dependency dependency) {info << "        "_liv << dependency.Id() << std::endl; });
                }

                if (dependencies.HasAnyOf(Manifest::DependencyType::WindowsLibrary))
                {
                    info << "    - "_liv << Resource::String::ShowLabelWindowsLibrariesDependencies << ' ' << std::endl;
                    dependencies.ApplyToType(Manifest::DependencyType::WindowsLibrary, [&info](Manifest::Dependency dependency) {info << "        "_liv << dependency.Id() << std::endl; });
                }

                if (dependencies.HasAnyOf(Manifest::DependencyType::Package))
                {
                    info << "    - "_liv << Resource::String::ShowLabelPackageDependencies << ' ' << std::endl;
                    dependencies.ApplyToType(Manifest::DependencyType::Package, [&info](Manifest::Dependency dependency)
                        {
                            info << "        "_liv << dependency.Id();
                            if (dependency.MinVersion)
                            {
                                info << " [>= " << dependency.MinVersion.value().ToString() << "]";
                            }
                            info << std::endl;
                        });
                }

                if (dependencies.HasAnyOf(Manifest::DependencyType::External))
                {
                    info << "    - "_liv << Resource::String::ShowLabelExternalDependencies << ' ' << std::endl;
                    dependencies.ApplyToType(Manifest::DependencyType::External, [&info](Manifest::Dependency dependency) {info << "        "_liv << dependency.Id() << std::endl; });
                }
            }
        }
        else
        {
            context.Reporter.Warn() << "  "_liv << Resource::String::NoApplicableInstallers << std::endl;
        }
    }

    void ShowManifestVersion(Execution::Context& context)
    {
        const auto& manifest = context.Get<Execution::Data::Manifest>();
        Execution::TableOutput<2> table(context.Reporter, { Resource::String::ShowVersion, Resource::String::ShowChannel });
        table.OutputLine({ manifest.Version, manifest.Channel });
        table.Complete();
    }

    void GetManifest::operator()(Execution::Context& context) const
    {
        if (context.Args.Contains(Execution::Args::Type::Manifest))
        {
            context <<
                GetManifestFromArg;
        }
        else
        {
            context <<
                OpenSource() <<
                SearchSourceForSingle <<
                HandleSearchResultFailures <<
                EnsureOneMatchFromSearchResult(OperationType::Show) <<
                GetManifestFromPackage(m_considerPins);
        }
    }
}

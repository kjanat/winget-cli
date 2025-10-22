// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "ShowFlow.h"
#include <winget/ManifestComparator.h>
#include "TableOutput.h"
#include <json/json.h>
#include <filesystem>

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

    void ShowManifestInfo(Execution::Context& context)
    {
        // Check if JSON output is requested
        if (context.Args.Contains(Execution::Args::Type::OutputFile))
        {
            const auto& manifest = context.Get<Execution::Data::Manifest>();
            const auto& installer = context.Get<Execution::Data::Installer>();
            
            Json::Value root(Json::objectValue);
            
            // Add package info
            root["Version"] = manifest.Version;
            
            auto description = manifest.CurrentLocalization.Get<Manifest::Localization::Description>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::Publisher>().empty())
                root["Publisher"] = manifest.CurrentLocalization.Get<Manifest::Localization::Publisher>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::PublisherUrl>().empty())
                root["PublisherUrl"] = manifest.CurrentLocalization.Get<Manifest::Localization::PublisherUrl>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::PublisherSupportUrl>().empty())
                root["PublisherSupportUrl"] = manifest.CurrentLocalization.Get<Manifest::Localization::PublisherSupportUrl>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::Author>().empty())
                root["Author"] = manifest.CurrentLocalization.Get<Manifest::Localization::Author>();
            
            if (!manifest.Moniker.empty())
                root["Moniker"] = manifest.Moniker;
            
            if (!description.empty() || !manifest.CurrentLocalization.Get<Manifest::Localization::ShortDescription>().empty())
                root["Description"] = description.empty() ? manifest.CurrentLocalization.Get<Manifest::Localization::ShortDescription>() : description;
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::PackageUrl>().empty())
                root["PackageUrl"] = manifest.CurrentLocalization.Get<Manifest::Localization::PackageUrl>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::License>().empty())
                root["License"] = manifest.CurrentLocalization.Get<Manifest::Localization::License>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::LicenseUrl>().empty())
                root["LicenseUrl"] = manifest.CurrentLocalization.Get<Manifest::Localization::LicenseUrl>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::PrivacyUrl>().empty())
                root["PrivacyUrl"] = manifest.CurrentLocalization.Get<Manifest::Localization::PrivacyUrl>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::Copyright>().empty())
                root["Copyright"] = manifest.CurrentLocalization.Get<Manifest::Localization::Copyright>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::CopyrightUrl>().empty())
                root["CopyrightUrl"] = manifest.CurrentLocalization.Get<Manifest::Localization::CopyrightUrl>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::ReleaseNotes>().empty())
                root["ReleaseNotes"] = manifest.CurrentLocalization.Get<Manifest::Localization::ReleaseNotes>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::ReleaseNotesUrl>().empty())
                root["ReleaseNotesUrl"] = manifest.CurrentLocalization.Get<Manifest::Localization::ReleaseNotesUrl>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::PurchaseUrl>().empty())
                root["PurchaseUrl"] = manifest.CurrentLocalization.Get<Manifest::Localization::PurchaseUrl>();
            
            if (!manifest.CurrentLocalization.Get<Manifest::Localization::InstallationNotes>().empty())
                root["InstallationNotes"] = manifest.CurrentLocalization.Get<Manifest::Localization::InstallationNotes>();
            
            const auto& documentations = manifest.CurrentLocalization.Get<Manifest::Localization::Documentations>();
            if (!documentations.empty())
            {
                Json::Value docsArray(Json::arrayValue);
                for (const auto& documentation : documentations)
                {
                    if (!documentation.DocumentUrl.empty())
                    {
                        Json::Value doc(Json::objectValue);
                        if (!documentation.DocumentLabel.empty())
                            doc["Label"] = documentation.DocumentLabel;
                        doc["Url"] = documentation.DocumentUrl;
                        docsArray.append(doc);
                    }
                }
                if (!docsArray.empty())
                    root["Documentation"] = docsArray;
            }
            
            const auto& tags = manifest.CurrentLocalization.Get<Manifest::Localization::Tags>();
            if (!tags.empty())
            {
                Json::Value tagsArray(Json::arrayValue);
                for (const auto& tag : tags)
                {
                    tagsArray.append(tag);
                }
                root["Tags"] = tagsArray;
            }
            
            const auto& agreements = manifest.CurrentLocalization.Get<Manifest::Localization::Agreements>();
            if (!agreements.empty())
            {
                Json::Value agreementsArray(Json::arrayValue);
                for (const auto& agreement : agreements)
                {
                    Json::Value agr(Json::objectValue);
                    if (!agreement.Label.empty())
                        agr["Label"] = agreement.Label;
                    if (!agreement.AgreementText.empty())
                        agr["AgreementText"] = agreement.AgreementText;
                    if (!agreement.AgreementUrl.empty())
                        agr["AgreementUrl"] = agreement.AgreementUrl;
                    agreementsArray.append(agr);
                }
                root["Agreements"] = agreementsArray;
            }
            
            // Add installer info
            if (installer)
            {
                Json::Value installerInfo(Json::objectValue);
                
                Manifest::InstallerTypeEnum effectiveInstallerType = installer->EffectiveInstallerType();
                Manifest::InstallerTypeEnum baseInstallerType = installer->BaseInstallerType;
                std::string shownInstallerType = Manifest::InstallerTypeToString(effectiveInstallerType);
                if (effectiveInstallerType != baseInstallerType)
                {
                    shownInstallerType += " ("_liv;
                    shownInstallerType += Manifest::InstallerTypeToString(baseInstallerType);
                    shownInstallerType += ')';
                }
                
                installerInfo["Type"] = shownInstallerType;
                
                if (!installer->Locale.empty())
                    installerInfo["Locale"] = installer->Locale;
                
                if (!installer->Url.empty())
                    installerInfo["Url"] = installer->Url;
                
                if (!installer->Sha256.empty())
                    installerInfo["Sha256"] = Utility::SHA256::ConvertToString(installer->Sha256);
                
                if (!installer->ProductId.empty())
                    installerInfo["ProductId"] = installer->ProductId;
                
                if (!installer->ReleaseDate.empty())
                    installerInfo["ReleaseDate"] = installer->ReleaseDate;
                
                installerInfo["OfflineDistributionSupported"] = !installer->DownloadCommandProhibited;
                
                const auto& dependencies = installer->Dependencies;
                if (dependencies.HasAny())
                {
                    Json::Value depsInfo(Json::objectValue);
                    
                    if (dependencies.HasAnyOf(Manifest::DependencyType::WindowsFeature))
                    {
                        Json::Value winFeatures(Json::arrayValue);
                        dependencies.ApplyToType(Manifest::DependencyType::WindowsFeature, [&winFeatures](Manifest::Dependency dependency) {
                            winFeatures.append(dependency.Id());
                        });
                        depsInfo["WindowsFeatures"] = winFeatures;
                    }
                    
                    if (dependencies.HasAnyOf(Manifest::DependencyType::WindowsLibrary))
                    {
                        Json::Value winLibraries(Json::arrayValue);
                        dependencies.ApplyToType(Manifest::DependencyType::WindowsLibrary, [&winLibraries](Manifest::Dependency dependency) {
                            winLibraries.append(dependency.Id());
                        });
                        depsInfo["WindowsLibraries"] = winLibraries;
                    }
                    
                    if (dependencies.HasAnyOf(Manifest::DependencyType::Package))
                    {
                        Json::Value packages(Json::arrayValue);
                        dependencies.ApplyToType(Manifest::DependencyType::Package, [&packages](Manifest::Dependency dependency) {
                            Json::Value pkg(Json::objectValue);
                            pkg["Id"] = dependency.Id();
                            if (dependency.MinVersion)
                            {
                                pkg["MinVersion"] = dependency.MinVersion.value().ToString();
                            }
                            packages.append(pkg);
                        });
                        depsInfo["Packages"] = packages;
                    }
                    
                    if (dependencies.HasAnyOf(Manifest::DependencyType::External))
                    {
                        Json::Value external(Json::arrayValue);
                        dependencies.ApplyToType(Manifest::DependencyType::External, [&external](Manifest::Dependency dependency) {
                            external.append(dependency.Id());
                        });
                        depsInfo["External"] = external;
                    }
                    
                    installerInfo["Dependencies"] = depsInfo;
                }
                
                root["Installer"] = installerInfo;
            }
            
            std::filesystem::path outputFilePath{ context.Args.GetArg(Execution::Args::Type::OutputFile) };
            std::ofstream outputFileStream{ outputFilePath };
            outputFileStream << root;
        }
        else
        {
            // Original output
            context << ShowPackageInfo << ShowInstallerInfo;
        }
    }

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
            shownInstallerType = Manifest::InstallerTypeToString(effectiveInstallerType);
            if (effectiveInstallerType != baseInstallerType)
            {
                shownInstallerType += " ("_liv;
                shownInstallerType += Manifest::InstallerTypeToString(baseInstallerType);
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

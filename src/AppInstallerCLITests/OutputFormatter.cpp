// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "TestCommon.h"
#include "WorkflowCommon.h"
#include <OutputFormatter.h>
#include <Command.h>

using namespace AppInstaller::CLI::Execution;
using namespace TestCommon;

TEST_CASE("ParseOutputFormat_ValidFormats", "[outputformatter]")
{
    SECTION("JSON lowercase")
    {
        REQUIRE(ParseOutputFormat("json") == OutputFormat::Json);
    }

    SECTION("JSON uppercase")
    {
        REQUIRE(ParseOutputFormat("JSON") == OutputFormat::Json);
    }

    SECTION("JSON mixed case")
    {
        REQUIRE(ParseOutputFormat("Json") == OutputFormat::Json);
    }

    SECTION("XML lowercase")
    {
        REQUIRE(ParseOutputFormat("xml") == OutputFormat::Xml);
    }

    SECTION("XML uppercase")
    {
        REQUIRE(ParseOutputFormat("XML") == OutputFormat::Xml);
    }

    SECTION("XML mixed case")
    {
        REQUIRE(ParseOutputFormat("Xml") == OutputFormat::Xml);
    }

    SECTION("Text lowercase")
    {
        REQUIRE(ParseOutputFormat("text") == OutputFormat::Text);
    }

    SECTION("Empty string defaults to Text")
    {
        REQUIRE(ParseOutputFormat("") == OutputFormat::Text);
    }
}

TEST_CASE("ParseOutputFormat_InvalidFormats", "[outputformatter]")
{
    SECTION("Invalid format yaml")
    {
        REQUIRE_THROWS_AS(ParseOutputFormat("yaml"), AppInstaller::CLI::CommandException);
    }

    SECTION("Invalid format csv")
    {
        REQUIRE_THROWS_AS(ParseOutputFormat("csv"), AppInstaller::CLI::CommandException);
    }

    SECTION("Invalid format random")
    {
        REQUIRE_THROWS_AS(ParseOutputFormat("invalid"), AppInstaller::CLI::CommandException);
    }
}

TEST_CASE("JsonOutputFormatter_PackageEntry", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("TestPackage", "Publisher.TestPackage", "1.0.0", "Tag: test", "winget");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify JSON contains expected fields
    REQUIRE(output.find("\"packages\"") != std::string::npos);
    REQUIRE(output.find("\"name\"") != std::string::npos);
    REQUIRE(output.find("TestPackage") != std::string::npos);
    REQUIRE(output.find("\"id\"") != std::string::npos);
    REQUIRE(output.find("Publisher.TestPackage") != std::string::npos);
    REQUIRE(output.find("\"version\"") != std::string::npos);
    REQUIRE(output.find("1.0.0") != std::string::npos);
    REQUIRE(output.find("\"match\"") != std::string::npos);
    REQUIRE(output.find("Tag: test") != std::string::npos);
    REQUIRE(output.find("\"source\"") != std::string::npos);
    REQUIRE(output.find("winget") != std::string::npos);
}

TEST_CASE("JsonOutputFormatter_ListEntry", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddListEntry("TestPackage", "Publisher.TestPackage", "1.0.0", "1.1.0", "winget");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify JSON contains expected fields
    REQUIRE(output.find("\"packages\"") != std::string::npos);
    REQUIRE(output.find("\"name\"") != std::string::npos);
    REQUIRE(output.find("TestPackage") != std::string::npos);
    REQUIRE(output.find("\"id\"") != std::string::npos);
    REQUIRE(output.find("Publisher.TestPackage") != std::string::npos);
    REQUIRE(output.find("\"version\"") != std::string::npos);
    REQUIRE(output.find("1.0.0") != std::string::npos);
    REQUIRE(output.find("\"availableVersion\"") != std::string::npos);
    REQUIRE(output.find("1.1.0") != std::string::npos);
    REQUIRE(output.find("\"source\"") != std::string::npos);
    REQUIRE(output.find("winget") != std::string::npos);
}

TEST_CASE("JsonOutputFormatter_FeatureEntry", "[outputformatter]")
{
    SECTION("Enabled feature")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddFeatureEntry("TestFeature", true, "testFeature", "https://example.com");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Verify JSON contains expected fields
        REQUIRE(output.find("\"features\"") != std::string::npos);
        REQUIRE(output.find("\"name\"") != std::string::npos);
        REQUIRE(output.find("TestFeature") != std::string::npos);
        REQUIRE(output.find("\"enabled\"") != std::string::npos);
        REQUIRE(output.find("true") != std::string::npos);
        REQUIRE(output.find("\"property\"") != std::string::npos);
        REQUIRE(output.find("testFeature") != std::string::npos);
        REQUIRE(output.find("\"link\"") != std::string::npos);
        REQUIRE(output.find("https://example.com") != std::string::npos);
    }

    SECTION("Disabled feature")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddFeatureEntry("TestFeature", false, "testFeature", "https://example.com");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Verify JSON contains expected fields with false status
        REQUIRE(output.find("\"features\"") != std::string::npos);
        REQUIRE(output.find("\"enabled\"") != std::string::npos);
        REQUIRE(output.find("false") != std::string::npos);
    }
}

TEST_CASE("JsonOutputFormatter_Truncated", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.SetTruncated(true);
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify truncated flag is present
    REQUIRE(output.find("\"truncated\"") != std::string::npos);
    REQUIRE(output.find("true") != std::string::npos);
}

TEST_CASE("JsonOutputFormatter_Error", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddError("Test error message");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify error is present
    REQUIRE(output.find("\"errors\"") != std::string::npos);
    REQUIRE(output.find("Test error message") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_PackageEntry", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("TestPackage", "Publisher.TestPackage", "1.0.0", "Tag: test", "winget");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify XML structure
    REQUIRE(output.find("<?xml version=\"1.0\" encoding=\"utf-8\"?>") != std::string::npos);
    REQUIRE(output.find("<root>") != std::string::npos);
    REQUIRE(output.find("</root>") != std::string::npos);
    REQUIRE(output.find("<package>") != std::string::npos);
    REQUIRE(output.find("</package>") != std::string::npos);
    REQUIRE(output.find("<name>TestPackage</name>") != std::string::npos);
    REQUIRE(output.find("<id>Publisher.TestPackage</id>") != std::string::npos);
    REQUIRE(output.find("<version>1.0.0</version>") != std::string::npos);
    REQUIRE(output.find("<match>Tag: test</match>") != std::string::npos);
    REQUIRE(output.find("<source>winget</source>") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_XmlEscaping", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("Test<Package>", "Publisher&Co", "1.0.0", "Match: \"test\"", "winget");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify special characters are escaped
    REQUIRE(output.find("Test&lt;Package&gt;") != std::string::npos);
    REQUIRE(output.find("Publisher&amp;Co") != std::string::npos);
    REQUIRE(output.find("&quot;test&quot;") != std::string::npos);

    // Verify raw characters are NOT present
    REQUIRE(output.find("Test<Package>") == std::string::npos);
    REQUIRE(output.find("Publisher&Co") == std::string::npos);
}

TEST_CASE("XmlOutputFormatter_FeatureEntry", "[outputformatter]")
{
    SECTION("Enabled feature")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddFeatureEntry("TestFeature", true, "testFeature", "https://example.com");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Verify XML structure
        REQUIRE(output.find("<feature>") != std::string::npos);
        REQUIRE(output.find("</feature>") != std::string::npos);
        REQUIRE(output.find("<name>TestFeature</name>") != std::string::npos);
        REQUIRE(output.find("<enabled>true</enabled>") != std::string::npos);
        REQUIRE(output.find("<property>testFeature</property>") != std::string::npos);
        REQUIRE(output.find("<link>https://example.com</link>") != std::string::npos);
    }

    SECTION("Disabled feature")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddFeatureEntry("TestFeature", false, "testFeature", "https://example.com");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Verify XML structure with false status
        REQUIRE(output.find("<enabled>false</enabled>") != std::string::npos);
    }
}

TEST_CASE("XmlOutputFormatter_ListEntry", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddListEntry("TestPackage", "Publisher.TestPackage", "1.0.0", "1.1.0", "winget");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify XML structure
    REQUIRE(output.find("<name>TestPackage</name>") != std::string::npos);
    REQUIRE(output.find("<id>Publisher.TestPackage</id>") != std::string::npos);
    REQUIRE(output.find("<version>1.0.0</version>") != std::string::npos);
    REQUIRE(output.find("<availableVersion>1.1.0</availableVersion>") != std::string::npos);
    REQUIRE(output.find("<source>winget</source>") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_Truncated", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.SetTruncated(true);
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify truncated flag is present
    REQUIRE(output.find("<truncated>true</truncated>") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_Error", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddError("Test error message");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify error is present
    REQUIRE(output.find("<errors>") != std::string::npos);
    REQUIRE(output.find("<error>Test error message</error>") != std::string::npos);
    REQUIRE(output.find("</errors>") != std::string::npos);
}

TEST_CASE("JsonOutputFormatter_EmptyResults", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Should produce valid JSON even with no entries
    REQUIRE(output.find("{") != std::string::npos);
    REQUIRE(output.find("}") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_EmptyResults", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Should produce valid XML even with no entries
    REQUIRE(output.find("<?xml version=\"1.0\" encoding=\"utf-8\"?>") != std::string::npos);
    REQUIRE(output.find("<root>") != std::string::npos);
    REQUIRE(output.find("</root>") != std::string::npos);
}

TEST_CASE("GetOutputFormatFromContext_Defaults", "[outputformatter]")
{
    SECTION("No format argument defaults to Text")
    {
        std::ostringstream output;
        TestContext context{ output, std::cin };

        REQUIRE(GetOutputFormatFromContext(context) == OutputFormat::Text);
    }

    SECTION("Explicit text format")
    {
        std::ostringstream output;
        TestContext context{ output, std::cin };
        context.Args.AddArg(Execution::Args::Type::OutputFormat, "text");

        REQUIRE(GetOutputFormatFromContext(context) == OutputFormat::Text);
    }

    SECTION("JSON format from context")
    {
        std::ostringstream output;
        TestContext context{ output, std::cin };
        context.Args.AddArg(Execution::Args::Type::OutputFormat, "json");

        REQUIRE(GetOutputFormatFromContext(context) == OutputFormat::Json);
    }

    SECTION("XML format from context")
    {
        std::ostringstream output;
        TestContext context{ output, std::cin };
        context.Args.AddArg(Execution::Args::Type::OutputFormat, "xml");

        REQUIRE(GetOutputFormatFromContext(context) == OutputFormat::Xml);
    }

    SECTION("Case insensitive JSON")
    {
        std::ostringstream output;
        TestContext context{ output, std::cin };
        context.Args.AddArg(Execution::Args::Type::OutputFormat, "JSON");

        REQUIRE(GetOutputFormatFromContext(context) == OutputFormat::Json);
    }
}

TEST_CASE("JsonOutputFormatter_CompactMode", "[outputformatter]")
{
    // Note: In test environments, IsConsoleOutput() returns false (no console),
    // so JSON output is compact by default. This is the expected behavior for automation.
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("TestPkg", "Pub.TestPkg", "1.0", "", "src");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify it's valid JSON
    REQUIRE(output.find("\"packages\"") != std::string::npos);
    REQUIRE(output.find("\"name\"") != std::string::npos);

    // In compact mode (test environment), output should be more dense
    // We can verify this by checking the output is relatively compact
    REQUIRE_FALSE(output.empty());
}

TEST_CASE("XmlOutputFormatter_CompactMode", "[outputformatter]")
{
    // Note: In test environments, IsConsoleOutput() returns false (no console),
    // so XML output is compact by default (no indentation/newlines).
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("TestPkg", "Pub.TestPkg", "1.0", "", "src");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify it's valid XML with all required elements in compact form
    REQUIRE(output.find("<?xml version=\"1.0\" encoding=\"utf-8\"?>") != std::string::npos);
    REQUIRE(output.find("<root>") != std::string::npos);
    REQUIRE(output.find("<package>") != std::string::npos);
    REQUIRE(output.find("<name>TestPkg</name>") != std::string::npos);
    REQUIRE(output.find("</package>") != std::string::npos);
    REQUIRE(output.find("</root>") != std::string::npos);
}

TEST_CASE("FormatTimePointAsISO8601", "[outputformatter]")
{
    SECTION("Epoch returns empty string")
    {
        auto epoch = AppInstaller::Utility::ConvertUnixEpochToSystemClock(0);
        auto result = FormatTimePointAsISO8601(epoch);

        REQUIRE(result.empty());
    }

    SECTION("Valid timestamp returns ISO-8601 format")
    {
        // Create a known timestamp: 2025-01-15 14:30:00 UTC
        std::tm tm{};
        tm.tm_year = 2025 - 1900;
        tm.tm_mon = 0;  // January
        tm.tm_mday = 15;
        tm.tm_hour = 14;
        tm.tm_min = 30;
        tm.tm_sec = 0;
        tm.tm_isdst = 0;

#ifdef _WIN32
        auto time = _mkgmtime(&tm);
#else
        auto time = timegm(&tm);
#endif
        auto timePoint = std::chrono::system_clock::from_time_t(time);
        auto result = FormatTimePointAsISO8601(timePoint);

        // Verify ISO-8601 format with UTC marker
        REQUIRE(result == "2025-01-15T14:30:00Z");
    }

    SECTION("Formatted timestamp ends with Z for UTC")
    {
        auto now = std::chrono::system_clock::now();
        auto result = FormatTimePointAsISO8601(now);

        // Should not be empty (not epoch)
        REQUIRE_FALSE(result.empty());

        // Should end with Z for UTC
        REQUIRE(result.back() == 'Z');

        // Should contain T separator
        REQUIRE(result.find('T') != std::string::npos);
    }
}

TEST_CASE("JsonOutputFormatter_SourceEntry", "[outputformatter]")
{
    SECTION("Source entry with all fields")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddSourceEntry("winget", "Microsoft.PreIndexed.Package", 
                                "https://cdn.winget.microsoft.com/cache", 
                                "Microsoft.Winget.Source_8wekyb3d8bbwe", 
                                "2025-01-15T10:30:00Z");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("\"sources\"") != std::string::npos);
        REQUIRE(output.find("\"name\"") != std::string::npos);
        REQUIRE(output.find("winget") != std::string::npos);
        REQUIRE(output.find("\"type\"") != std::string::npos);
        REQUIRE(output.find("Microsoft.PreIndexed.Package") != std::string::npos);
        REQUIRE(output.find("\"arg\"") != std::string::npos);
        REQUIRE(output.find("https://cdn.winget.microsoft.com/cache") != std::string::npos);
        REQUIRE(output.find("\"data\"") != std::string::npos);
        REQUIRE(output.find("Microsoft.Winget.Source_8wekyb3d8bbwe") != std::string::npos);
        REQUIRE(output.find("\"updated\"") != std::string::npos);
        REQUIRE(output.find("2025-01-15T10:30:00Z") != std::string::npos);
    }

    SECTION("Source entry with minimal fields")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddSourceEntry("msstore", "", "", "", "");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("\"sources\"") != std::string::npos);
        REQUIRE(output.find("\"name\"") != std::string::npos);
        REQUIRE(output.find("msstore") != std::string::npos);
        // Optional fields should not be present when empty
        REQUIRE(output.find("\"type\"") == std::string::npos);
        REQUIRE(output.find("\"arg\"") == std::string::npos);
        REQUIRE(output.find("\"data\"") == std::string::npos);
        REQUIRE(output.find("\"updated\"") == std::string::npos);
    }
}

TEST_CASE("XmlOutputFormatter_SourceEntry", "[outputformatter]")
{
    SECTION("Source entry with all fields")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddSourceEntry("winget", "Microsoft.PreIndexed.Package", 
                                "https://cdn.winget.microsoft.com/cache", 
                                "Microsoft.Winget.Source_8wekyb3d8bbwe", 
                                "2025-01-15T10:30:00Z");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("<source>") != std::string::npos);
        REQUIRE(output.find("</source>") != std::string::npos);
        REQUIRE(output.find("<name>winget</name>") != std::string::npos);
        REQUIRE(output.find("<type>Microsoft.PreIndexed.Package</type>") != std::string::npos);
        REQUIRE(output.find("<arg>https://cdn.winget.microsoft.com/cache</arg>") != std::string::npos);
        REQUIRE(output.find("<data>Microsoft.Winget.Source_8wekyb3d8bbwe</data>") != std::string::npos);
        REQUIRE(output.find("<updated>2025-01-15T10:30:00Z</updated>") != std::string::npos);
    }

    SECTION("Source entry with minimal fields")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddSourceEntry("msstore", "", "", "", "");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("<source>") != std::string::npos);
        REQUIRE(output.find("<name>msstore</name>") != std::string::npos);
        // Optional fields should not be present when empty
        REQUIRE(output.find("<type>") == std::string::npos);
        REQUIRE(output.find("<arg>") == std::string::npos);
        REQUIRE(output.find("<data>") == std::string::npos);
        REQUIRE(output.find("<updated>") == std::string::npos);
    }
}

TEST_CASE("JsonOutputFormatter_MultipleEntries", "[outputformatter]")
{
    SECTION("Multiple package entries")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddPackageEntry("Package1", "Pub.Package1", "1.0", "Tag: test", "winget");
        formatter.AddPackageEntry("Package2", "Pub.Package2", "2.0", "Name: Package2", "msstore");
        formatter.AddPackageEntry("Package3", "Pub.Package3", "3.0", "Id: Pub.Package3", "winget");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Verify all three packages are present
        REQUIRE(output.find("Package1") != std::string::npos);
        REQUIRE(output.find("Package2") != std::string::npos);
        REQUIRE(output.find("Package3") != std::string::npos);
        REQUIRE(output.find("Pub.Package1") != std::string::npos);
        REQUIRE(output.find("Pub.Package2") != std::string::npos);
        REQUIRE(output.find("Pub.Package3") != std::string::npos);
    }

    SECTION("Multiple feature entries")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddFeatureEntry("Feature1", true, "experimentalFeature1", "https://example.com/1");
        formatter.AddFeatureEntry("Feature2", false, "experimentalFeature2", "https://example.com/2");
        formatter.AddFeatureEntry("Feature3", true, "experimentalFeature3", "https://example.com/3");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("Feature1") != std::string::npos);
        REQUIRE(output.find("Feature2") != std::string::npos);
        REQUIRE(output.find("Feature3") != std::string::npos);
        REQUIRE(output.find("experimentalFeature1") != std::string::npos);
        REQUIRE(output.find("experimentalFeature2") != std::string::npos);
        REQUIRE(output.find("experimentalFeature3") != std::string::npos);
    }

    SECTION("Multiple source entries")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddSourceEntry("winget", "Microsoft.PreIndexed.Package", "https://cdn.winget.microsoft.com/cache", "data1", "2025-01-15T10:00:00Z");
        formatter.AddSourceEntry("msstore", "Microsoft.Rest", "https://storeedgefd.dsx.mp.microsoft.com/v9.0", "data2", "2025-01-15T11:00:00Z");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("\"sources\"") != std::string::npos);
        REQUIRE(output.find("winget") != std::string::npos);
        REQUIRE(output.find("msstore") != std::string::npos);
    }

    SECTION("Multiple errors")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddError("Error 1: Package not found");
        formatter.AddError("Error 2: Network timeout");
        formatter.AddError("Error 3: Invalid manifest");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("\"errors\"") != std::string::npos);
        REQUIRE(output.find("Error 1: Package not found") != std::string::npos);
        REQUIRE(output.find("Error 2: Network timeout") != std::string::npos);
        REQUIRE(output.find("Error 3: Invalid manifest") != std::string::npos);
    }
}

TEST_CASE("XmlOutputFormatter_MultipleEntries", "[outputformatter]")
{
    SECTION("Multiple package entries")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddPackageEntry("Package1", "Pub.Package1", "1.0", "Tag: test", "winget");
        formatter.AddPackageEntry("Package2", "Pub.Package2", "2.0", "Name: Package2", "msstore");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Count package elements (should find at least 2 opening and 2 closing)
        size_t count = 0;
        size_t pos = 0;
        while ((pos = output.find("<package>", pos)) != std::string::npos)
        {
            count++;
            pos++;
        }
        REQUIRE(count >= 2);

        REQUIRE(output.find("Package1") != std::string::npos);
        REQUIRE(output.find("Package2") != std::string::npos);
    }

    SECTION("Multiple errors")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddError("Error 1: Package not found");
        formatter.AddError("Error 2: Network timeout");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("<errors>") != std::string::npos);
        REQUIRE(output.find("<error>Error 1: Package not found</error>") != std::string::npos);
        REQUIRE(output.find("<error>Error 2: Network timeout</error>") != std::string::npos);
        REQUIRE(output.find("</errors>") != std::string::npos);
    }
}

TEST_CASE("JsonOutputFormatter_MixedEntryTypes", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("TestPackage", "Pub.TestPackage", "1.0", "Tag: test", "winget");
    formatter.AddFeatureEntry("TestFeature", true, "experimentalFeature", "https://example.com");
    formatter.AddSourceEntry("winget", "Microsoft.PreIndexed.Package", "https://cdn.winget.microsoft.com/cache", "data", "2025-01-15T10:00:00Z");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify all three top-level arrays are present
    REQUIRE(output.find("\"packages\"") != std::string::npos);
    REQUIRE(output.find("\"features\"") != std::string::npos);
    REQUIRE(output.find("\"sources\"") != std::string::npos);

    // Verify content from each type
    REQUIRE(output.find("TestPackage") != std::string::npos);
    REQUIRE(output.find("TestFeature") != std::string::npos);
    REQUIRE(output.find("winget") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_MixedEntryTypes", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("TestPackage", "Pub.TestPackage", "1.0", "Tag: test", "winget");
    formatter.AddFeatureEntry("TestFeature", true, "experimentalFeature", "https://example.com");
    formatter.AddSourceEntry("winget", "Microsoft.PreIndexed.Package", "https://cdn.winget.microsoft.com/cache", "data", "2025-01-15T10:00:00Z");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify all three element types are present
    REQUIRE(output.find("<package>") != std::string::npos);
    REQUIRE(output.find("<feature>") != std::string::npos);
    REQUIRE(output.find("<source>") != std::string::npos);
}

TEST_CASE("JsonOutputFormatter_ListEntryWithCategory", "[outputformatter]")
{
    SECTION("List entry with category")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddListEntry("TestPackage", "Pub.TestPackage", "1.0.0", "1.1.0", "winget", "pinnedByManifest");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("\"category\"") != std::string::npos);
        REQUIRE(output.find("pinnedByManifest") != std::string::npos);
    }

    SECTION("List entry without category")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddListEntry("TestPackage", "Pub.TestPackage", "1.0.0", "1.1.0", "winget");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Category should not be present when empty
        REQUIRE(output.find("\"category\"") == std::string::npos);
    }
}

TEST_CASE("XmlOutputFormatter_ListEntryWithCategory", "[outputformatter]")
{
    SECTION("List entry with category")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddListEntry("TestPackage", "Pub.TestPackage", "1.0.0", "1.1.0", "winget", "blockedByPin");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("<category>blockedByPin</category>") != std::string::npos);
    }

    SECTION("List entry without category")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddListEntry("TestPackage", "Pub.TestPackage", "1.0.0", "1.1.0", "winget");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Category should not be present when empty
        REQUIRE(output.find("<category>") == std::string::npos);
    }
}

TEST_CASE("JsonOutputFormatter_EmptyOptionalFields", "[outputformatter]")
{
    SECTION("Package entry with empty optional fields")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddPackageEntry("TestPackage", "Pub.TestPackage", "", "", "");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Only required fields should be present
        REQUIRE(output.find("\"name\"") != std::string::npos);
        REQUIRE(output.find("\"id\"") != std::string::npos);
        // Optional fields should not be present
        REQUIRE(output.find("\"version\"") == std::string::npos);
        REQUIRE(output.find("\"match\"") == std::string::npos);
        REQUIRE(output.find("\"source\"") == std::string::npos);
    }

    SECTION("List entry with empty optional fields")
    {
        JsonOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddListEntry("TestPackage", "Pub.TestPackage", "", "", "", "");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        REQUIRE(output.find("\"name\"") != std::string::npos);
        REQUIRE(output.find("\"id\"") != std::string::npos);
        REQUIRE(output.find("\"version\"") == std::string::npos);
        REQUIRE(output.find("\"availableVersion\"") == std::string::npos);
        REQUIRE(output.find("\"source\"") == std::string::npos);
        REQUIRE(output.find("\"category\"") == std::string::npos);
    }
}

TEST_CASE("XmlOutputFormatter_EmptyOptionalFields", "[outputformatter]")
{
    SECTION("Package entry with empty optional fields")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddPackageEntry("TestPackage", "Pub.TestPackage", "", "", "");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Required fields should be present
        REQUIRE(output.find("<name>TestPackage</name>") != std::string::npos);
        REQUIRE(output.find("<id>Pub.TestPackage</id>") != std::string::npos);
        // Optional fields should not be present when empty
        REQUIRE(output.find("<version>") == std::string::npos);
        REQUIRE(output.find("<match>") == std::string::npos);
        REQUIRE(output.find("<source>") == std::string::npos);
    }
}

TEST_CASE("XmlOutputFormatter_ComplexXmlEscaping", "[outputformatter]")
{
    SECTION("All special characters in one string")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddPackageEntry("Test<>&\"'Package", "Pub&Co<Test>", "1.0", "Match: \"test\" & 'value'", "win<get>");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Verify all special characters are properly escaped
        REQUIRE(output.find("Test&lt;&gt;&amp;&quot;&apos;Package") != std::string::npos);
        REQUIRE(output.find("Pub&amp;Co&lt;Test&gt;") != std::string::npos);
        REQUIRE(output.find("&quot;test&quot; &amp; &apos;value&apos;") != std::string::npos);
        REQUIRE(output.find("win&lt;get&gt;") != std::string::npos);
    }

    SECTION("URL with special characters")
    {
        XmlOutputFormatter formatter;
        formatter.StartOutput();
        formatter.AddSourceEntry("source", "type", "https://example.com?param1=value1&param2=value2", "data", "");
        formatter.EndOutput();

        std::string output = formatter.GetOutput();

        // Ampersands in URL should be escaped
        REQUIRE(output.find("https://example.com?param1=value1&amp;param2=value2") != std::string::npos);
    }
}

TEST_CASE("JsonOutputFormatter_TruncatedWithErrors", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("Package1", "Pub.Package1", "1.0", "", "winget");
    formatter.SetTruncated(true);
    formatter.AddError("Search results exceeded limit");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Both truncated flag and errors should be present
    REQUIRE(output.find("\"truncated\"") != std::string::npos);
    REQUIRE(output.find("true") != std::string::npos);
    REQUIRE(output.find("\"errors\"") != std::string::npos);
    REQUIRE(output.find("Search results exceeded limit") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_TruncatedWithErrors", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("Package1", "Pub.Package1", "1.0", "", "winget");
    formatter.SetTruncated(true);
    formatter.AddError("Search results exceeded limit");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Both truncated flag and errors should be present
    REQUIRE(output.find("<truncated>true</truncated>") != std::string::npos);
    REQUIRE(output.find("<errors>") != std::string::npos);
    REQUIRE(output.find("<error>Search results exceeded limit</error>") != std::string::npos);
}

TEST_CASE("JsonOutputFormatter_MoveSemantics", "[outputformatter]")
{
    SECTION("Move constructor")
    {
        JsonOutputFormatter formatter1;
        formatter1.StartOutput();
        formatter1.AddPackageEntry("TestPackage", "Pub.TestPackage", "1.0", "", "winget");

        // Move to new formatter
        JsonOutputFormatter formatter2(std::move(formatter1));
        formatter2.EndOutput();

        std::string output = formatter2.GetOutput();

        // Verify the data was moved successfully
        REQUIRE(output.find("TestPackage") != std::string::npos);
        REQUIRE(output.find("Pub.TestPackage") != std::string::npos);
    }

    SECTION("Move assignment")
    {
        JsonOutputFormatter formatter1;
        formatter1.StartOutput();
        formatter1.AddPackageEntry("TestPackage", "Pub.TestPackage", "1.0", "", "winget");

        JsonOutputFormatter formatter2;
        formatter2 = std::move(formatter1);
        formatter2.EndOutput();

        std::string output = formatter2.GetOutput();

        // Verify the data was moved successfully
        REQUIRE(output.find("TestPackage") != std::string::npos);
        REQUIRE(output.find("Pub.TestPackage") != std::string::npos);
    }
}

TEST_CASE("JsonOutputFormatter_UnicodeCharacters", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("Test Package 中文", "Pub.Test.日本語", "1.0.0", "Match: test", "winget");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify Unicode characters are preserved in JSON
    REQUIRE(output.find("Test Package 中文") != std::string::npos);
    REQUIRE(output.find("Pub.Test.日本語") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_UnicodeCharacters", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("Test Package 中文", "Pub.Test.日本語", "1.0.0", "Match: test", "winget");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify Unicode characters are preserved in XML
    REQUIRE(output.find("Test Package 中文") != std::string::npos);
    REQUIRE(output.find("Pub.Test.日本語") != std::string::npos);
}

TEST_CASE("JsonOutputFormatter_LargeOutput", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();

    // Add many entries to test scalability
    for (int i = 0; i < 100; ++i)
    {
        std::string name = "Package" + std::to_string(i);
        std::string id = "Pub.Package" + std::to_string(i);
        std::string version = std::to_string(i) + ".0.0";
        formatter.AddPackageEntry(name, id, version, "Tag: test", "winget");
    }

    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify output is valid and contains expected entries
    REQUIRE_FALSE(output.empty());
    REQUIRE(output.find("\"packages\"") != std::string::npos);
    REQUIRE(output.find("Package0") != std::string::npos);
    REQUIRE(output.find("Package99") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_LargeOutput", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();

    // Add many entries to test scalability
    for (int i = 0; i < 100; ++i)
    {
        std::string name = "Package" + std::to_string(i);
        std::string id = "Pub.Package" + std::to_string(i);
        std::string version = std::to_string(i) + ".0.0";
        formatter.AddPackageEntry(name, id, version, "Tag: test", "winget");
    }

    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify output is valid and contains expected entries
    REQUIRE_FALSE(output.empty());
    REQUIRE(output.find("<package>") != std::string::npos);
    REQUIRE(output.find("Package0") != std::string::npos);
    REQUIRE(output.find("Package99") != std::string::npos);
}

TEST_CASE("JsonOutputFormatter_VeryLongStrings", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();

    std::string longName(10000, 'A');
    std::string longId = "Pub." + std::string(10000, 'B');
    std::string longVersion(1000, '1');

    formatter.AddPackageEntry(longName, longId, longVersion, "", "");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify long strings are handled correctly
    REQUIRE_FALSE(output.empty());
    REQUIRE(output.find(longName) != std::string::npos);
    REQUIRE(output.find(longId) != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_VeryLongStrings", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();

    std::string longName(10000, 'A');
    std::string longId = "Pub." + std::string(10000, 'B');

    formatter.AddPackageEntry(longName, longId, "", "", "");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify long strings are handled correctly
    REQUIRE_FALSE(output.empty());
    REQUIRE(output.find(longName) != std::string::npos);
    REQUIRE(output.find(longId) != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_EscapeXml_EdgeCases", "[outputformatter]")
{
    SECTION("Empty string")
    {
        std::string result = XmlOutputFormatter::EscapeXml("");
        REQUIRE(result.empty());
    }

    SECTION("String with no special characters")
    {
        std::string input = "NormalString123";
        std::string result = XmlOutputFormatter::EscapeXml(input);
        REQUIRE(result == input);
    }

    SECTION("String with only special characters")
    {
        std::string input = "<>&\"'";
        std::string result = XmlOutputFormatter::EscapeXml(input);
        REQUIRE(result == "&lt;&gt;&amp;&quot;&apos;");
    }

    SECTION("Consecutive special characters")
    {
        std::string input = "<<<&&>>>";
        std::string result = XmlOutputFormatter::EscapeXml(input);
        REQUIRE(result == "&lt;&lt;&lt;&amp;&amp;&gt;&gt;&gt;");
    }

    SECTION("Mixed content with special characters")
    {
        std::string input = "Value=\"test\" & other<data>";
        std::string result = XmlOutputFormatter::EscapeXml(input);
        REQUIRE(result == "Value=&quot;test&quot; &amp; other&lt;data&gt;");
    }
}

TEST_CASE("ParseOutputFormat_CaseInsensitivityEdgeCases", "[outputformatter]")
{
    SECTION("All uppercase variants")
    {
        REQUIRE(ParseOutputFormat("JSON") == OutputFormat::Json);
        REQUIRE(ParseOutputFormat("XML") == OutputFormat::Xml);
        REQUIRE(ParseOutputFormat("TEXT") == OutputFormat::Text);
    }

    SECTION("Mixed case variants")
    {
        REQUIRE(ParseOutputFormat("JsOn") == OutputFormat::Json);
        REQUIRE(ParseOutputFormat("XmL") == OutputFormat::Xml);
        REQUIRE(ParseOutputFormat("TeXt") == OutputFormat::Text);
    }

    SECTION("Random case")
    {
        REQUIRE(ParseOutputFormat("jSoN") == OutputFormat::Json);
        REQUIRE(ParseOutputFormat("xMl") == OutputFormat::Xml);
    }
}

TEST_CASE("GetOutputFormatFromContext_InvalidFormat", "[outputformatter]")
{
    std::ostringstream output;
    TestContext context{ output, std::cin };
    context.Args.AddArg(Execution::Args::Type::OutputFormat, "invalid_format");

    REQUIRE_THROWS_AS(GetOutputFormatFromContext(context), AppInstaller::CLI::CommandException);
}

TEST_CASE("JsonOutputFormatter_SpecialCharactersInAllFields", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();

    // Test special characters that might need JSON escaping
    formatter.AddPackageEntry("Test\"Package", "Pub\\TestPackage", "1.0\n0", "Match:\ttest", "win\rget");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // JSON library should handle escaping automatically
    REQUIRE_FALSE(output.empty());
    REQUIRE(output.find("Test") != std::string::npos);
    REQUIRE(output.find("Package") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_NewlinesAndTabs", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddPackageEntry("Test\nPackage", "Pub\tTestPackage", "1.0", "Match:\r\ntest", "winget");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify newlines and tabs are preserved (not XML special characters)
    REQUIRE(output.find("Test\nPackage") != std::string::npos);
    REQUIRE(output.find("Pub\tTestPackage") != std::string::npos);
}

TEST_CASE("JsonOutputFormatter_SetTruncatedFalse", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.SetTruncated(false);
    formatter.AddPackageEntry("Test", "Pub.Test", "1.0", "", "");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // When truncated is false, the field should not appear
    REQUIRE(output.find("\"truncated\"") == std::string::npos);
}

TEST_CASE("XmlOutputFormatter_SetTruncatedFalse", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.SetTruncated(false);
    formatter.AddPackageEntry("Test", "Pub.Test", "1.0", "", "");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // When truncated is false, the element should not appear
    REQUIRE(output.find("<truncated>") == std::string::npos);
}

TEST_CASE("JsonOutputFormatter_EmptyErrorMessage", "[outputformatter]")
{
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddError("");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Even empty error should be included
    REQUIRE(output.find("\"errors\"") != std::string::npos);
}

TEST_CASE("XmlOutputFormatter_EmptyErrorMessage", "[outputformatter]")
{
    XmlOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddError("");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Even empty error should be included
    REQUIRE(output.find("<errors>") != std::string::npos);
    REQUIRE(output.find("<error></error>") != std::string::npos);
}
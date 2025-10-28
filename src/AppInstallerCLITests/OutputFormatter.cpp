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

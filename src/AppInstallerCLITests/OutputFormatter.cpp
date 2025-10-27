// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "TestCommon.h"
#include <OutputFormatter.h>
#include <Command.h>

using namespace AppInstaller::CLI::Execution;

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
    JsonOutputFormatter formatter;
    formatter.StartOutput();
    formatter.AddFeatureEntry("TestFeature", "enabled", "testFeature", "https://example.com");
    formatter.EndOutput();

    std::string output = formatter.GetOutput();

    // Verify JSON contains expected fields
    REQUIRE(output.find("\"features\"") != std::string::npos);
    REQUIRE(output.find("\"name\"") != std::string::npos);
    REQUIRE(output.find("TestFeature") != std::string::npos);
    REQUIRE(output.find("\"status\"") != std::string::npos);
    REQUIRE(output.find("enabled") != std::string::npos);
    REQUIRE(output.find("\"property\"") != std::string::npos);
    REQUIRE(output.find("testFeature") != std::string::npos);
    REQUIRE(output.find("\"link\"") != std::string::npos);
    REQUIRE(output.find("https://example.com") != std::string::npos);
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

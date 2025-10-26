// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "TestCommon.h"
#include "WorkflowCommon.h"
#include <Commands/SearchCommand.h>
#include <Commands/ListCommand.h>
#include <Commands/ShowCommand.h>
#include <json/json.h>

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace TestCommon;
using namespace AppInstaller::CLI;
using namespace AppInstaller::CLI::Execution;
using namespace AppInstaller::Utility;

// Helper to parse JSON from string
Json::Value ParseJson(const std::string& jsonString)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream jsonStream(jsonString);
    bool success = Json::parseFromStream(builder, jsonStream, &root, &errors);
    REQUIRE(success);
    return root;
}

TEST_CASE (
"OutputFormat_SearchCommand_ValidJson"
,
"[OutputFormat][SearchCommand]"
)
{
    std::ostringstream searchOutput;
    TestContext context{ searchOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::Query, TSR::TestQuery_ReturnOne.Query);
    context.Args.AddArg(Args::Type::OutputFormat, "json");

    SearchCommand search({});
    search.Execute(context);
    INFO(searchOutput.str());

    // Verify JSON output structure
    auto json = ParseJson(searchOutput.str());
    REQUIRE(json.isMember("Packages"));
    REQUIRE(json["Packages"].isArray());
    REQUIRE(json["Packages"].size() > 0);

    auto firstPackage = json["Packages"][0];
    REQUIRE(firstPackage.isMember("PackageName"));
    REQUIRE(firstPackage.isMember("PackageId"));
    REQUIRE(firstPackage.isMember("Version"));
}

TEST_CASE (
"OutputFormat_SearchCommand_ValidTable"
,
"[OutputFormat][SearchCommand]"
)
{
    std::ostringstream searchOutput;
    TestContext context{ searchOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::Query, TSR::TestQuery_ReturnOne.Query);
    context.Args.AddArg(Args::Type::OutputFormat, "table");

    SearchCommand search({});
    search.Execute(context);
    INFO(searchOutput.str());

    // Verify table output (not JSON)
    REQUIRE(searchOutput.str().find("Name") != std::string::npos);
    REQUIRE(searchOutput.str().find("Id") != std::string::npos);
    REQUIRE_THROWS(ParseJson(searchOutput.str()));
}

TEST_CASE (
"OutputFormat_SearchCommand_DefaultIsTable"
,
"[OutputFormat][SearchCommand]"
)
{
    std::ostringstream searchOutput;
    TestContext context{ searchOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::Query, TSR::TestQuery_ReturnOne.Query);
    // No OutputFormat arg - should default to table

    SearchCommand search({});
    search.Execute(context);
    INFO(searchOutput.str());

    // Verify table output (not JSON)
    REQUIRE(searchOutput.str().find("Name") != std::string::npos);
    REQUIRE_THROWS(ParseJson(searchOutput.str()));
}

TEST_CASE (
"OutputFormat_SearchCommand_CaseInsensitive"
,
"[OutputFormat][SearchCommand]"
)
{
    std::ostringstream searchOutput;
    TestContext context{ searchOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::Query, TSR::TestQuery_ReturnOne.Query);
    context.Args.AddArg(Args::Type::OutputFormat, "JSON");

    SearchCommand search({});
    search.Execute(context);
    INFO(searchOutput.str());

    // Verify JSON output
    auto json = ParseJson(searchOutput.str());
    REQUIRE(json.isMember("Packages"));
}

TEST_CASE (
"OutputFormat_SearchCommand_WithWhitespace"
,
"[OutputFormat][SearchCommand]"
)
{
    std::ostringstream searchOutput;
    TestContext context{ searchOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::Query, TSR::TestQuery_ReturnOne.Query);
    context.Args.AddArg(Args::Type::OutputFormat, " json ");

    SearchCommand search({});
    search.Execute(context);
    INFO(searchOutput.str());

    // Verify JSON output
    auto json = ParseJson(searchOutput.str());
    REQUIRE(json.isMember("Packages"));
}

TEST_CASE (
"OutputFormat_SearchCommand_InvalidFormat"
,
"[OutputFormat][SearchCommand]"
)
{
    Args args;
    args.AddArg(Args::Type::Query, "test");
    args.AddArg(Args::Type::OutputFormat, "xml");

    SearchCommand search({});
    REQUIRE_THROWS_AS(search.ValidateArguments(args), CommandException);
}

TEST_CASE (
"OutputFormat_SearchCommand_InvalidFormatCsv"
,
"[OutputFormat][SearchCommand]"
)
{
    Args args;
    args.AddArg(Args::Type::Query, "test");
    args.AddArg(Args::Type::OutputFormat, "csv");

    SearchCommand search({});
    REQUIRE_THROWS_AS(search.ValidateArguments(args), CommandException);
}

TEST_CASE (
"OutputFormat_ListCommand_ValidJson"
,
"[OutputFormat][ListCommand]"
)
{
    std::ostringstream listOutput;
    TestContext context{ listOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForShellExecute(context);
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::OutputFormat, "json");

    ListCommand list({});
    list.Execute(context);
    INFO(listOutput.str());

    // Verify JSON output structure
    auto json = ParseJson(listOutput.str());
    REQUIRE(json.isMember("Packages"));
    REQUIRE(json["Packages"].isArray());
}

TEST_CASE (
"OutputFormat_ListCommand_ValidTable"
,
"[OutputFormat][ListCommand]"
)
{
    std::ostringstream listOutput;
    TestContext context{ listOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForShellExecute(context);
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::OutputFormat, "table");

    ListCommand list({});
    list.Execute(context);
    INFO(listOutput.str());

    // Verify table output (not JSON)
    // List output may be empty or have content, just verify it's not JSON
    std::string output = listOutput.str();
    if (!output.empty())
    {
        REQUIRE_THROWS(ParseJson(output));
    }
}

TEST_CASE (
"OutputFormat_ListCommand_InvalidFormat"
,
"[OutputFormat][ListCommand]"
)
{
    Args args;
    args.AddArg(Args::Type::OutputFormat, "yaml");

    ListCommand list({});
    REQUIRE_THROWS_AS(list.ValidateArguments(args), CommandException);
}

TEST_CASE (
"OutputFormat_ShowCommand_ValidJson"
,
"[OutputFormat][ShowCommand]"
)
{
    std::ostringstream showOutput;
    TestContext context{ showOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::Query, TSR::TestQuery_ReturnOne.Query);
    context.Args.AddArg(Args::Type::OutputFormat, "json");

    ShowCommand show({});
    show.Execute(context);
    INFO(showOutput.str());

    // Verify JSON output structure
    auto json = ParseJson(showOutput.str());
    REQUIRE(json.isMember("PackageName"));
    REQUIRE(json.isMember("PackageId"));
    REQUIRE(json.isMember("Version"));
    REQUIRE(json.isMember("Publisher"));
}

TEST_CASE (
"OutputFormat_ShowCommand_ValidTable"
,
"[OutputFormat][ShowCommand]"
)
{
    std::ostringstream showOutput;
    TestContext context{ showOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::Query, TSR::TestQuery_ReturnOne.Query);
    context.Args.AddArg(Args::Type::OutputFormat, "table");

    ShowCommand show({});
    show.Execute(context);
    INFO(showOutput.str());

    // Verify table output (not JSON)
    REQUIRE(showOutput.str().find("AppInstallerCliTest.TestExeInstaller") != std::string::npos);
    REQUIRE_THROWS(ParseJson(showOutput.str()));
}

TEST_CASE (
"OutputFormat_ShowCommand_DefaultIsTable"
,
"[OutputFormat][ShowCommand]"
)
{
    std::ostringstream showOutput;
    TestContext context{ showOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    OverrideForOpenSource(context, CreateTestSource({ TSR::TestQuery_ReturnOne }));
    context.Args.AddArg(Args::Type::Query, TSR::TestQuery_ReturnOne.Query);
    // No OutputFormat arg - should default to table

    ShowCommand show({});
    show.Execute(context);
    INFO(showOutput.str());

    // Verify table output (not JSON)
    REQUIRE(showOutput.str().find("AppInstallerCliTest.TestExeInstaller") != std::string::npos);
    REQUIRE_THROWS(ParseJson(showOutput.str()));
}

TEST_CASE (
"OutputFormat_ShowCommand_InvalidFormat"
,
"[OutputFormat][ShowCommand]"
)
{
    Args args;
    args.AddArg(Args::Type::Query, "test");
    args.AddArg(Args::Type::OutputFormat, "html");

    ShowCommand show({});
    REQUIRE_THROWS_AS(show.ValidateArguments(args), CommandException);
}

TEST_CASE (
"OutputFormat_ShowCommand_JsonWithManifest"
,
"[OutputFormat][ShowCommand]"
)
{
    std::ostringstream showOutput;
    TestContext context{ showOutput, std::cin };
    auto previousThreadGlobals = context.SetForCurrentThread();
    context.Args.AddArg(Args::Type::Manifest, TestDataFile("InstallFlowTest_Exe.yaml").GetPath().u8string());
    context.Args.AddArg(Args::Type::OutputFormat, "json");

    ShowCommand show({});
    show.Execute(context);
    INFO(showOutput.str());

    // Verify JSON output structure
    auto json = ParseJson(showOutput.str());
    REQUIRE(json.isMember("PackageName"));
    REQUIRE(json.isMember("PackageId"));
    REQUIRE(json.isMember("Version"));

    // Verify optional fields may or may not be present
    if (json.isMember("Installer"))
    {
        REQUIRE(json["Installer"].isObject());
        REQUIRE(json["Installer"].isMember("InstallerType"));
    }
}

TEST_CASE (
"OutputFormat_ValidationNormalization"
,
"[OutputFormat]"
)
{
    // Test that various formats of "table" are normalized and accepted
    std::vector<std::string> validTableFormats = {
        "table",
        "TABLE",
        "Table",
        " table ",
        " TABLE ",
        "  table  "
    };

    for (const auto& format : validTableFormats)
    {
        INFO("Testing format: '" + format + "'");
        Args args;
        args.AddArg(Args::Type::Query, "test");
        args.AddArg(Args::Type::OutputFormat, format);

        SearchCommand search({});
        REQUIRE_NOTHROW(search.ValidateArguments(args));
    }

    // Test that various formats of "json" are normalized and accepted
    std::vector<std::string> validJsonFormats = {
        "json",
        "JSON",
        "Json",
        " json ",
        " JSON ",
        "  json  "
    };

    for (const auto& format : validJsonFormats)
    {
        INFO("Testing format: '" + format + "'");
        Args args;
        args.AddArg(Args::Type::Query, "test");
        args.AddArg(Args::Type::OutputFormat, format);

        SearchCommand search({});
        REQUIRE_NOTHROW(search.ValidateArguments(args));
    }
}

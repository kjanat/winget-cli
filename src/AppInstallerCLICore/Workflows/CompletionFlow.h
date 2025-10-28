// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "ExecutionContext.h"
#include "WorkflowBase.h"
#include <winget/RepositorySearch.h>

/**
 * Provide completion candidates for a source name argument based on CompletionData.
 * @param context Execution context containing CompletionData used to produce completions.
 */
 
/**
 * End the completion flow when the current completion word is empty.
 * @param context Execution context containing CompletionData whose completion word is validated.
 */
 
/**
 * Emit the matched field value for the current SearchResult as a completion candidate.
 * @param context Execution context containing a SearchResult whose matched field will be output.
 */
 
/**
 * Emit available version strings for the single SearchResult as completion candidates.
 * @param context Execution context containing CompletionData and a SearchResult to derive versions from.
 */
 
/**
 * Emit available channel identifiers for the single SearchResult as completion candidates.
 * @param context Execution context containing CompletionData and a SearchResult to derive channels from.
 */
 
/**
 * Execute the completion flow for an argument when a command targets a single manifest (for example, show or install).
 * The behavior used depends on the argument type provided to the task.
 * @param context Execution context containing CompletionData used by the task.
 */
 
/**
 * Execute the completion flow for an argument when a command targets a single manifest (for example, show or install),
 * using an already-open source present in the execution context.
 * The behavior used depends on the argument type provided to the task.
 * @param context Execution context containing CompletionData and an open Source used by the task.
 */
 
/**
 * Output an empty line to indicate that no completion candidates are available.
 * @param context Execution context used to emit the empty completion output.
 */
 
/**
 * Emit completion candidates for the --output-format argument (for example, "json" and "xml").
 * @param context Execution context used to output the format candidates.
 */
namespace AppInstaller::CLI::Workflow
{
    // Outputs completion possibilities for the source name argument.
    // Required Args: None
    // Inputs: CompletionData
    // Outputs: None
    void CompleteSourceName(Execution::Context& context);

    // Terminates the context if the completion word is empty.
    // Required Args: None
    // Inputs: CompletionData
    // Outputs: None
    void RequireCompletionWordNonEmpty(Execution::Context& context);

    // Outputs the matched field for the results.
    // Required Args: None
    // Inputs: SearchResult
    // Outputs: None
    void CompleteWithMatchedField(Execution::Context& context);

    // Outputs the versions available for the single search result.
    // Required Args: None
    // Inputs: CompletionData, SearchResult
    // Outputs: None
    void CompleteWithSearchResultVersions(Execution::Context& context);

    // Outputs the channels available for the single search result.
    // Required Args: None
    // Inputs: CompletionData, SearchResult
    // Outputs: None
    void CompleteWithSearchResultChannels(Execution::Context& context);

    // Executes the appropriate completion flow for the given argument in the context of a command
    // that targets a single manifest (ex. show or install).
    // Required Args: None
    // Inputs: CompletionData
    // Outputs: None
    struct CompleteWithSingleSemanticsForValue : public WorkflowTask
    {
        CompleteWithSingleSemanticsForValue(Execution::Args::Type type) : WorkflowTask("CompleteWithSingleSemanticsForValue"), m_type(type) {}

        void operator()(Execution::Context& context) const override;

    private:
        Execution::Args::Type m_type;
    };

    // Executes the appropriate completion flow for the given argument in the context of a command
    // that targets a single manifest (ex. show or install), using the already open source.
    // Required Args: None
    // Inputs: CompletionData, Source
    // Outputs: None
    struct CompleteWithSingleSemanticsForValueUsingExistingSource : public WorkflowTask
    {
        CompleteWithSingleSemanticsForValueUsingExistingSource(Execution::Args::Type type) : WorkflowTask("CompleteWithSingleSemanticsForValueUsingExistingSource"), m_type(type) {}

        void operator()(Execution::Context& context) const override;

    private:
        Execution::Args::Type m_type;
    };

    // Outputs an empty line to indicate that there are no completions.
    // Required Args: None
    // Inputs: None
    // Outputs: None
    void CompleteWithEmptySet(Execution::Context& context);

    // Outputs completion values for the --output-format argument (json, xml).
    // Required Args: None
    // Inputs: None
    // Outputs: None
    void CompleteOutputFormat(Execution::Context& context);
}
---
title: features Command
description: Displays the list of experimental features available and the state.
ms.date: 05/5/2021
ms.topic: overview
ms.localizationpriority: medium
---

# features command (winget)

The **features** command of the [winget](index.md) tool displays a list of the experimental features available with your version of the Windows Package Manager.

Each feature can be turned on individually by enabling the features through [**settings**](settings.md).

You can find the latest up to date information on the [experimental features](../../../Settings.md#experimental-features) web page.

## Usage

`winget features [<options>]`

![features command](images/features.png)

Notice above that the status of each feature is listed.  If the feature is **disabled** you will not be able to use it.  If the feature is **enabled** you will notice that the command will be available to you through winget.

To enabled any disabled features, go to **settings** and enable the feature.

Note: features may be managed by group policy. You can use the **winget --info** command to view any policies in effect on your system.

## Options

| Option       | Description                                                             |
|--------------|-------------------------------------------------------------------------|
| **--format** | Specifies the output format. Supported formats: json, xml, text (default) |

### Output formats

The **features** command supports structured output formats for automated processing.

**JSON format:**
```powershell
winget features --format json
```

The JSON output contains an array of feature objects, each with the following properties:
- `name` - The display name of the feature
- `status` - Either "enabled" or "disabled"
- `property` - The JSON property name used in settings.json
- `link` - Documentation URL for the feature

**XML format:**
```powershell
winget features --format xml
```

The XML output contains the same information as JSON, with each feature represented as an XML element with corresponding child elements for name, status, property, and link.

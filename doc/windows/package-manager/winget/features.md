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

![features command](images/features.png)

Notice above that the status of each feature is listed.  If the feature is **disabled** you will not be able to use it.  If the feature is **enabled** you will notice that the command will be available to you through winget.

To enabled any disabled features, go to **settings** and enable the feature.

Note: features may be managed by group policy. You can use the **winget --info** command to view any policies in effect on your system.

## Options

The following options are available.

| Option  | Description |
|--------------|-------------|
| **--format** | Specifies the output format (json, xml). |
| **-?, --help** | Gets additional help on this command. |

## Output formats

The **features** command supports structured output formats for automation and scripting.

### JSON output

Use `--format json` to output features in JSON format:

```powershell
winget features --format json
```

Output structure:
```json
{
  "features": [
    {
      "name": "Direct MSI Install",
      "status": "enabled",
      "property": "directMSI",
      "link": "https://aka.ms/winget-settings"
    }
  ]
}
```

### XML output

Use `--format xml` to output features in XML format:

```powershell
winget features --format xml
```

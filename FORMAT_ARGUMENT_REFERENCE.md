# WinGet --format Argument Reference

## Overview

The `--format` argument enables structured output (JSON or XML) for several winget commands, making it easier to parse results in scripts and automation tools.

## Supported Commands

- `winget search`
- `winget list`
- `winget show`
- `winget upgrade`
- `winget source list`
- `winget features`

## Syntax

```
winget <command> --format <format>
```

Where `<format>` is one of:
- `json` - Output in JSON format
- `xml` - Output in XML format
- `text` - Default text/table format (optional, this is the default)

Format names are case-insensitive.

## Examples

### Search Command

**JSON Output:**
```bash
winget search vscode --format json
```

Output:
```json
{
  "packages": [
    {
      "name": "Visual Studio Code",
      "id": "Microsoft.VisualStudioCode",
      "version": "1.85.0",
      "match": "",
      "source": "winget"
    }
  ],
  "truncated": false
}
```

**XML Output:**
```bash
winget search vscode --format xml
```

Output:
```xml
<?xml version="1.0" encoding="utf-8"?>
<packages>
  <package>
    <name>Visual Studio Code</name>
    <id>Microsoft.VisualStudioCode</id>
    <version>1.85.0</version>
    <match></match>
    <source>winget</source>
  </package>
  <truncated>false</truncated>
</packages>
```

### List Command

**JSON Output:**
```bash
winget list --format json
```

Output:
```json
{
  "packages": [
    {
      "name": "Visual Studio Code",
      "id": "Microsoft.VisualStudioCode",
      "installedVersion": "1.84.0",
      "availableVersion": "1.85.0",
      "source": "winget"
    }
  ],
  "truncated": false
}
```

**List Upgrades:**
```bash
winget list --upgrade-available --format json
```

Output includes additional fields:
```json
{
  "packages": [...],
  "truncated": false,
  "availableUpgrades": 5,
  "packagesWithUnknownVersionSkipped": 0,
  "packagesWithUserPinsSkipped": 0
}
```

### Source List Command

**JSON Output:**
```bash
winget source list --format json
```

Output:
```json
{
  "sources": [
    {
      "name": "winget",
      "type": "Microsoft.PreIndexed.Package",
      "arg": "https://cdn.winget.microsoft.com/cache",
      "data": "Microsoft.Winget.Source_8wekyb3d8bbwe",
      "identifier": "Microsoft.Winget.Source_8wekyb3d8bbwe",
      "trustLevel": "Trusted",
      "explicit": false,
      "updated": "2024-01-15 10:30:45"
    }
  ]
}
```

### Features Command

**JSON Output:**
```bash
winget features --format json
```

Output:
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

## JSON Schema Structure

### Search/List Results

```json
{
  "packages": [
    {
      "name": "string",
      "id": "string",
      "version": "string",         // For search
      "installedVersion": "string", // For list
      "availableVersion": "string", // For list (if upgrade available)
      "match": "string",
      "source": "string"
    }
  ],
  "truncated": boolean,
  "availableUpgrades": number,    // For list --upgrade-available
  "packagesWithUnknownVersionSkipped": number,
  "packagesWithUserPinsSkipped": number,
  "packagesRequiringExplicitUpgrade": [...],  // Array of packages
  "packagesBlockedByPin": [...]               // Array of packages
}
```

### Source List

```json
{
  "sources": [
    {
      "name": "string",
      "type": "string",
      "arg": "string",
      "data": "string",
      "identifier": "string",
      "trustLevel": "string",
      "explicit": boolean,
      "updated": "string"
    }
  ]
}
```

### Features

```json
{
  "features": [
    {
      "name": "string",
      "status": "enabled|disabled",
      "property": "string",
      "link": "string"
    }
  ]
}
```

## Use Cases

### PowerShell Integration

```powershell
# Get all installed packages as objects
$packages = winget list --format json | ConvertFrom-Json

# Filter for specific packages
$packages.packages | Where-Object { $_.name -like "*Visual Studio*" }

# Count available upgrades
$upgrades = winget list --upgrade-available --format json | ConvertFrom-Json
$upgrades.availableUpgrades
```

### Bash/Shell Scripting

```bash
# Using jq to parse JSON output
winget search nodejs --format json | jq '.packages[] | select(.id | contains("Node"))'

# Get package count
winget list --format json | jq '.packages | length'

# Check if specific package is installed
winget list --id Microsoft.VisualStudioCode --format json | jq '.packages | length'
```

### Python Integration

```python
import json
import subprocess

# Run winget and parse JSON output
result = subprocess.run(
    ['winget', 'list', '--format', 'json'],
    capture_output=True,
    text=True
)

data = json.loads(result.stdout)
for package in data['packages']:
    print(f"{package['name']}: {package['installedVersion']}")
```

## Notes

- The `--format` argument is optional; text format is used by default
- Format argument is case-insensitive (`json`, `JSON`, `Json` all work)
- Structured output maintains all information from text output
- Empty results return valid JSON/XML with empty arrays
- Error messages are still output to stderr in text format for structured outputs

## Error Handling

If an invalid format is specified:
```
Error: Unsupported output format: invalid
Supported formats: json, xml, text
```

## Compatibility

- Minimum winget version: 1.28.0 (with this feature)
- Works with all standard winget filters and arguments
- Compatible with all existing command options

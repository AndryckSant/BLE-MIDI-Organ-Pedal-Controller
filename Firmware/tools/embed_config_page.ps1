$ErrorActionPreference = "Stop"

$pagePath = Join-Path $PSScriptRoot "..\main\src\core\states\config_page.html"
$headerPath = Join-Path $PSScriptRoot "..\main\src\core\states\config_page.h"
$page = [System.IO.File]::ReadAllText((Resolve-Path $pagePath))
$delimiter = "BLEHTML"

if ($page.Contains(")$delimiter`"")) {
    throw "The HTML contains the C++ raw-string delimiter; choose another delimiter."
}

$header = @"
#pragma once
#include <Arduino.h>

static const char kConfigurationPageTemplate[] PROGMEM = R"$delimiter(
$page
)$delimiter";
"@

[System.IO.File]::WriteAllText(
    [System.IO.Path]::GetFullPath($headerPath),
    $header,
    [System.Text.UTF8Encoding]::new($false))

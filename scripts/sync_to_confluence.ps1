# sync_to_confluence.ps1
# Called by the Kiro PostFileSave hook whenever HIGH_LEVEL_DESIGN.md is saved.
# Reads credentials from .confluence.env and pushes the markdown content to
# the configured Confluence page using the REST API v2 (storage format).

param(
    [string]$FilePath   # absolute path to the saved file, injected by the hook
)

# ── 1. Locate the workspace root (directory containing this script's parent) ──
$scriptDir   = Split-Path -Parent $MyInvocation.MyCommand.Path
$workspaceDir = Split-Path -Parent $scriptDir
$envFile      = Join-Path $workspaceDir ".confluence.env"

if (-not (Test-Path $envFile)) {
    Write-Error "Credentials file not found: $envFile"
    exit 1
}

# ── 2. Parse the .env file ────────────────────────────────────────────────────
$config = @{}
Get-Content $envFile | Where-Object { $_ -match '^\s*[^#].*=' } | ForEach-Object {
    $parts = $_ -split '=', 2
    $config[$parts[0].Trim()] = $parts[1].Trim()
}

$baseUrl  = $config['CONFLUENCE_BASE_URL']
$email    = $config['CONFLUENCE_EMAIL']
$token    = $config['CONFLUENCE_API_TOKEN']
$pageId   = $config['CONFLUENCE_PAGE_ID']

if (-not ($baseUrl -and $email -and $token -and $pageId)) {
    Write-Error "One or more required values missing in .confluence.env"
    exit 1
}

# ── 3. Read the markdown file ─────────────────────────────────────────────────
if (-not $FilePath) {
    $FilePath = Join-Path $workspaceDir "HIGH_LEVEL_DESIGN.md"
}

if (-not (Test-Path $FilePath)) {
    Write-Error "Markdown file not found: $FilePath"
    exit 1
}

$markdownContent = Get-Content $FilePath -Raw -Encoding UTF8

# ── 4. Build auth header ──────────────────────────────────────────────────────
$b64Auth = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes("${email}:${token}"))
$headers = @{
    Authorization  = "Basic $b64Auth"
    "Content-Type" = "application/json"
    Accept         = "application/json"
}

# ── 5. Fetch current page version (required for update) ───────────────────────
$pageUrl = "$baseUrl/wiki/rest/api/content/$pageId`?expand=version,title"
try {
    $currentPage = Invoke-RestMethod -Uri $pageUrl -Headers $headers -Method GET
} catch {
    Write-Error "Failed to fetch current page info: $_"
    exit 1
}

$currentVersion = $currentPage.version.number
$pageTitle      = $currentPage.title
$nextVersion    = $currentVersion + 1

# ── 6. Convert markdown to Confluence storage format ─────────────────────────
# Confluence does not render raw markdown natively in storage format.
# We wrap the content in a markdown macro so it renders properly.
$escapedContent = $markdownContent `
    -replace '&',  '&amp;'  `
    -replace '<',  '&lt;'   `
    -replace '>',  '&gt;'

$storageBody = @"
<ac:structured-macro ac:name="markdown" ac:schema-version="1">
  <ac:plain-text-body><![CDATA[$markdownContent]]></ac:plain-text-body>
</ac:structured-macro>
"@

# ── 7. Build the update payload ───────────────────────────────────────────────
$payload = @{
    version = @{ number = $nextVersion }
    title   = $pageTitle
    type    = "page"
    body    = @{
        storage = @{
            value          = $storageBody
            representation = "storage"
        }
    }
} | ConvertTo-Json -Depth 10

# ── 8. PUT the update ─────────────────────────────────────────────────────────
$updateUrl = "$baseUrl/wiki/rest/api/content/$pageId"
try {
    $result = Invoke-RestMethod -Uri $updateUrl -Headers $headers -Method PUT -Body $payload
    $newVer = $result.version.number
    Write-Host "[Confluence Sync] OK — '$pageTitle' updated to version $newVer"
    Write-Host "[Confluence Sync] View: $baseUrl/wiki/pages/$pageId"
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    $errorBody  = $_.ErrorDetails.Message
    Write-Error "[Confluence Sync] FAILED (HTTP $statusCode): $errorBody"
    exit 1
}

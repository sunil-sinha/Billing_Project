# Decode the Confluence tiny link AQAJ to a numeric page ID
$bytes = [System.Convert]::FromBase64String('AQAJ')
$val = 0
foreach ($b in $bytes) { $val = $val * 256 + $b }
Write-Host "Decoded page ID from AQAJ: $val"

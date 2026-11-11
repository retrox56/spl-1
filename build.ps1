New-Item -ItemType Directory -Force build | Out-Null

function Build-Exe($name, $files, $needsCurl) {
    $a = @("-std=c++17", "-Wall", "-Wextra", "-Iinclude") + $files + @("-o", "build\$name.exe")
    if ($needsCurl) { $a += "-lcurl" }
    & g++ @a
    if ($LASTEXITCODE -eq 0) { Write-Host "OK      $name" -ForegroundColor Green }
    else { Write-Host "FAILED  $name" -ForegroundColor Red }
}

Build-Exe "test_url"     @("src\url_utils.cpp", "tests\test_url.cpp") $false
Build-Exe "test_links"   @("src\link_extractor.cpp", "tests\test_links.cpp") $false
Build-Exe "test_http"    @("src\http_client.cpp", "tests\test_http.cpp") $true
Build-Exe "demo_extract" @("src\http_client.cpp", "src\url_utils.cpp", "src\link_extractor.cpp", "tests\demo_extract.cpp") $true
Build-Exe "crawler"      @("main.cpp", "src\http_client.cpp", "src\url_utils.cpp", "src\link_extractor.cpp", "src\crawler.cpp") $true
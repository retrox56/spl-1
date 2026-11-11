# Web Crawler & Website Structure Analyzer

C++17 project (SPL1). Given a website URL, it crawls pages with BFS, extracts and normalizes links, avoids duplicates, and will build a link graph and structure report.

## Modules
| Module | Files | Purpose |
|---|---|---|
| HttpClient | `include/http_client.h`, `src/http_client.cpp` | HTTP/HTTPS GET via libcurl. Redirects are reported, not hidden. |
| url_utils | `include/url_utils.h`, `src/url_utils.cpp` | Parse, resolve relative links, normalize, same-host check. |
| link_extractor | `include/link_extractor.h`, `src/link_extractor.cpp` | Finds `<a href>` links in HTML. |
| crawler, graph, report | planned | BFS crawl, link graph, structure report. |

## Folder layout
```
include/   header files (.h)
src/       implementation files (.cpp)
tests/     small test programs
build/     compiled .exe files (created by you, not committed)
output/    generated reports
```

## Environment setup (Windows 10/11)

### 1. Install MSYS2
Download the installer from https://www.msys2.org and install to the default folder `C:\msys64`.

### 2. Update MSYS2 (important)
Open **MSYS2 UCRT64** from the Start menu. It must be UCRT64, not MSYS or MINGW64. Run:
```bash
pacman -Syu
```
Press `Y`. If it asks you to close the window, close it, reopen **MSYS2 UCRT64**, and run `pacman -Syu` again. Repeat until it says there is nothing to do.

Do this before installing anything. An out-of-date package database causes many `404` errors, because the mirrors have already replaced the old package files.

### 3. Install the compiler and libcurl
In the same MSYS2 UCRT64 terminal:
```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-curl
```
Check that it installed:
```bash
pacman -Qs curl
```

### 4. Put the compiler on the Windows PATH
`pacman` only works inside the MSYS2 terminal. The compiler needs to be reachable from PowerShell and VS Code.

1. Press the Windows key and search for **Edit environment variables for your account**.
2. Select `Path`, click **Edit**, then **New**, and add `C:\msys64\ucrt64\bin`.
3. Click OK and **open a new PowerShell window**.

No admin rights? Use this for the current PowerShell window only:
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
```

### 5. Verify (PowerShell)
```powershell
g++ --version
where.exe g++
Get-ChildItem C:\msys64\ucrt64\include\curl\curl.h
Get-ChildItem C:\msys64\ucrt64\lib\libcurl*
Get-ChildItem C:\msys64\ucrt64\bin\libcurl*.dll
```
`where.exe g++` should show `C:\msys64\ucrt64\bin\g++.exe`.

### Why `pacman` fails in PowerShell
`pacman` lives in `C:\msys64\usr\bin`, which is deliberately not on the Windows PATH. Only the MSYS2 terminals set it up. The `curl.exe` that ships with Windows is a command-line tool only. It has no headers or link libraries, so you can't compile against it.

## Build and run
From the project folder:
```powershell
mkdir build
g++ -std=c++17 -Wall -Wextra -Iinclude src\http_client.cpp tests\test_http.cpp -o build\test_http.exe -lcurl
g++ -std=c++17 -Wall -Wextra -Iinclude src\url_utils.cpp tests\test_url.cpp -o build\test_url.exe
g++ -std=c++17 -Wall -Wextra -Iinclude src\link_extractor.cpp tests\test_links.cpp -o build\test_links.exe
```
Run:
```powershell
.\build\test_http.exe https://example.com
.\build\test_url.exe
.\build\test_links.exe
```

## Troubleshooting
| Problem | Fix |
|---|---|
| `pacman` not recognized | Use the MSYS2 UCRT64 terminal, not PowerShell. |
| `404` errors during install | Run `pacman -Syu` until nothing is left to update, then install again. |
| `curl/curl.h: No such file` | libcurl isn't installed, or the wrong g++ is first on PATH. Check `where.exe g++`. |
| Program exits with no output | A DLL is missing. Make sure `C:\msys64\ucrt64\bin` is on PATH. |
| HTTPS certificate error | Pass a CA bundle to `test_http` as the second argument. |

## Fallback if the lab PC can't install MSYS2
Compile at home, then copy `build\*.exe` plus the DLLs they need into one folder on a USB drive. In the MSYS2 UCRT64 terminal, `ldd build/test_http.exe` lists the DLLs. Copy those from `C:\msys64\ucrt64\bin`. This is a backup plan and I haven't run it on your machine.

## Status
HttpClient, url_utils, and link_extractor are implemented. The crawler engine, graph, and report are in progress.
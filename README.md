# Web Crawler & Website Structure Analyzer

SPL1 project, written in C++17. Given a website URL, it fetches pages, extracts and normalizes links, detects duplicates, and will crawl the site with BFS and report its structure.

## Current status
| Module | Files | Status |
|---|---|---|
| HttpClient | `include/http_client.h`, `src/http_client.cpp` | Done |
| URL utilities | `include/url_utils.h`, `src/url_utils.cpp` | Done |
| Link extractor | `include/link_extractor.h`, `src/link_extractor.cpp` | Done |
| BFS crawler | planned | |
| Link graph and statistics | planned | |
| Report generation | planned | |

- **HttpClient** does HTTP/HTTPS GET with libcurl. Redirects are not followed automatically, so the crawler can record redirect chains.
- **URL utilities** parse URLs, resolve relative links, normalize them (lowercase host, default ports, dot segments, fragments), and compare hosts.
- **Link extractor** reads `<a href>` links from HTML and skips comments, scripts and styles.

## Folder layout
```
include/   header files
src/       implementation files
tests/     test programs and sample_page.html
build/     compiled programs (not committed)
output/    generated reports
docs/      notes
```

## Environment setup (Windows)

### 1. Install MSYS2
Download from https://www.msys2.org and install to `C:\msys64`.

### 2. Update it
Open **MSYS2 UCRT64** from the Start menu (not MSYS or MINGW64):
```bash
pacman -Syu
```
If it asks you to close the window, reopen **MSYS2 UCRT64** and run it again until there is nothing left to update. An out-of-date package database gives `404` errors during installs.

### 3. Install compiler and libcurl
```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-curl
pacman -Qs curl
```

### 4. Add the compiler to PATH
`pacman` only works inside the MSYS2 terminal, but the compiler has to work in PowerShell. Add `C:\msys64\ucrt64\bin` to your user PATH (search Windows for "Edit environment variables for your account"), then open a new PowerShell window. For one window only:
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
```

### 5. Check
```powershell
g++ --version
where.exe g++
Get-ChildItem C:\msys64\ucrt64\include\curl\curl.h
Get-ChildItem C:\msys64\ucrt64\bin\libcurl*.dll
```

## Build and run
From the project folder:
```powershell
mkdir build
g++ -std=c++17 -Wall -Wextra -Iinclude src\url_utils.cpp tests\test_url.cpp -o build\test_url.exe
g++ -std=c++17 -Wall -Wextra -Iinclude src\link_extractor.cpp tests\test_links.cpp -o build\test_links.exe
g++ -std=c++17 -Wall -Wextra -Iinclude src\http_client.cpp tests\test_http.cpp -o build\test_http.exe -lcurl
g++ -std=c++17 -Wall -Wextra -Iinclude src\http_client.cpp src\url_utils.cpp src\link_extractor.cpp tests\demo_extract.cpp -o build\demo_extract.exe -lcurl
```

Tests:
```powershell
.\build\test_url.exe
.\build\test_links.exe
.\build\test_http.exe http://github.com
```

Demo, on a local HTML file or a live site:
```powershell
.\build\demo_extract.exe file tests\sample_page.html http://example.com/blog/index.html
.\build\demo_extract.exe url https://books.toscrape.com
```

## Troubleshooting
| Problem | Fix |
|---|---|
| `pacman` not recognized | Use the MSYS2 UCRT64 terminal, not PowerShell. |
| `404` errors during install | Run `pacman -Syu` until nothing is left, then install again. |
| `curl/curl.h: No such file` | libcurl is not installed, or another g++ is first on PATH. Check `where.exe g++`. |
| Program exits with no output | A DLL is missing. Check that `C:\msys64\ucrt64\bin` is on PATH. |
| HTTPS certificate error | Pass a CA bundle to `test_http` as the second argument. |

## Known limitations
- Subdomains count as different hosts (`docs.site.com` vs `site.com`).
- `/` and `/index.html` are treated as different URLs, and so are `/about` and `/about/`.
- Tracking parameters such as `utm_source` are kept in URLs.
- IPv6 hosts are not supported.
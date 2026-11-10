# Web Crawler & Website Structure Analyzer

A C++17 project (SPL1) that crawls a website, extracts hyperlinks, and analyzes its structure.

## Implemented modules
- `HttpClient`: HTTP/HTTPS fetching with libcurl (redirects are reported, not hidden)
- `url_utils`: URL parsing, relative-link resolution, normalization, same-host checks

## Planned modules
- `link_extractor`: HTML link extraction
- `crawler`: BFS crawling with visited-URL tracking
- `graph`: website link graph and statistics
- `report`: structure report generation

## Build (Windows, MSYS2 UCRT64 g++)
```
g++ -std=c++17 -Wall -Wextra -Iinclude src\http_client.cpp tests\test_http.cpp -o build\test_http.exe -lcurl
g++ -std=c++17 -Wall -Wextra -Iinclude src\url_utils.cpp tests\test_url.cpp -o build\test_url.exe
```
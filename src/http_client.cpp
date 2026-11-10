#include "http_client.h"

#include <curl/curl.h>
#include <algorithm>
#include <cctype>

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

// Context passed to the write callback so it can enforce a size limit.
struct WriteContext {
    HttpResponse* response;
    size_t maxBytes;
};

// libcurl calls this repeatedly as body data arrives.
// Returning anything other than (size * nmemb) tells libcurl to abort.
size_t writeCallback(char* data, size_t size, size_t nmemb, void* userdata) {
    WriteContext* ctx = static_cast<WriteContext*>(userdata);
    size_t bytes = size * nmemb;
    if (ctx->response->body.size() + bytes > ctx->maxBytes) {
        return 0;  // body too large: abort transfer
    }
    ctx->response->body.append(data, bytes);
    return bytes;
}

// libcurl calls this once per header line, e.g. "Content-Type: text/html\r\n".
size_t headerCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    HttpResponse* response = static_cast<HttpResponse*>(userdata);
    size_t bytes = size * nitems;
    std::string line(buffer, bytes);

    // A new "HTTP/..." status line means a new header block: reset our fields.
    if (line.compare(0, 5, "HTTP/") == 0) {
        response->contentType.clear();
        response->location.clear();
        return bytes;
    }

    size_t colon = line.find(':');
    if (colon == std::string::npos) return bytes;

    std::string name = toLower(trim(line.substr(0, colon)));
    std::string value = trim(line.substr(colon + 1));

    if (name == "content-type") response->contentType = value;
    else if (name == "location") response->location = value;

    return bytes;
}

} // namespace

HttpClient::HttpClient()
    : curl_(nullptr),
      timeoutSec_(15),
      userAgent_("SPL1-WebCrawler/0.1"),
      followRedirects_(false),
      maxBodyBytes_(5 * 1024 * 1024) {
    static bool globalInitDone = false;
    if (!globalInitDone) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        globalInitDone = true;
    }
    curl_ = curl_easy_init();
}

HttpClient::~HttpClient() {
    if (curl_) curl_easy_cleanup(static_cast<CURL*>(curl_));
}

HttpResponse HttpClient::get(const std::string& url) {
    HttpResponse response;
    response.url = url;

    if (!curl_) {
        response.errorMessage = "curl handle was not initialized";
        return response;
    }

    CURL* curl = static_cast<CURL*>(curl_);
    curl_easy_reset(curl);  // clear options from any previous request

    WriteContext ctx{&response, maxBodyBytes_};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent_.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSec_);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeoutSec_);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, followRedirects_ ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");  // allow gzip etc. and auto-decode

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);

    if (!caBundlePath_.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAINFO, caBundlePath_.c_str());
    }

    CURLcode code = curl_easy_perform(curl);

    if (code != CURLE_OK) {
        response.success = false;
        response.errorMessage = curl_easy_strerror(code);
        return response;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &response.totalTimeSec);
    response.success = true;
    return response;
}
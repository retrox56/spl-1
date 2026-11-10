#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>

// Everything the crawler needs to know about one HTTP request.
struct HttpResponse {
    bool success = false;        // true if a response was received (any status code)
    long statusCode = 0;         // 200, 301, 404, ... (0 if no response)
    std::string url;             // URL that was requested
    std::string body;            // response body
    std::string contentType;     // e.g. "text/html; charset=UTF-8"
    std::string location;        // value of Location header (for redirects)
    std::string errorMessage;    // filled when success == false
    double totalTimeSec = 0.0;   // time taken for the request
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    // Copying would double-free the curl handle, so forbid it.
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    HttpResponse get(const std::string& url);

    void setTimeout(long seconds)                 { timeoutSec_ = seconds; }
    void setUserAgent(const std::string& ua)      { userAgent_ = ua; }
    void setFollowRedirects(bool follow)          { followRedirects_ = follow; }
    void setCaBundlePath(const std::string& path) { caBundlePath_ = path; }
    void setMaxBodyBytes(size_t bytes)            { maxBodyBytes_ = bytes; }

private:
    void* curl_;                 // CURL* handle (void* keeps curl.h out of this header)
    long timeoutSec_;
    std::string userAgent_;
    bool followRedirects_;
    std::string caBundlePath_;   // optional, helps if HTTPS certificate checks fail
    size_t maxBodyBytes_;
};

#endif
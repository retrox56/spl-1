#include "http_client.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: test_http <url> [ca-bundle-path]\n";
        return 1;
    }

    HttpClient client;
    if (argc >= 3) client.setCaBundlePath(argv[2]);

    HttpResponse r = client.get(argv[1]);

    if (!r.success) {
        std::cout << "Request failed: " << r.errorMessage << "\n";
        return 1;
    }

    std::cout << "Status:       " << r.statusCode << "\n"
              << "Content-Type: " << r.contentType << "\n"
              << "Location:     " << r.location << "\n"
              << "Body size:    " << r.body.size() << " bytes\n"
              << "Time:         " << r.totalTimeSec << " s\n\n"
              << r.body.substr(0, 300) << "\n";
    return 0;
}   
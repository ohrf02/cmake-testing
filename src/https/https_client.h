#ifndef HTTPS_CLIENT_H
#define HTTPS_CLIENT_H

#include <string>

#define NOMINMAX
#include <windows.h>
#undef NOMINMAX

#include <winhttp.h>

namespace https_lib
{

class HttpsClient
{
  public:
    // Constructor that accepts a URL parameter
    explicit HttpsClient(const std::string &url = "");

    // Destructor
    ~HttpsClient();

    // Method to perform GET request and return the response
    std::string get(const std::string &request = "");

    bool SendRequest(const std::wstring &method, const std::wstring &body = L"");

    // Method to set the URL dynamically (optional)
    void setUrl(const std::string &url);

  private:
    // Helper methods
    bool initialize();
    void cleanup();

    // WinHTTP handles
    HINTERNET hSession;
    HINTERNET hConnect;
    HINTERNET hRequest;

    // URL for the HTTP request
    std::string m_url;
};

} // namespace https_lib

#endif // HTTPS_CLIENT_H

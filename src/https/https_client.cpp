#include "https_client.h"
#include <iostream>
#include <stdexcept>
#include <string>

namespace https_lib
{

HttpsClient::HttpsClient(const std::string &url)
    : hSession(nullptr), hConnect(nullptr), hRequest(nullptr), m_url(url)
{
    if (!initialize())
    {
        throw std::runtime_error("Failed to initialize WinHTTP session.");
    }
}

HttpsClient::~HttpsClient() { cleanup(); }

bool HttpsClient::initialize()
{
    // Ensure the URL has the correct protocol and host
    if (m_url.empty())
    {
        m_url = "https://google.com"; // Default URL
    }

    // Find the start of the host part of the URL (remove protocol)
    size_t protocolPos = m_url.find("://");
    if (protocolPos == std::string::npos)
    {
        std::wcerr << L"Invalid URL: Missing protocol." << std::endl;
        return false;
    }

    // Extract the host (after "://")
    std::string host = m_url.substr(protocolPos + 3); // Remove "https://"
    size_t pathPos = host.find('/');
    if (pathPos != std::string::npos)
    {
        host = host.substr(0, pathPos); // Extract the host
    }

    // Initialize WinHTTP session handle
    hSession = WinHttpOpen(L"", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
    {
        DWORD error = GetLastError();
        std::wcerr << L"WinHttpOpen failed with error code: " << error << std::endl;
        return false;
    }

    // Set up the connection with just the host (no protocol)
    hConnect = WinHttpConnect(hSession, std::wstring(host.begin(), host.end()).c_str(),
                              INTERNET_DEFAULT_HTTPS_PORT, 0);

    if (!hConnect)
    {
        DWORD error = GetLastError();
        std::wcerr << L"WinHttpConnect failed with error code: " << error << std::endl;
        return false;
    }

    return true;
}

void HttpsClient::cleanup()
{
    if (hRequest)
    {
        WinHttpCloseHandle(hRequest);
        hRequest = nullptr;
    }
    if (hConnect)
    {
        WinHttpCloseHandle(hConnect);
        hConnect = nullptr;
    }
    if (hSession)
    {
        WinHttpCloseHandle(hSession);
        hSession = nullptr;
    }
}

void HttpsClient::setUrl(const std::string &url) { m_url = url; }

std::string HttpsClient::get(const std::string &request)
{
    std::string response;

    if (!hSession || !hConnect || m_url.empty())
    {
        throw std::runtime_error("Invalid session, connection handle or URL.");
    }

    // Create HTTP GET request using the set URL
    hRequest = WinHttpOpenRequest(
        hConnect, L"GET", std::wstring(request.begin(), request.end()).c_str(), NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest)
    {
        throw std::runtime_error("Failed to open HTTP request.");
    }

    DWORD timeout = 5000; // Timeout in milliseconds (5 seconds)
    WinHttpSetTimeouts(hRequest, timeout, timeout, timeout, timeout);

    // Send the HTTP request
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0,
                            0, 0))
    {
        throw std::runtime_error("Failed to send HTTP request.");
    }

    // Receive the response
    BOOL bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults)
    {
        DWORD dwError = GetLastError();
        std::wcerr << L"Failed to receive HTTP response. Error code: " << dwError << std::endl;
    }

    // Read the response
    DWORD bytesRead = 0;
    DWORD bufferLength = 1024;
    char buffer[1024] = {0};

    while (true)
    {
        if (!WinHttpReadData(hRequest, (LPVOID)buffer, bufferLength, &bytesRead) || bytesRead == 0)
        {
            break;
        }
        response.append(buffer, bytesRead);
    }

    return response;
}

bool HttpsClient::SendRequest(const std::wstring &method, const std::wstring &body)
{
    // Open the request handle
    hRequest =
        WinHttpOpenRequest(hConnect, method.c_str(), (method == L"GET" ? body.c_str() : L"/"), NULL,
                           WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    if (hRequest == nullptr)
    {
        std::wcerr << L"WinHttpOpenRequest failed with error code: " << GetLastError() << std::endl;
        return false;
    }

    // Set the necessary headers (e.g., Content-Type for POST/PUT)
    if (method == L"POST" || method == L"PUT")
    {
        const std::wstring contentType = L"Content-Type: application/json";
        if (!WinHttpAddRequestHeaders(hRequest, contentType.c_str(),
                                      static_cast<DWORD>(contentType.length()),
                                      WINHTTP_ADDREQ_FLAG_ADD))
        {
            std::wcerr << L"Failed to add request headers. Error code: " << GetLastError()
                       << std::endl;
            return false;
        }
    }

    // Send the request with or without a body (for POST/PUT, we send the body)
    BOOL bResults;
    if (method == L"POST" || method == L"PUT")
    {

        if (std::numeric_limits<DWORD>::max() > body.length())
        {
            std::wcerr << L"Got too long body " << std::endl;
            return false;
        }

        DWORD dwSize = static_cast<DWORD>(body.length());
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      (LPVOID)body.c_str(), dwSize, dwSize, 0);
    }
    else
    {
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }

    if (!bResults)
    {
        std::wcerr << L"WinHttpSendRequest failed with error code: " << GetLastError() << std::endl;
        return false;
    }

    // Receive the response
    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults)
    {
        std::wcerr << L"WinHttpReceiveResponse failed with error code: " << GetLastError()
                   << std::endl;
        return false;
    }

    // Read and print the response
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    DWORD dwBufferLength = 0;
    BOOL bRead = FALSE;

    // Allocate memory for the response
    char pszOutBuffer[1024] = {0};
    if (!pszOutBuffer)
    {
        std::wcerr << L"Out of memory" << std::endl;
        return false;
    }
    std::string response;

    while (true)
    {
        dwDownloaded = 0;
        ZeroMemory(pszOutBuffer, 1024);
        if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, sizeof(pszOutBuffer), &dwDownloaded) ||
            dwDownloaded == 0)
        {
            break;
        }
        response.append(pszOutBuffer, dwDownloaded);
    }

    std::cout << " Response: " << response << std::endl;

    // Clean up
    return true;
}

} // namespace https_lib

#include <https/https_client.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <shlobj.h> // For getting user profile path
#include <sqlite3.h>
#include <windows.h>

using json = nlohmann::json;

std::string getChromeHistoryPath()
{
    char localAppData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, localAppData)))
    {
        return std::string(localAppData) + "\\Google\\Chrome\\User Data\\Default\\History";
    }
    return "";
}

void readChromeHistory(const std::string &dbPath)
{
    // Copy the locked history file to a temporary location
    std::string tempPath = dbPath + ".backup";
    CopyFileA(dbPath.c_str(), tempPath.c_str(), FALSE);

    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *query =
        "SELECT url, title, last_visit_time FROM urls ORDER BY last_visit_time DESC LIMIT 100;";

    if (sqlite3_open(tempPath.c_str(), &db) != SQLITE_OK)
    {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        auto url = sqlite3_column_text(stmt, 0);
        auto title = sqlite3_column_text(stmt, 1);
        long long visitTime = sqlite3_column_int64(stmt, 2);

        std::cout << "Title: " << title << "\nURL: " << url << "\nVisit Time: " << visitTime
                  << "\n\n";
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    // Remove the temporary file
    DeleteFileA(tempPath.c_str());
}

int main()
{
    std::string historyPath = getChromeHistoryPath();
    if (historyPath.empty())
    {
        std::cerr << "Could not determine Chrome history path." << std::endl;
        return 1;
    }

    std::cout << "Reading Chrome history from: " << historyPath << std::endl;
    readChromeHistory(historyPath);

    std::string jsonString = R"({
        "name": "John Doe",
        "age": 30,
        "isEmployed": true
    })";

    try
    {
        // Parse the JSON string into a json object
        json jsonData = json::parse(jsonString);

        // Access values
        std::cout << "Name: " << jsonData["name"] << "\n";
        std::cout << "Name: " << jsonData["name"] << "\n";

        std::cout << "Age: " << jsonData["age"] << "\n";
        std::cout << "Employed: " << jsonData["isEmployed"] << "\n";
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }

    try
    {
        // Create HttpsClient instance
        https_lib::HttpsClient client("https://google.com");

        // Perform GET request
        client.SendRequest(L"GET", L"/");
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}

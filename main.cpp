#include <iostream>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <ctime>

using namespace std;

class URLShortener {
    map<string, string> shortToLong;
    map<string, string> longToShort;
    map<string, int> clickCount;
    map<string, time_t> expiryTime;

    int idCounter = 1;

    string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

public:

    //  Base62 Encoding
    string encodeBase62(int num) {
        string result = "";

        while(num > 0) {
            result += characters[num % 62];
            num /= 62;
        }

        reverse(result.begin(), result.end());
        return result;
    }

    //  Shorten URL (with optional custom code)
    void shortenURL(string longURL, string customCode = "") {
        if(longURL.find("http")!=0){
            longURL = "https://"+longURL;
        }
        if(longToShort.find(longURL) != longToShort.end()) {
            cout << "Short URL already exists: short.ly/"
                 << longToShort[longURL] << endl;
            return;
        }

        string shortCode;

        // Custom alias
        if(customCode != "") {
            if(shortToLong.find(customCode) != shortToLong.end()) {
                cout << "Custom code already taken!\n";
                return;
            }
            shortCode = customCode;
        } 
        else {
            shortCode = encodeBase62(idCounter++);
        }

        shortToLong[shortCode] = longURL;
        longToShort[longURL] = shortCode;
        clickCount[shortCode] = 0;

        // Expiry = 1 hour
        time_t now = time(0);
        expiryTime[shortCode] = now + 3600;

        cout << "Short URL: short.ly/" << shortCode << endl;

        saveToFile(); 
    }

    // 🔹 Access URL
    void accessURL(string shortCode) {

        if(shortToLong.find(shortCode) == shortToLong.end()) {
            cout << "Invalid short URL\n";
            return;
        }

        time_t now = time(0);

        if(now > expiryTime[shortCode]) {
            cout << "Link expired!\n";
            string longURL=shortToLong[shortCode];

            shortToLong.erase(shortCode);
            longToShort.erase(longURL);
            clickCount.erase(shortCode);
            expiryTime.erase(shortCode);

            saveToFile();
            return;
        }

        clickCount[shortCode]++;
        string url = shortToLong[shortCode];
        cout << "Redirecting to URL: " << url << endl;

        //  OS-based command
string command;

#ifdef _WIN32
command = "start " + url;
#elif _APPLE_
command = "open " + url;
#else
command = "xdg-open " + url;
#endif

system(command.c_str());

    saveToFile(); 
    }

    // 🔹 Show stats
    void showStats() {
        cout << "\n--- URL Statistics ---\n";

        for(auto &entry : shortToLong) {
            cout << "short.ly/" << entry.first
                 << " -> " << entry.second
                 << " | Clicks: " << clickCount[entry.first]
                 << endl;
        }
    }

    // 🔹 Search
    void searchURL(string longURL) {
        if(longToShort.find(longURL) != longToShort.end()) {
            cout << "Short URL: short.ly/"
                 << longToShort[longURL] << endl;
        } else {
            cout << "URL not found\n";
        }
    }

    // 🔹 Save to file
    void saveToFile() {
        ofstream file("url.txt");

        for(auto &entry : shortToLong) {
            string shortCode = entry.first;
            string longURL = entry.second;

            file << shortCode << " "
                 << longURL << " "
                 << clickCount[shortCode] << " "
                 << expiryTime[shortCode] << endl;
        }

        // save counter
        file << "COUNTER " << idCounter << endl;

        file.close();
    }

    // 🔹 Load from file
    void loadFromFile() {
        ifstream file("url.txt");

        if(!file) return;

        string key;

        while(file >> key) {

            if(key == "COUNTER") {
                file >> idCounter;
            }
            else {
                string shortCode = key;
                string longURL;
                int clicks;
                time_t exp;

                file >> longURL >> clicks >> exp;

                shortToLong[shortCode] = longURL;
                longToShort[longURL] = shortCode;
                clickCount[shortCode] = clicks;
                expiryTime[shortCode] = exp;
            }
        }

        file.close();
    }
};

int main() {
    URLShortener u;

    u.loadFromFile(); 

    int choice;
    string longURL, shortCode, customCode;

    while(true) {
        cout << "\n1. Shorten URL\n2. Access URL\n3. Show Stats\n4. Search URL\n5. Exit\n";
        cin >> choice;

        if(choice == 1) {
            cout << "Enter long URL: ";
            cin >> longURL;

            cout << "Custom short code? (enter '-' for no): ";
            cin >> customCode;

            if(customCode == "-")
                u.shortenURL(longURL);
            else
                u.shortenURL(longURL, customCode);
        }
        else if(choice == 2) {
            cout << "Enter short code: ";
            cin >> shortCode;
            u.accessURL(shortCode);
        }
        else if(choice == 3) {
            u.showStats();
        }
        else if(choice == 4) {
            cout << "Enter long URL to search: ";
            cin >> longURL;
            u.searchURL(longURL);
        }
        else {
            break;
        }
    }

    return 0;
}

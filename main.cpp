#include <iostream>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <ctime>
#include <limits>

using namespace std;

class URLShortener {
    map<string, string> shortToLong;
    map<string, string> longToShort;
    map<string, int> clickCount;
    map<string, time_t> expiryTime;

    int idCounter = 0;

    string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    // Every long URL is normalized the same way before it's used as a
    // map key, in shortenURL() AND searchURL() alike. Previously only
    // shortenURL() added the "https://" prefix, so searching for the
    // exact text you'd just shortened ("google.com") failed to find
    // the entry stored under "https://google.com".
    string normalizeURL(string url) {
        if (url.find("http") != 0) {
            url = "https://" + url;
        }
        return url;
    }

    // Base62 encode, 0-indexed. num == 0 needs a special case since
    // the while loop below never executes for 0, which would
    // otherwise silently produce an empty short code.
    string encodeBase62(int num) {
        if (num == 0) return string(1, characters[0]);

        string result = "";
        while (num > 0) {
            result += characters[num % 62];
            num /= 62;
        }
        reverse(result.begin(), result.end());
        return result;
    }

    // Expiry was previously only ever checked inside accessURL() — a
    // link nobody visits stays in shortToLong forever, so shortenURL()'s
    // duplicate check would treat a dead, expired mapping as still
    // valid and refuse to generate a fresh short code for the same
    // long URL. Call this before any lookup that cares about validity.
    void purgeIfExpired(const string& shortCode) {
        auto it = expiryTime.find(shortCode);
        if (it == expiryTime.end()) return;

        if (time(0) > it->second) {
            string longURL = shortToLong[shortCode];
            shortToLong.erase(shortCode);
            longToShort.erase(longURL);
            clickCount.erase(shortCode);
            expiryTime.erase(shortCode);
        }
    }

public:

    void shortenURL(string longURL, string customCode = "") {
        longURL = normalizeURL(longURL);

        // Sweep every existing entry for this long URL's slot so an
        // expired-but-never-visited mapping doesn't block a fresh one.
        if (longToShort.find(longURL) != longToShort.end()) {
            purgeIfExpired(longToShort[longURL]);
        }

        if (longToShort.find(longURL) != longToShort.end()) {
            cout << "Short URL already exists: short.ly/"
                 << longToShort[longURL] << endl;
            return;
        }

        string shortCode;

        if (customCode != "") {
            if (shortToLong.find(customCode) != shortToLong.end()) {
                purgeIfExpired(customCode);
            }
            if (shortToLong.find(customCode) != shortToLong.end()) {
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

        time_t now = time(0);
        expiryTime[shortCode] = now + 3600; // 1 hour

        cout << "Short URL: short.ly/" << shortCode << endl;

        saveToFile();
    }

    void accessURL(string shortCode) {

        if (shortToLong.find(shortCode) == shortToLong.end()) {
            cout << "Invalid short URL\n";
            return;
        }

        purgeIfExpired(shortCode);

        if (shortToLong.find(shortCode) == shortToLong.end()) {
            cout << "Link expired!\n";
            saveToFile();
            return;
        }

        clickCount[shortCode]++;
        string url = shortToLong[shortCode];

        // A real URL-shortener service returns an HTTP 301/302 redirect
        // response here — the browser, not the server, does the actual
        // navigation. This console app simulates that response instead
        // of spawning a real browser process, which avoids depending on
        // OS-specific commands (xdg-open/open/start) or a GUI browser
        // being available at all, and avoids passing user-supplied
        // input to a shell.
        cout << "Redirecting to: " << url << endl;

        saveToFile();
    }

    void showStats() {
        cout << "\n--- URL Statistics ---\n";

        for (auto &entry : shortToLong) {
            cout << "short.ly/" << entry.first
                 << " -> " << entry.second
                 << " | Clicks: " << clickCount[entry.first]
                 << endl;
        }
    }

    void searchURL(string longURL) {
        longURL = normalizeURL(longURL);

        if (longToShort.find(longURL) != longToShort.end()) {
            purgeIfExpired(longToShort[longURL]);
        }

        if (longToShort.find(longURL) != longToShort.end()) {
            cout << "Short URL: short.ly/"
                 << longToShort[longURL] << endl;
        } else {
            cout << "URL not found\n";
        }
    }

    void saveToFile() {
        ofstream file("url.txt");

        for (auto &entry : shortToLong) {
            string shortCode = entry.first;
            string longURL = entry.second;

            file << shortCode << " "
                 << longURL << " "
                 << clickCount[shortCode] << " "
                 << expiryTime[shortCode] << endl;
        }

        file << "COUNTER " << idCounter << endl;

        file.close();
    }

    void loadFromFile() {
        ifstream file("url.txt");

        if (!file) return;

        string key;

        while (file >> key) {

            if (key == "COUNTER") {
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

    while (true) {
        cout << "\n1. Shorten URL\n2. Access URL\n3. Show Stats\n4. Search URL\n5. Exit\n";

        if (!(cin >> choice)) {
            // Non-numeric input leaves choice unchanged and cin in a
            // fail state. Previously this fell through to the exit
            // branch below (choice keeps its old/garbage value, which
            // never matches 1-4), silently quitting the program on
            // any typo instead of just asking again.
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input — please enter a number from 1 to 5.\n";
            continue;
        }

        if (choice == 1) {
            cout << "Enter long URL: ";
            cin >> longURL;

            cout << "Custom short code? (enter '-' for no): ";
            cin >> customCode;

            if (customCode == "-")
                u.shortenURL(longURL);
            else
                u.shortenURL(longURL, customCode);
        }
        else if (choice == 2) {
            cout << "Enter short code: ";
            cin >> shortCode;
            u.accessURL(shortCode);
        }
        else if (choice == 3) {
            u.showStats();
        }
        else if (choice == 4) {
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

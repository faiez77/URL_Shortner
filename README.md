# 🔗 URL Shortener (C++)

A simple URL Shortener system built using C++ that converts long URLs into short codes, supports custom aliases, tracks clicks, and manages expiry.

---

## 🚀 Features

- 🔹 Shorten long URLs into unique short codes
- 🔹 Custom alias support (user-defined short code)
- 🔹 Automatic expiry of links (default: 1 hour)
- 🔹 Click tracking (analytics)
- 🔹 Search short URL using long URL
- 🔹 Persistent storage using file handling
- 🔹 Auto-delete expired URLs
  
---

## 🛠️ Technologies Used

- C++
- STL (`map`, `string`, `algorithm`)
- File Handling (`fstream`)
- Time functions (`ctime`)

---

## ⚙️ How It Works

1. User enters a long URL
2. System generates a short code (Base62 encoding)
3. Mapping is stored:
   - short → long URL
   - long → short URL
4. Expiry time is assigned (1 hour)
5. On accessing:
   - Redirects to original URL
   - Increases click count
6. Expired links are automatically removed

---

##🔮 Future Improvements:

Convert into Web API (Flask / Node.js)

Add database (MySQL / SQLite)

Build frontend UI (React)

Deploy as live service

Add authentication for users

## ▶️ How to Run

### Compile:
```bash
g++ main.cpp -o shortener

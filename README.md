# URL Shortener (C++)

A console-based URL shortener with custom aliases, click tracking,
link expiry, and file-based persistence — built with the STL, no
external dependencies.

## Features

- Shorten a long URL into a short Base62 code (`short.ly/a`, `/b`, ...)
- Optional custom alias (user-chosen short code)
- Click tracking per short link
- 1-hour link expiry, enforced consistently everywhere a link is
  looked up (not just on access)
- Search by long URL to find its existing short code
- Persistent storage via a flat file (`url.txt`) — state survives
  between runs

## How it works

1. Enter a long URL (a bare domain like `google.com` is automatically
   normalized to `https://google.com`)
2. The system generates a short code via Base62 encoding of a
   sequential counter, or uses your custom alias if you provide one
3. Both directions are stored (`short → long` and `long → short`) so
   lookups work either way
4. Every lookup — access, search, or a new shorten request for an
   already-shortened URL — checks and purges expiry first, so a dead
   link can never block a fresh one or be returned as still valid
5. On access, click count increments and the destination is printed
   (see "Design notes" below for why this doesn't launch a real
   browser)

## Bugs found and fixed

This project went through an actual debugging pass — not just written
once and left, four real, reproducible bugs were found by building
and running it, then fixed and re-verified:

| # | Bug | How it was found | Fix |
|---|---|---|---|
| 1 | First auto-generated short code is `b`, permanently skipping `a` | Ran the program, checked the output | Base62 encoding now 0-indexed with an explicit `num == 0` case |
| 2 | Searching for a URL using the exact text you shortened it with (`google.com`) returned "not found" | Ran the shorten → search flow directly | `searchURL()` now applies the same `https://` normalization as `shortenURL()`, via a shared helper |
| 3 | An expired link that nobody ever accessed would block re-shortening the same URL forever — expiry was only ever checked inside `accessURL()` | Rebuilt with a 1-second test expiry, confirmed the stale mapping blocked a fresh shorten | Added `purgeIfExpired()`, called before every lookup that cares about validity (shorten, search, access) |
| 4 | Unsanitized shell injection: the original code passed the raw URL straight into `system()` to launch a browser | Fed a URL containing `;touch $IFS/tmp/pwned` to the *original* code — confirmed it actually created the file | Removed `system()` and the OS-specific browser-launch code entirely — see Design notes |
| 5 | Typing anything non-numeric at the menu silently exits the whole program instead of showing an error | Ran the program and typed `abc` at the menu | Failed `cin` extraction is now detected, cleared, and re-prompted instead of falling through to the exit branch |

Also fixed: `#elif _APPLE_` (not a real predefined macro — the correct
one is `__APPLE__`, double underscore each side) would have silently
broken macOS builds even before the redirect code was removed.

## Design notes

**Why there's no real browser launch.** The original version used
`system()` with OS-specific commands (`xdg-open`/`open`/`start`) to
actually open the destination URL in a browser. This was removed
entirely rather than sanitized, for two reasons:

- It's what created the shell-injection vulnerability above — removing
  it removes that entire risk class, permanently, rather than trying
  to filter dangerous input around it
- It doesn't match how a real URL shortener works anyway: a backend
  service returns an HTTP redirect response (301/302); the *browser*
  (the client) is what navigates. The console app now prints
  `Redirecting to: <url>`, which is a more accurate simulation of that
  server-side behavior than actually spawning a process

**Complexity.** Every operation (`shortenURL`, `accessURL`,
`searchURL`) is O(log n) via `std::map`. At real-world scale this
would move to `std::unordered_map` for O(1) average-case lookups, plus
a distributed ID-generation scheme instead of a single in-memory
counter, since a single counter doesn't survive multiple server
instances.

## Known limitations

- Single in-memory counter for auto-generated codes means this
  wouldn't work correctly if run as multiple concurrent instances
  sharing one `url.txt`
- No automated tests yet

## Sample run

```
$ g++ main.cpp -o shortener
$ ./shortener

1. Shorten URL
2. Access URL
3. Show Stats
4. Search URL
5. Exit
1
Enter long URL: github.com/faiez77
Custom short code? (enter '-' for no): -
Short URL: short.ly/a

1. Shorten URL
2. Access URL
3. Show Stats
4. Search URL
5. Exit
1
Enter long URL: leetcode.com/problems/two-sum
Custom short code? (enter '-' for no): leet
Short URL: short.ly/leet

1. Shorten URL
2. Access URL
3. Show Stats
4. Search URL
5. Exit
2
Enter short code: a
Redirecting to: https://github.com/faiez77

1. Shorten URL
2. Access URL
3. Show Stats
4. Search URL
5. Exit
3

--- URL Statistics ---
short.ly/a -> https://github.com/faiez77 | Clicks: 1
short.ly/leet -> https://leetcode.com/problems/two-sum | Clicks: 0

1. Shorten URL
2. Access URL
3. Show Stats
4. Search URL
5. Exit
4
Enter long URL to search: github.com/faiez77
Short URL: short.ly/a
```

## How to run

```bash
g++ main.cpp -o shortener
./shortener        # Linux/macOS
.\shortener.exe    # Windows
```

No external dependencies — standard library only (`<map>`, `<string>`,
`<fstream>`, `<ctime>`, `<algorithm>`).

## Future improvements

- Convert into a real web API (Flask / Node.js / a C++ HTTP framework)
- Replace flat-file storage with a real database (SQLite / MySQL)
- Move from `std::map` to `std::unordered_map` for O(1) lookups
- Add automated tests (custom-alias collision, expiry, search
  normalization)
- Web frontend, user authentication, deploy as a live service

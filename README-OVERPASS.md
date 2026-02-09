ADAS CSV Road-Type Tagger
=================================

This small C++ tool reads a CSV file with `Latitude`/`Longitude` columns and queries the OpenStreetMap Overpass API to find the nearby `highway` tag (road type). It writes a new CSV with an added `RoadType` column.

Files added:
- `src/main.cpp` — the program
- `CMakeLists.txt` — build file

Dependencies
- libcurl (for HTTP requests)
- nlohmann/json (single header library or package)
- CMake and a C++17 compiler

macOS (Homebrew) example:

```bash
brew install cmake curl nlohmann-json
mkdir build && cd build
cmake ..
cmake --build . --config Release
./adas_overpass ../adas-events-2026-02-07T16-22-58.csv
```

Linux (Ubuntu) example:

```bash
sudo apt update
sudo apt install build-essential cmake libcurl4-openssl-dev nlohmann-json3-dev
mkdir build && cd build
cmake ..
make -j
./adas_overpass ../adas-events-2026-02-07T16-22-58.csv
```

Notes
- The tool calls `https://overpass-api.de/api/interpreter`. Be mindful of rate-limits — the program sleeps briefly between requests. If you have many rows, consider batching or running during off-peak times.
- If latitude/longitude columns are named differently, the tool looks for `Latitude`/`Longitude` (case-insensitive) or `lat`/`lng`.
- If no nearby highway is found, the `RoadType` cell will be empty.

Usage
- `./adas_overpass input.csv [output.csv]`

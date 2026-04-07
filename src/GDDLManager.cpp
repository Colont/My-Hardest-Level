#include "GDDLManager.hpp"
#include <sstream>
#include <fstream>
#include <chrono>

using namespace geode::prelude;


// 10d010496b2ca71c78224deb7ae92557486a2e8a9371dec03ab706c5aa69cbea


std::vector<GDDLDemon> GDDLManager::demons;
bool GDDLManager::loaded = false;

// Returns the path to the cached CSV file
static std::filesystem::path getCachePath() {
    return Mod::get()->getSaveDir() / "gddl_cache.csv";
}

// Returns the path to the cache timestamp file
static std::filesystem::path getCacheTimePath() {
    return Mod::get()->getSaveDir() / "gddl_cache_time.txt";
}

// Check if cache exists and is less than 24 hours old
static bool isCacheValid() {
    auto cachePath = getCachePath();
    auto timePath = getCacheTimePath();

    if (!std::filesystem::exists(cachePath) || !std::filesystem::exists(timePath)) {
        return false;
    }

    std::ifstream timeFile(timePath);
    long long timestamp;
    if (!(timeFile >> timestamp)) return false;

    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // Cache is valid for 24 hours (86400 seconds)
    return (now - timestamp) < 86400;
}

static void saveCacheTimestamp() {
    auto timePath = getCacheTimePath();
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    std::ofstream timeFile(timePath);
    timeFile << now;
}

// Splits a CSV line correctly, handling quoted fields with commas
static std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                field += '"'; // escaped quote
                i++;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

// Parse CSV string into demons vector
static bool parseCSV(const std::string& str) {
    GDDLManager::demons.clear();

    std::istringstream ss(str);
    std::string line;
    std::getline(ss, line); // skip header

    while (std::getline(ss, line)) {
        auto fields = parseCSVLine(line);
        if (fields.size() < 6) continue;

        GDDLDemon demon;

        // Column 0: ID
        try { demon.id = std::stoi(fields[0]); } catch (...) { continue; }

        // Column 1: Level name
        demon.name = fields[1];

        // Column 5: Tier (skipping creator, song, difficulty)
        try { demon.tier = std::stof(fields[5]); } catch (...) { continue; }

        GDDLManager::demons.push_back(demon);
    }

    GDDLManager::loaded = true;
    return !GDDLManager::demons.empty();
}

void GDDLManager::load(
    TaskHolder<web::WebResponse>& listener,
    std::function<void()> success,
    std::function<void(int)> failure
) {
    // Try loading from cache first
    if (isCacheValid()) {
        auto cachePath = getCachePath();
        std::ifstream cacheFile(cachePath);
        std::string content((std::istreambuf_iterator<char>(cacheFile)),
                             std::istreambuf_iterator<char>());

        if (!content.empty() && parseCSV(content)) {
            log::info("Loaded {} demons from cache", demons.size());
            success();
            return;
        }
        // If cache parse failed, fall through to fetch
    }

    // Cache is missing or stale, fetch from Google Sheets
    log::info("Cache miss or stale, fetching from Google Sheets...");
    listener.spawn(
        web::WebRequest().get("https://docs.google.com/spreadsheets/d/1qKlWKpDkOpU1ZF6V6xGfutDY2NvcA8MNPnsv6GBkKPQ/export?format=csv&sheet=GDDL"),
        [success, failure](web::WebResponse res) {
            if (!res.ok()) {
                failure(res.code());
                return;
            }
            auto str = res.string().unwrapOrDefault();
            if (str.empty()) {
                failure(-999);
                return;
            }

            // Save to cache
            auto cachePath = getCachePath();
            std::ofstream cacheFile(cachePath);
            cacheFile << str;
            cacheFile.close();
            saveCacheTimestamp();

            if (parseCSV(str)) {
                log::info("Fetched and cached {} demons", demons.size());
                success();
            } else {
                failure(-998);
            }
        }
    );
}
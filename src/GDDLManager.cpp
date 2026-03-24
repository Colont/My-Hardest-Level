#include "GDDLManager.hpp"
#include <sstream>
using namespace geode::prelude;

std::vector<GDDLDemon> GDDLManager::demons;
bool GDDLManager::loaded = false;

void GDDLManager::load(
    TaskHolder<web::WebResponse>& listener,
    std::function<void()> success,
    std::function<void(int)> failure
) {
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
            std::istringstream ss(res.string().unwrapOrDefault());
            std::string line;
            std::getline(ss, line); // skip header
            while (std::getline(ss, line)) {
                std::istringstream lineStream(line);
                std::string cell;
                GDDLDemon demon;
                
                // Column 1: ID
                std::getline(lineStream, cell, ',');
                cell.erase(std::remove(cell.begin(), cell.end(), '"'), cell.end());
                try { demon.id = std::stoi(cell); } catch (...) { continue; }
                
                // Column 2: Level name
                std::getline(lineStream, cell, ',');
                cell.erase(std::remove(cell.begin(), cell.end(), '"'), cell.end());
                demon.name = cell;
                
                // Column 3: Creator (skip)
                std::getline(lineStream, cell, ',');
                
                // Column 4: Song name (skip)
                std::getline(lineStream, cell, ',');
                
                // Column 5: Official Difficulty (skip)
                std::getline(lineStream, cell, ',');
                
                // Column 6: Tier
                std::getline(lineStream, cell, ',');
                cell.erase(std::remove(cell.begin(), cell.end(), '"'), cell.end());
                try { demon.tier = std::stof(cell); } catch (...) { continue; }
                
                demons.push_back(demon);
            
            }
            loaded = true;
            success();
        }
    );
}
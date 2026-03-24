#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
using namespace geode::prelude;

// Creates a struct to hold data about demons in GDDL
struct GDDLDemon {
    int id;
    float tier;
    std::string name;
};

namespace GDDLManager {
    // extern is used to make demons load only in GDDLManager.cpp and be accessible in MyHardestLayer.cpp.
    extern std::vector<GDDLDemon> demons;
    extern bool loaded; 
    void load(
        TaskHolder<web::WebResponse>& listener,
        std::function<void()> success,
        std::function<void(int)> failure
    );
    
}   
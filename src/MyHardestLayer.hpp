#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/binding/LevelManagerDelegate.hpp>
using namespace geode::prelude;


// Creates a layer that is shown when the user clicks on a button in the LevelSearchLayer.
class MyHardestLayer : public CCLayer, public LevelManagerDelegate {
    public: 
        static MyHardestLayer* create(); // standard create function for a CCLayer
        static CCScene* scene(); // creates a scene with this layer as the child, used for transitioning to this layer
    protected: 
        bool init() override; // to check for errors during initialization
        void onBack(CCObject*); // called when the user clicks the back button in the top left corner of the search page
        void keyBackClicked() override; // called when user hits esc

        CCLabelBMFont* m_countLabel; // number of demons completed shown in the top right corner of the search page
        GJListLayer* m_list; // the list of demons shown in the middle of the search page
        TaskHolder<web::WebResponse> m_listener; // used to load the demon data from the google sheet, stored as a member variable so that it can be cancelled when the user leaves the page
        LoadingCircle* m_loadingCircle; // shown in the middle of the search page while the demon data is being loaded
        CCMenuItemSpriteExtra* m_leftButton; // used to navigate to the previous page of demons, shown in the bottom left corner of the search page
        CCMenuItemSpriteExtra* m_rightButton; // used to navigate to the next page of demons, shown in the bottom right corner of the search page
        std::vector<std::string> m_levelIDs; // the list of level IDs of the demons that will be shown in the search page, used for pagination
        int m_page = 0; // the current page of demons being shown, used for pagination

        void populateList(); // populates the list with the demons corresponding to the current page, called after the demon data is loaded and when the user navigates to a different page
        void loadLevelsFinished(CCArray* levels, const char* key, int) override; // called when the level data is finished loading, populates the list with the loaded demons
        void loadLevelsFailed(const char* key, int) override; // called when the level data fails to load
        void setupPageInfo(gd::string, const char*) override; // called to update the page information
        void onPrevPage(CCObject*); // called when the user clicks the previous page button
        void onNextPage(CCObject*); // called when the user clicks the next page button

        ~MyHardestLayer() override; // destructor, used to cancel the web request if the user leaves the page before the demon data finishes loading
            
};
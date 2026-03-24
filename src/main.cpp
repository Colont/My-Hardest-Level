#include <Geode/Geode.hpp>
using namespace geode::prelude;
#include <Geode/modify/LevelSearchLayer.hpp>
#include "MyHardestLayer.hpp"

class $modify(MyLevelSearchLayer, LevelSearchLayer) {
    bool init(int searchType) {
        if (!LevelSearchLayer::init(searchType)) {
            return false;
        }

        auto myButtonSprite = CircleButtonSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        myButtonSprite->setScale(0.8f);

        auto myButton = CCMenuItemSpriteExtra::create(
            myButtonSprite,
            this,
            menu_selector(MyLevelSearchLayer::onMyHardest)
        );
        myButton->setID("my-hardest-button"_spr);

        if (auto menu = getChildByID("other-filter-menu")) {
            menu->addChild(myButton);
            menu->updateLayout();
        }

        return true;
    }

    void onMyHardest(CCObject*) {
    CCDirector::get()->pushScene(
        CCTransitionFade::create(0.5f, MyHardestLayer::scene())
    );
    }


};
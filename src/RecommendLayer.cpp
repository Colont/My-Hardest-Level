#include "RecommendLayer.hpp"


RecommendLayer* RecommendLayer::create() {
    auto ret = new RecommendLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool RecommendLayer::init() {
    if (!CCLayer::init()) return false;

    setKeypadEnabled(true);
    // bg info
    

    return true;
};

void RecommendLayer::onBack(CCObject*) {
    CCDirector::sharedDirector()->popScene();
};

void RecommendLayer::keyBackClicked() {
    onBack(nullptr);
}; 
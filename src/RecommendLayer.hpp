#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
using namespace geode::prelude;

class RecommendLayer : public CCLayer{
    public:
        static RecommendLayer* create();
    protected:
        bool init() override;
        void onBack(CCObject*);
        void keyBackClicked() override;
};
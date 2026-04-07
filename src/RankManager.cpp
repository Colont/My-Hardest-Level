#include <Geode/Geode.hpp>
#include <Geode/modify/LevelCell.hpp>
#include "RankManager.hpp"

using namespace geode::prelude;


class $modify(MyLevelCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        if (!RankManager::active) return;
        auto id = fmt::to_string(level->m_levelID.value());
        auto it = RankManager::ranks.find(id);
        if (it == RankManager::ranks.end()) return;

        int rank = it->second;

        auto label = CCLabelBMFont::create(
            fmt::format("#{}", rank).c_str(),
            "chatFont.fnt"
        );
        label->setScale(.75f);
        label->setAnchorPoint({ 0.0f, 0.5f });
        label->setPosition({ 330.0f, 12.0f });
        label->setColor({ 70, 70, 70 });
        label->setZOrder(10);
        label->setID("rank-label"_spr);
        this->addChild(label);
    }
};

#include "MyHardestLayer.hpp"
#include "GDDLManager.hpp"
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/CustomListView.hpp>
using namespace geode::prelude;

MyHardestLayer* MyHardestLayer::create() {
    auto ret = new MyHardestLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene* MyHardestLayer::scene() {
    auto ret = CCScene::create();
    AppDelegate::get()->m_runningScene = ret;
    ret->addChild(MyHardestLayer::create());
    return ret;
}

bool MyHardestLayer::init() {
    if (!CCLayer::init()) return false;

        setKeypadEnabled(true);
        // bg info
        auto screenSize = CCDirector::get()->getWinSize();
        auto bg = CCSprite::create("GJ_gradientBG.png");

        bg->setAnchorPoint({ 0.0f, 0.0f });
        bg->setScaleX((screenSize.width + 10.0f) / bg->getTextureRect().size.width);
        bg->setScaleY((screenSize.height + 10.0f) / bg->getTextureRect().size.height);
        bg->setPosition({ -5.0f, -5.0f });
        bg->setColor({ 30, 60, 150 }); // medium blue
        addChild(bg); // creates child

        // number of demons completed in the top right corner of the search page
        m_countLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_countLabel->setAnchorPoint({ 1.0f, 1.0f });
        m_countLabel->setScale(0.6f);
        m_countLabel->setPosition({ screenSize.width - 7.0f, screenSize.height - 3.0f });
        addChild(m_countLabel);

        // The list of demons in the middle of the search page, currently empty
        m_list = GJListLayer::create(nullptr, "My Hardest", {0, 0, 0, 180}, 356.0f, 220.0f, 0);
        m_list -> setPosition(screenSize / 2.0f - m_list->getContentSize() / 2.0f );
        addChild(m_list);

        // Back button info
        auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        auto backButton = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(MyHardestLayer::onBack));
        backButton->setPosition({ 25.0f, screenSize.height - 25.0f });
        auto menu = CCMenu::create();
        menu->setPosition({ 0.0f, 0.0f });
        

        auto leftSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        m_leftButton = CCMenuItemSpriteExtra::create(leftSprite, 
            this,
             menu_selector(MyHardestLayer::onPrevPage));
        m_leftButton->setVisible(false);
        m_leftButton->setPosition({ 24.0f, screenSize.height / 2.0f });
        


        auto rightSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        rightSprite->setFlipX(true);
        m_rightButton = CCMenuItemSpriteExtra::create(rightSprite, 
            this, 
            menu_selector(MyHardestLayer::onNextPage));
        m_rightButton -> setVisible(false);
        m_rightButton->setPosition({ screenSize.width - 24.0f, screenSize.height / 2.0f });


        menu->addChild(m_leftButton);
        menu->addChild(m_rightButton);
        menu->addChild(backButton);
        addChild(menu);

        // loading circle while GDDL data is being fetched
        m_loadingCircle = LoadingCircle::create();
        m_loadingCircle->setParentLayer(this);
        m_loadingCircle->show();

        GDDLManager::load(m_listener,
        [this]() {
                m_loadingCircle->setVisible(false);
                
                auto glm = GameLevelManager::sharedState();
                auto completedLevels = glm->getCompletedLevels(false);
                
                for (auto level : CCArrayExt<GJGameLevel*>(completedLevels)) {
                    if (level->m_normalPercent == 100 && level->m_demon.value() > 0) {
                        auto id = fmt::to_string(level->m_levelID.value());
                        for (auto& demon : GDDLManager::demons) {
                            if (fmt::to_string(demon.id) == id) {
                                m_levelIDs.push_back(id);
                                break;
                            }
                        }
                    }
                }

                // sort by tier hardest first
                std::sort(m_levelIDs.begin(), m_levelIDs.end(), [](const std::string& a, const std::string& b) {
                    auto findTier = [](const std::string& id) -> float {
                        for (auto& demon : GDDLManager::demons) {
                            if (fmt::to_string(demon.id) == id) return demon.tier;
                        }
                        return 0.0f;
                    };
                    return findTier(a) > findTier(b);
                });

            populateList();
        },
        [this](int code) {
            m_loadingCircle->setVisible(false);
            FLAlertLayer::create(
                "Load Failed",
                fmt::format("Error code: {}", code).c_str(),
                "OK"
            )->show();
        });

    return true;
}

// When user goes back, we want to cancel the web request if it hasn't finished loading yet to avoid potential crashes 
void MyHardestLayer::onBack(CCObject*) {
CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}

// Called when the user hits esc, does the same thing as clicking the back button
void MyHardestLayer::keyBackClicked() {
    onBack(nullptr);
}


// Populates the list with the demons corresponding to the current page
void MyHardestLayer::populateList() {
    auto glm = GameLevelManager::get();
    glm->m_levelManagerDelegate = this;

    std::string query = "";
    auto start = m_page * 10;
    auto end = std::min((int)m_levelIDs.size(), start + 10);
    for (int i = start; i < end; i++) {
        if (!query.empty()) query += ",";
        query += m_levelIDs[i];
    }

    log::info("Query: {}", query);
    log::info("Search type: {}", (int)SearchType::Search);
    
    auto searchObject = GJSearchObject::create(static_cast<SearchType>(10), query);
    log::info("Search query after create: {}", searchObject->m_searchQuery);
    glm->getOnlineLevels(searchObject);
}

// Onces the levels have loaded
void MyHardestLayer::loadLevelsFinished(CCArray* levels, const char*, int) {

    if (auto listView = m_list->m_listView) {
    listView->removeFromParent();
    listView->release();
    }

    auto listView = CustomListView::create(levels, 
        BoomListType::Level, 
        220.0f, 356.0f);

    listView->retain();
    listView->setPosition({ 0, 0.0f });
    m_list->addChild(listView, 6, 9);
    m_list->m_listView = listView;

    m_leftButton->setVisible(m_page > 0);
    m_rightButton->setVisible((m_page + 1) * 10 < (int)m_levelIDs.size());
    m_loadingCircle->setVisible(false);
    m_countLabel->setVisible(true);

}

// Levels don't load
void MyHardestLayer::loadLevelsFailed(const char* key, int code) {
    m_loadingCircle->setVisible(false);
    FLAlertLayer::create(
        "Load Failed",
        fmt::format("key: {}, code: {}", key, code).c_str(),
        "OK"
    )->show();
}

void MyHardestLayer::setupPageInfo(gd::string, const char*) {
    m_countLabel->setString(fmt::format("{} to {} of {}",
    m_page * 10 + 1,
    std::min((int)m_levelIDs.size(), (m_page + 1) * 10),
    m_levelIDs.size()).c_str());

}

void MyHardestLayer::onPrevPage(CCObject*) {
    if (m_page > 0) {
        m_page--;
        populateList();
    }
}

void MyHardestLayer::onNextPage(CCObject*) {
    if ((m_page + 1) * 10 < (int)m_levelIDs.size()) {
        m_page++;
        populateList();
    }
}

MyHardestLayer::~MyHardestLayer() {
    m_listener.cancel();
    auto glm = GameLevelManager::get();
    if (glm->m_levelManagerDelegate == this)
        glm->m_levelManagerDelegate = nullptr;
}
#include "battlescreen.h"
#include "enemyspawner.h"
#include "inventory.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QEvent>
#include <QHash>
#include <QMouseEvent>
#include <algorithm>

// CONSTRUCTOR
BattleScreen::BattleScreen(vector<Character*> partyIn,
                           vector<vector<string>> wavesIn,
                           Inventory* inventoryIn,
                           BattleMode mode, int W, int H,
                           QString backgroundPath,
                           QWidget *parent)
    : QWidget(parent), party(partyIn),
    inventory(inventoryIn),
    mode(mode), currentEnemyIndex(0), W(W), H(H),
    isAnimating(false), combinedIndex(0),
    waves(wavesIn), currentWave(0), totalCoinsEarned(0),
    m_backgroundPath(backgroundPath)
{
    // Spawn wave pertama
    for (const string& name : waves[0]) {
        Enemy* e = EnemySpawner::createEnemy(name);
        if (e) enemies.push_back(e);
    }

    battleSystem = new BattleSystem(party, enemies, *inventory, mode);
    setupUI();
    updatePartyUI();
    buildTurnOrder();
    updateTurnOrder();
    setupConnections();

    selectedEnemyIndex = getFirstAliveEnemyIndex();
    if (selectedEnemyIndex != -1) {
        selectEnemyTarget(selectedEnemyIndex);

        // Update portrait ke character pertama yang giliran
        if (!combinedOrder.empty()) {
            auto& first = combinedOrder[combinedIndex];
            if (first.isParty)
                updatePortraitByName(first.name);
        }

        dialogueText->setText("Choose an action.");

        if (mode == STORY_BATTLE) {
            btnAutoplay->hide();
            btnAutoplay->setEnabled(false);
        } else {
            btnAutoplay->show();
            btnAutoplay->setEnabled(true);
        }
    }

    // Show Battle Guide only for story battle
    static bool battleGuideShown = false;

    if (mode == STORY_BATTLE && !battleGuideShown) {
        battleGuideShown = true;

        QTimer::singleShot(300, this, [this]() {
            showBattleGuidePopup();
        });
    }
}

// ── LOCK / UNLOCK ──
void BattleScreen::lockAllInput() {
    isAnimating = true;
    setButtonsEnabled(false);
}

void BattleScreen::unlockAllInput() {
    isAnimating = false;
    setButtonsEnabled(true);
}

// ── SETUP UI ──
void BattleScreen::setupUI() {
    setFixedSize(W, H);
    setStyleSheet("background-color: #0d0d1a;");

    float sx = W / 800.0f;
    float sy = H / 600.0f;

    auto x  = [&](int v){ return (int)(v * sx); };
    auto y  = [&](int v){ return (int)(v * sy); };
    auto w  = [&](int v){ return (int)(v * sx); };
    auto fs = [&](int v){ return QString("font-size:%1px;").arg((int)(v * sx)); };

    // BACKGROUND
    bgLabel = new QLabel(this);
    bgLabel->setGeometry(0, 0, W, y(460));  // ← y(460) bukan H
    bgLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

    QPixmap bg(m_backgroundPath);
    if (!bg.isNull())
        bgLabel->setPixmap(bg.scaled(W, y(460), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    else
        bgLabel->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
                               "stop:0 #0a0a1a, stop:0.6 #1a1a3a, stop:1 #2a2a1a);");

    QLabel *bgOverlay = new QLabel(this);
    bgOverlay->setGeometry(0, 0, W, y(460));
    bgOverlay->setStyleSheet("background: rgba(0, 0, 0, 120);");
    bgOverlay->show();

    // TOP
    chapterLabel = new QLabel("Chapter 1 — The Forest", this);
    chapterLabel->setGeometry(0, y(8), W, y(20));
    chapterLabel->setAlignment(Qt::AlignCenter);
    chapterLabel->setStyleSheet("color:white; background:none;" + fs(11) + "font-family:'Courier New'; letter-spacing:3px;");

    waveLabel = new QLabel("⚔ Wave 1/" + QString::number(waves.size()), this);
    waveLabel->setGeometry(x(10), y(8), w(100), y(20));
    waveLabel->setStyleSheet("color:white; background:none;" + fs(11) + "font-family:'Courier New';");

    // HERO SPRITES
    int spriteSize = w(65);

    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        characterSprites[i] = new QLabel(this);
        characterSprites[i]->setGeometry(0, 0, spriteSize, spriteSize);
        characterSprites[i]->setAlignment(Qt::AlignCenter);
        characterSprites[i]->setCursor(Qt::PointingHandCursor);
        characterSprites[i]->installEventFilter(this);
        characterSprites[i]->hide();

        characterHpBars[i] = nullptr;
        characterHpText[i] = nullptr;
        characterPfp[i] = nullptr;
        characterNameLabels[i] = nullptr;
        skillCooldownLabels[i] = nullptr;
    }

    // ENEMY SPRITES
    int enemyX[3] = {x(620), x(530), x(620)};
    int enemyY[3] = {y(70),  y(160), y(250)};
    int enemySpriteSize = (int)(80 * W / 800.0f); // enemy

    for (int i = 0; i < 3; i++) {
        enemyNameLabels[i] = new QLabel(this);
        enemyNameLabels[i]->setGeometry(enemyX[i], enemyY[i] - y(20), enemySpriteSize, y(14));
        enemyNameLabels[i]->setStyleSheet(
            "background: rgba(0,0,0,150);"
            "color:#ff8888;" + fs(9) +
            "font-family:'Courier New';"
            "border-radius:3px;"
            "padding: 1px 3px;");
        enemyNameLabels[i]->setAlignment(Qt::AlignCenter);
        enemyNameLabels[i]->setText(i < (int)enemies.size() ?
                                        QString::fromStdString(enemies[i]->getName()) : "?");

        enemyHpBars3[i] = new QProgressBar(this);
        enemyHpBars3[i]->setGeometry(enemyX[i], enemyY[i] - y(8), enemySpriteSize, y(6));
        enemyHpBars3[i]->setRange(0, 100);
        enemyHpBars3[i]->setValue(100);
        enemyHpBars3[i]->setTextVisible(false);
        enemyHpBars3[i]->setStyleSheet(
            "QProgressBar{background:rgba(0,0,0,150);border-radius:3px;}"
            "QProgressBar::chunk{background:#cc4444;border-radius:3px;}");

        enemySprites3[i] = new QLabel(this);
        enemySprites3[i]->setGeometry(enemyX[i], enemyY[i], enemySpriteSize, enemySpriteSize);
        enemySprites3[i]->setAlignment(Qt::AlignCenter);
        enemySprites3[i]->setCursor(Qt::PointingHandCursor);
        enemySprites3[i]->installEventFilter(this);

        if (i < (int)enemies.size()) {
            QString enemyIdlePath = getEnemyIdleSpritePath(enemies[i]->getName());
            QPixmap enemyIdle(enemyIdlePath);
            if (!enemyIdle.isNull()) {
                enemySprites3[i]->setStyleSheet("background:none; border:none;");
                enemySprites3[i]->setPixmap(enemyIdle.scaled(
                    enemySpriteSize, enemySpriteSize,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                enemySprites3[i]->setStyleSheet(
                    "background-color:#2a1a1a; color:#ff8888;"
                    "border-radius:" + QString::number(enemySpriteSize/2) + "px;"
                                                             "border:2px solid #cc4444;"
                                                             "font-family:'Courier New';" + fs(14));
                enemySprites3[i]->setText("E");
            }
            enemySprites3[i]->show();
            enemyNameLabels[i]->show();
            enemyHpBars3[i]->show();
        } else {
            enemySprites3[i]->hide();
            enemyNameLabels[i]->hide();
            enemyHpBars3[i]->hide();
        }

        if (i == 0) {
            enemySprite    = enemySprites3[i];
            enemyHpBar     = enemyHpBars3[i];
            enemyNameLabel = enemyNameLabels[i];
        }
    }

    enemyHpText = new QLabel("", this);
    enemyHpText->hide();

    for (int i = 0; i < MAX_ENEMY_SIZE; i++) {
        int indicatorW = spriteSize;
        int indicatorH = y(14);
        enemyTargetIndicators[i] = new QLabel(this);
        enemyTargetIndicators[i]->setGeometry(
            enemyX[i],
            enemyY[i] + spriteSize - y(5),
            indicatorW,
            indicatorH);
        enemyTargetIndicators[i]->setStyleSheet(
            "background: rgba(255,221,68,60);"
            "border: 2px solid rgba(255,221,68,200);"
            "border-radius:" + QString::number(indicatorH/2) + "px;");
        enemyTargetIndicators[i]->hide();
        enemyTargetIndicators[i]->lower();  // ← di belakang sprite
    }

    // BOTTOM PANEL — solid
    QLabel *bottomPanel = new QLabel(this);
    bottomPanel->setGeometry(0, y(433), W, y(300));
    bottomPanel->setStyleSheet("background:#0d0d1a;");

    // DIALOGUE BOX
    QLabel *dlgBox = new QLabel(this);
    dlgBox->setGeometry(0, y(365), W, y(70));
    dlgBox->setStyleSheet(
        "background-color: rgba(13, 13, 26, 180);"
        "border-top: 2px solid #4a4a8a;"
        "border-bottom: 4px solid #4a4a8a;"
        );

    // PORTRAIT
    dialoguePortrait = new QLabel(this);
    dialoguePortrait->setGeometry(-130, y(-25), x(360), y(580));
    dialoguePortrait->setStyleSheet("background:none; border:none;");
    dialoguePortrait->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

    // DIALOGUE TEXT
    dialogueText = new QLabel("Select an action...", this);
    dialogueText->setGeometry(x(250), y(365), x(350), y(70));
    dialogueText->setStyleSheet("background:none; color:white;" + fs(11) + "font-family:'Courier New';");
    dialogueText->setWordWrap(true);

    // PARTY HP BARS
    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        characterPfp[i] = new QLabel(this);
        characterPfp[i]->setAlignment(Qt::AlignCenter);
        characterPfp[i]->hide();

        characterNameLabels[i] = new QLabel(this);
        characterNameLabels[i]->setStyleSheet(
            "color:white;" + fs(12) +
            "font-family:'Courier New'; font-weight:bold;"
            );
        characterNameLabels[i]->hide();

        skillCooldownLabels[i] = new QLabel("", this);
        skillCooldownLabels[i]->setStyleSheet(
            "color:#ffaa44;" + fs(9) +
            "font-family:'Courier New'; letter-spacing:-1px;"
            );
        skillCooldownLabels[i]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        skillCooldownLabels[i]->hide();

        characterHpBars[i] = new QProgressBar(this);
        characterHpBars[i]->setRange(0, 100);
        characterHpBars[i]->setValue(100);
        characterHpBars[i]->setTextVisible(false);
        characterHpBars[i]->setStyleSheet(
            "QProgressBar{background:#0d0d1a; border:2px solid #44cc44; border-radius:3px;}"
            "QProgressBar::chunk{background:#44cc44; border-radius:2px;}"
            );
        characterHpBars[i]->hide();

        characterHpText[i] = new QLabel(this);
        characterHpText[i]->setStyleSheet(
            "color:#aaaaaa;" + fs(9) +
            "font-family:'Courier New'; font-weight:bold;"
            );
        characterHpText[i]->setText("100/100");
        characterHpText[i]->hide();
    }

    // TURN ORDER PANEL
    QLabel *turnPanel = new QLabel(this);
    turnPanel->setGeometry(x(605), y(380), x(180), y(175));
    turnPanel->setStyleSheet("background:#0d0d1a; border:2px solid #4a4a8a; border-radius:6px;");

    QLabel *turnTitle = new QLabel("TURN ORDER", this);
    turnTitle->setGeometry(x(605), y(390), x(180), y(16));
    turnTitle->setAlignment(Qt::AlignCenter);
    turnTitle->setStyleSheet("background:none; color:#666688;" + fs(9) + "font-family:'Courier New'; letter-spacing:2px;");

    for (int i = 0; i < 4; i++) {
        turnOrderLabels[i] = new QLabel(this);
        turnOrderLabels[i]->setGeometry(x(620), y(415) + i*y(32), x(150), y(24));
        turnOrderLabels[i]->setStyleSheet(i == 0 ?
                                              "background:#1a1a00; border:1px solid #ffdd44; color:#ffdd44;"
                                                  + fs(10) + "font-family:'Courier New'; padding:2px 4px;" :
                                              "background:#0d0d1a; border:1px solid #2a2a4a; color:#888888;"
                                                  + fs(10) + "font-family:'Courier New'; padding:2px 4px;");
        turnOrderLabels[i]->setText(i == 0 ? "▶ —" : "  —");
    }

    // BUTTONS
    QString btnStyle = "QPushButton{background:#1a1a2e;color:white;border:2px solid #4a4a8a;"
                       "font-family:'Courier New';" + fs(13) + "font-weight:bold;}"
                                  "QPushButton:hover{background:#4a4a8a;color:#ffdd44;}";

    int btnW = (1100 - x(5)) / 4;
    int btnY = y(515);
    int btnH = y(40);

    btnFight = new QPushButton("FIGHT", this);
    btnFight->setGeometry(x(25), btnY, btnW, btnH);
    btnSkill = new QPushButton("SKILL", this);
    btnSkill->setGeometry(x(25) + btnW, btnY, btnW, btnH);
    btnItem  = new QPushButton("ITEM",  this);
    btnItem->setGeometry(x(25) + btnW*2, btnY, btnW, btnH);
    btnFlee  = new QPushButton("FLEE",  this);
    btnFlee->setGeometry(x(25) + btnW*3, btnY, btnW, btnH);
    btnAutoplay = new QPushButton("⚡ AUTO: OFF", this);
    btnAutoplay->setGeometry(W - x(180), y(5), x(100), y(30));
    btnAutoplay->setStyleSheet(
        "QPushButton{"
        "background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #1e1e2e,stop:1 #0d0d1a);"
        "color:#8888aa;border:2px solid #333344;border-radius:6px;"
        "font-family:'Courier New';font-size:16px;font-weight:bold;padding:2px 6px;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "stop:0 #2a2a3e,stop:1 #1a1a2e);color:#aaaacc;}");

    btnFight->setStyleSheet(btnStyle);
    btnSkill->setStyleSheet(btnStyle);
    btnItem->setStyleSheet(btnStyle);
    btnFlee->setStyleSheet(btnStyle);

    battleLog = new QListWidget(this);
    battleLog->hide();

    // INVENTORY PANEL
    inventoryPanel = new QLabel(this);
    inventoryPanel->setGeometry(x(250), y(180), x(300), y(180));
    inventoryPanel->setStyleSheet("background:#111122; border:2px solid #ffdd44; border-radius:8px;");
    inventoryPanel->hide();

    for (int i = 0; i < 3; i++) {
        itemButtons[i] = new QPushButton(this);
        itemButtons[i]->setGeometry(x(270), y(200) + i * y(35), x(260), y(30));
        itemButtons[i]->setStyleSheet(
            "QPushButton{background:#1a1a2e; color:white; border:1px solid #4a4a8a;"
            "font-family:'Courier New'; font-size:12px; text-align:left; padding-left:8px;}"
            "QPushButton:hover{background:#4a4a8a; color:#ffdd44;}");
        itemButtons[i]->hide();
    }

    btnCancelInventory = new QPushButton("Cancel", this);
    btnCancelInventory->setGeometry(x(270), y(315), x(260), y(30));
    btnCancelInventory->setStyleSheet(
        "QPushButton{background:#2a1a1a; color:#ff8888; border:1px solid #cc4444;"
        "font-family:'Courier New'; font-size:12px;}"
        "QPushButton:hover{background:#cc4444; color:white;}");
    btnCancelInventory->hide();

    updateTurnOrder();

    // CHECKING STATS PANEL
    statsPanel = new QLabel(this);
    statsPanel->setGeometry(0, 0, x(50), y(80));
    statsPanel->setStyleSheet(
        "background:#0d0d1a;"
        "border:2px solid #ffdd44;"
        "border-radius:6px;"
        "padding:4px;");
    statsPanel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    statsPanel->setWordWrap(true);
    statsPanel->hide();
    statsPanel->raise();

    updatePartyUI();
}

void BattleScreen::updatePortrait(const QString& portraitPath) {
    QPixmap portrait(portraitPath);
    if (!portrait.isNull()) {
        dialoguePortrait->setPixmap(portrait.scaled(
            dialoguePortrait->width(),
            dialoguePortrait->height(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));
    }
}

QString BattleScreen::getPortraitPathByName(const string& name) const {
    static const QHash<QString, QString> portraitMap = {
        {"Ethan",   ":/assets/portraits/ethan_neutral.png"},
        {"MC",      ":/assets/portraits/mc_neutral.PNG"},
        {"Hubert",  ":/assets/portraits/hubert_neutral.PNG"},

        {"Wolf",    ":/assets/portraits/wolf_neutral.PNG"},
        {"Snake",   ":/assets/portraits/snake_neutral.PNG"},
        {"Slime",   ":/assets/portraits/slime_neutral.PNG"},
        {"Gorilla", ":/assets/portraits/gorilla_neutral.PNG"}
    };

    return portraitMap.value(
        QString::fromStdString(name),
        ":/assets/portraits/mc_neutral.PNG"
        );
}

void BattleScreen::updatePortraitByName(const string& name) {
    updatePortrait(getPortraitPathByName(name));
}

QString BattleScreen::getIdleSpritePathByName(const string& name) const
{
    static const QHash<QString, QString> idleMap = {
        {"MC",     ":/assets/sprites/mc_attackidle.png"},
        {"Ethan",  ":/assets/sprites/ethan_attackidle.png"},
        {"Hubert", ":/assets/sprites/hubert_attackidle.png"}
    };

    return idleMap.value(
        QString::fromStdString(name),
        ":/assets/sprites/mc_attackidle.png"
        );
}

QStringList BattleScreen::getBasicAttackFramesByName(const string& name) const
{
    static const QHash<QString, QStringList> attackMap = {
        {"MC", {
                   ":/assets/sprites/mc_ba1.png",
                   ":/assets/sprites/mc_ba2.png",
                   ":/assets/sprites/mc_ba3.png",
                   ":/assets/sprites/mc_ba4.png"
               }},
        {"Ethan", {
                      ":/assets/sprites/ethan_ba1.png",
                      ":/assets/sprites/ethan_ba2.png",
                      ":/assets/sprites/ethan_ba3.png",
                      ":/assets/sprites/ethan_ba4.png"
                  }},
        {"Hubert", {
                       ":/assets/sprites/hubert_ba1.png",
                       ":/assets/sprites/hubert_ba2.png",
                       ":/assets/sprites/hubert_ba3.png",
                       ":/assets/sprites/hubert_ba4.png"
                   }}
    };

    return attackMap.value(
        QString::fromStdString(name),
        attackMap.value("MC")
        );
}

QStringList BattleScreen::getSkillFramesByName(const string& name) const
{
    static const QHash<QString, QStringList> skillMap = {
        {"MC", {
                   ":/assets/sprites/mc_skill1.png",
                   ":/assets/sprites/mc_skill2.png",
                   ":/assets/sprites/mc_skill3.png",
                   ":/assets/sprites/mc_skill4.png",
                   ":/assets/sprites/mc_skill5.png"
               }},
        {"Ethan", {
                      ":/assets/sprites/ethan_skill1.png",
                      ":/assets/sprites/ethan_skill2.png",
                      ":/assets/sprites/ethan_skill3.png",
                      ":/assets/sprites/ethan_skill4.png",
                      ":/assets/sprites/ethan_skill5.png"
                  }},
        {"Hubert", {
                       ":/assets/sprites/hubert_skill1.png",
                       ":/assets/sprites/hubert_skill2.png",
                       ":/assets/sprites/hubert_skill3.png",
                       ":/assets/sprites/hubert_skill4.png",
                       ":/assets/sprites/hubert_skill5.png"
                   }}
    };

    return skillMap.value(
        QString::fromStdString(name),
        skillMap.value("MC")
        );
}

QString BattleScreen::getEnemyIdleSpritePath(const string& name) const {
    static const QHash<QString, QString> idleMap = {
        {"Slime",   ":/assets/sprites/slime_attackidle.png"},
        {"Wolf",    ":/assets/sprites/wolf_attackidle.png"},
        {"Snake",   ":/assets/sprites/snake_attackidle.png"},
        {"Gorilla", ":/assets/sprites/gorilla_attackidle.png"}
    };
    return idleMap.value(QString::fromStdString(name), "");
}

QStringList BattleScreen::getEnemyAttackFrames(const string& name) const {
    QString n = QString::fromStdString(name);
    return {
        ":/assets/sprites/" + n.toLower() + "_ba1.png",
        ":/assets/sprites/" + n.toLower() + "_ba2.png",
        ":/assets/sprites/" + n.toLower() + "_ba3.png",
        ":/assets/sprites/" + n.toLower() + "_ba4.png"
    };
}

// ── SETUP CONNECTIONS ──
void BattleScreen::setupConnections() {
    btnFight->installEventFilter(this);
    btnSkill->installEventFilter(this);
    btnItem->installEventFilter(this);
    btnFlee->installEventFilter(this);

    // FIGHT
    connect(btnFight, &QPushButton::clicked, this, [this]() {
        if (isAnimating) return;
        lockAllInput();

        if (party.empty() || combinedOrder.empty()) { unlockAllInput(); return; }
        if (combinedIndex >= (int)combinedOrder.size()) { unlockAllInput(); return; }

        auto& current = combinedOrder[combinedIndex];
        if (!current.isParty) { unlockAllInput(); return; }
        if (!party[current.index]->isAlive()) { advanceTurn(); return; }

        if (selectedEnemyIndex < 0 || selectedEnemyIndex >= (int)enemies.size() ||
            enemies[selectedEnemyIndex]->isDead()) {
            selectedEnemyIndex = getFirstAliveEnemyIndex();
            if (selectedEnemyIndex == -1) { showBattleEndBanner(true); return; }
        }

        currentEnemyIndex = selectedEnemyIndex;
        selectEnemyTarget(selectedEnemyIndex);
        playAttackAnimation(current.index);
    });

    // SKILL
    connect(btnSkill, &QPushButton::clicked, this, [this]() {
        if (isAnimating) return;
        if (party.empty() || combinedOrder.empty()) return;

        auto& current = combinedOrder[combinedIndex];

        if (!current.isParty || !party[current.index]->isAlive()) return;

        auto& skills = party[current.index]->getSkills();

        if (skills.empty()) {
            dialogueText->setText("No skills available!");
            return;
        }

        // IMPORTANT: check cooldown BEFORE choosing target
        if (!skills[0].isReady()) {
            dialogueText->setText(QString::fromStdString(
                skills[0].getName() + " is still on cooldown!"
                ));
            return;
        }

        if (skills[0].getType() == DAMAGE) {
            if (selectedEnemyIndex < 0 ||
                selectedEnemyIndex >= (int)enemies.size() ||
                enemies[selectedEnemyIndex]->isDead()) {
                selectedEnemyIndex = getFirstAliveEnemyIndex();
                if (selectedEnemyIndex == -1) { showBattleEndBanner(true); return; }
            }

            currentEnemyIndex = selectedEnemyIndex;
            updatePortraitByName(party[current.index]->getName());
            lockAllInput();

            int spriteSize = (int)(65 * W / 800.0f);
            QLabel* sprite = characterSprites[current.index];
            int charIdx = current.index;
            int enemyIdx = currentEnemyIndex;

            QStringList frames = getSkillFramesByName(
                party[charIdx]->getName()
            );

            int* frameIndex = new int(0);
            QTimer* frameTimer = new QTimer(this);

            connect(frameTimer, &QTimer::timeout, this, [=]() {
                if (*frameIndex < (int)frames.size()) {
                    QPixmap px(frames[*frameIndex]);
                    if (!px.isNull())
                        sprite->setPixmap(px.scaled(
                            spriteSize * 2, spriteSize * 2,
                            Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    (*frameIndex)++;
                } else {
                    frameTimer->stop();
                    delete frameIndex;
                    frameTimer->deleteLater();

                    // Restore idle
                    QPixmap idle(getIdleSpritePathByName(
                        party[charIdx]->getName()
                        ));

                    if (!idle.isNull())
                        sprite->setPixmap(idle.scaled(
                            spriteSize * 2, spriteSize * 2,
                            Qt::KeepAspectRatio, Qt::SmoothTransformation));

                    // Damage setelah animasi selesai
                    string log = battleSystem->characterUseSkill(
                        party[charIdx], 0, enemies[enemyIdx], nullptr);

                    dialogueText->setText(QString::fromStdString(log));
                    addBattleLog(QString::fromStdString(log));
                    updateEnemyUI();

                    if (enemyIdx < (int)enemies.size() && enemies[enemyIdx]->isDead()) {
                        QTimer::singleShot(600, this, &BattleScreen::playDefeatAnimation);
                    } else {
                        advanceTurn();
                    }
                }
            });
            frameTimer->start(120);

        } else {
            pendingSkillCharIndex = current.index;
            selectionMode = SelectionMode::SKILL_ALLY;

            setButtonsEnabled(false);

            updateAllyHighlight();
            dialogueText->setText("Click an ally to use skill on.");
        }
    });

    // ITEM
    connect(btnItem, &QPushButton::clicked, this, [this]() {
        if (isAnimating || isInventoryOpen) return;
        if (combinedOrder.empty() || combinedIndex >= (int)combinedOrder.size()) return;

        auto& current = combinedOrder[combinedIndex];
        if (!current.isParty || !party[current.index]->isAlive()) return;

        openInventoryPanel();
    });

    // FLEE
    connect(btnFlee, &QPushButton::clicked, this, [this]() {
        if (isAnimating) return;
        lockAllInput();
        if (battleSystem->attemptFlee()) {
            dialogueText->setText("Successfully fled from battle!");
            QTimer::singleShot(1000, this, [this]() { emit battleFinished(false); });
        } else {
            dialogueText->setText("Can't flee from a story battle!");
            QTimer::singleShot(500, this, [this]() { unlockAllInput(); });
        }
    });

    // AUTOPLAY
    connect(btnAutoplay, &QPushButton::clicked, this, [this]() {
        isAutoplay = !isAutoplay;
        battleSystem->setAutoplay(isAutoplay);

        if (isAutoplay) {
            btnAutoplay->setText("⚡ AUTO: ON");
            btnAutoplay->setStyleSheet(
                "QPushButton{"
                "background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2a5a2a,stop:1 #1a3a1a);"
                "color:#44ff44;border:2px solid #44cc44;border-radius:6px;"
                "font-family:'Courier New';font-size:16px;font-weight:bold;padding:2px 6px;}"
                "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
                "stop:0 #3a7a3a,stop:1 #2a5a2a);color:#88ff88;}");

            if (!combinedOrder.empty() && combinedIndex < (int)combinedOrder.size()) {
                auto& current = combinedOrder[combinedIndex];
                if (current.isParty && party[current.index]->isAlive())
                    QTimer::singleShot(500, this, [this]() { executeAutoTurn(); });
            }
        } else {
            btnAutoplay->setText("⚡ AUTO: OFF");
            btnAutoplay->setStyleSheet(
                "QPushButton{"
                "background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #1e1e2e,stop:1 #0d0d1a);"
                "color:#555566;border:2px solid #333344;border-radius:6px;"
                "font-family:'Courier New';font-size:16px;font-weight:bold;padding:2px 6px;}"
                "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
                "stop:0 #2a2a3e,stop:1 #1a1a2e);color:#8888aa;}");
        }
    });

    // ITEM BUTTONS
    for (int i = 0; i < 3; i++) {
        connect(itemButtons[i], &QPushButton::clicked, this, [this, i]() {
            showPartyTargetChoices(i);
        });
    }

    // CANCEL INVENTORY
    connect(btnCancelInventory, &QPushButton::clicked, this, [this]() {
        selectionMode = SelectionMode::NONE;
        pendingSkillCharIndex = -1;
        selectedItemIndex = -1;

        for (int i = 0; i < 3; i++) {
            resetAllySpriteSize(i);
        }

        updateAllyHighlight();
        closeInventoryPanel();
    });

    // ESC
    QShortcut *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(esc, &QShortcut::activated, [this]() {
        if (window()->isFullScreen()) window()->showNormal();
        else window()->showFullScreen();
    });
}

void BattleScreen::showBattleGuidePopup()
{
    QLabel *overlay = new QLabel(this);
    overlay->setGeometry(0, 0, W, H);
    overlay->setStyleSheet("background: rgba(0, 0, 0, 170);");
    overlay->show();
    overlay->raise();

    QLabel *panel = new QLabel(this);
    panel->setGeometry(W / 2 - 420, H / 2 - 310, 840, 620);
    panel->setStyleSheet(
        "background:#0d0d1a;"
        "border:3px solid #ffdd44;"
        "border-radius:10px;"
        );
    panel->show();
    panel->raise();

    QLabel *title = new QLabel("BATTLE GUIDE", this);
    title->setGeometry(W / 2 - 350, H / 2 - 275, 700, 40);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "background:none;"
        "color:#ffdd44;"
        "font-size:28px;"
        "font-family:'Courier New';"
        "font-weight:bold;"
        "letter-spacing:3px;"
        );
    title->show();
    title->raise();

    QLabel *guideText = new QLabel(this);
    guideText->setGeometry(W / 2 - 380, H / 2 - 210, 760, 440);
    guideText->setWordWrap(true);
    guideText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    guideText->setText(
        "<span style='color:#ffdd44;'>FIGHT</span> — Strike the highlighted enemy with a basic attack.<br><br>"

        "<span style='color:#ffdd44;'>SKILL</span> — Use a special ability. Some skills need time to recharge before they can be used again.<br>"
        "<span style='color:#ffaa44;'>Cooldown</span> is shown with the <span style='color:#ffaa44;'>⏳</span> hourglass symbol. "
        "The number beside it shows how many turns remain before the skill is ready.<br><br>"

        "<span style='color:#ffdd44;'>ITEM</span> — Use potions or revive stones from your inventory to heal or support your party.<br><br>"

        "<span style='color:#ffdd44;'>FLEE</span> — Attempt to escape from battle. Story battles cannot be escaped.<br><br>"

        "<span style='color:#ffdd44;'>TARGET</span> — Click an enemy sprite to choose who you want to attack. "
        "The highlighted enemy is your current target.<br><br>"

        "<span style='color:#ffdd44;'>TURN ORDER</span> — Shows the order of upcoming turns. "
        "Turn order is based on each character's <span style='color:#aaaaff;'>SPD</span> stat.<br><br>"

        "<span style='color:#ffdd44;'>INSPECT</span> — Right-click a character sprite or enemy sprite to view their "
        "<span style='color:#44cc44;'>HP</span>, "
        "<span style='color:#ff8888;'>ATK</span>, "
        "<span style='color:#ffaa44;'>DEF</span>, and "
        "<span style='color:#aaaaff;'>SPD</span> stats.<br><br>"

        "<span style='color:white;'>Defeat all enemies to win.</span>"
        );
    guideText->setStyleSheet(
        "background:none;"
        "color:white;"
        "font-size:15px;"
        "font-family:'Courier New';"
        "line-height:130%;"
        );
    guideText->setTextFormat(Qt::RichText);
    guideText->show();
    guideText->raise();

    QPushButton *btnBegin = new QPushButton("BEGIN", this);
    btnBegin->setGeometry(W / 2 - 90, H / 2 + 210, 180, 42);
    btnBegin->setStyleSheet(
        "QPushButton{"
        "background:#1a1a2e;"
        "color:#ffdd44;"
        "border:2px solid #ffdd44;"
        "border-radius:6px;"
        "font-size:16px;"
        "font-family:'Courier New';"
        "font-weight:bold;"
        "}"
        "QPushButton:hover{"
        "background:#ffdd44;"
        "color:#0d0d1a;"
        "}"
        );
    btnBegin->show();
    btnBegin->raise();

    connect(btnBegin, &QPushButton::clicked, this, [=]() {
        btnBegin->deleteLater();
        guideText->deleteLater();
        title->deleteLater();
        panel->deleteLater();
        overlay->deleteLater();

        unlockAllInput();
    });

    lockAllInput();
}

// ── ADVANCE TURN ──
void BattleScreen::advanceTurn()
{
    if (combinedOrder.empty()) {
        showBattleEndBanner(true);
        return;
    }

    combinedIndex = (combinedIndex + 1) % (int)combinedOrder.size();

    int tries = 0;
    while (tries < (int)combinedOrder.size()) {
        TurnEntry& entry = combinedOrder[combinedIndex];

        if (entry.isParty) {
            if (entry.index < 0 ||
                entry.index >= (int)party.size() ||
                !party[entry.index] ||
                !party[entry.index]->isAlive()) {

                combinedIndex = (combinedIndex + 1) % (int)combinedOrder.size();
                tries++;
                continue;
            }
        } else {
            if (entry.index < 0 ||
                entry.index >= (int)enemies.size() ||
                !enemies[entry.index] ||
                enemies[entry.index]->isDead()) {

                combinedIndex = (combinedIndex + 1) % (int)combinedOrder.size();
                tries++;
                continue;
            }
        }

        break;
    }

    if (tries >= (int)combinedOrder.size()) {
        battleSystem->allCharactersDead() ? showBattleEndBanner(false) : showBattleEndBanner(true);
        return;
    }

    updateTurnOrder();
    TurnEntry& next = combinedOrder[combinedIndex];

    if (!next.isParty) {
        QTimer::singleShot(1000, this, [this]() { playEnemyTurn(); });
    } else {
        QTimer::singleShot(500, this, [this]() {
            unlockAllInput();
            updateSkillCooldowns();

            if (!combinedOrder.empty() && combinedIndex < (int)combinedOrder.size()) {
                auto& next = combinedOrder[combinedIndex];

                dialogueText->setText(QString::fromStdString(next.name) + "'s turn!");
                updatePortraitByName(next.name);
                updateTurnOrder();

                if (isAutoplay && next.isParty) {
                    QTimer::singleShot(500, this, [this]() { executeAutoTurn(); });
                }
            }
        });
    }
}

// ── SKILL COOLDOWN ──
void BattleScreen::updateSkillCooldowns()
{
    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        if (!skillCooldownLabels[i]) continue;

        if (i >= getActivePartySize()) {
            skillCooldownLabels[i]->hide();
            continue;
        }

        skillCooldownLabels[i]->show();

        auto& skills = party[i]->getSkills();

        if (skills.empty() || skills[0].isReady()) {
            skillCooldownLabels[i]->setText("");
        } else {
            skillCooldownLabels[i]->setText(
                "⏳ " + QString::number(skills[0].getCurrentCooldown())
                );
        }
    }
}

// ── CHECKING STATS ──
void BattleScreen::showStatsPanel(int hp, int maxHp, int atk, int def, int spd, QString name, QPoint pos) {
    QString text = QString("<b style='color:#ffdd44;'>%1</b><br>"
                           "<span style='color:#44cc44;'>❤ %2/%3</span><br>"
                           "<span style='color:#ff8888;'>⚔ ATK: %4</span><br>"
                           "<span style='color:#8888ff;'>🛡 DEF: %5</span><br>"
                           "<span style='color:#ffaa44;'>💨 SPD: %6</span>")
                       .arg(name)
                       .arg(hp).arg(maxHp)
                       .arg(atk).arg(def).arg(spd);

    statsPanel->setText(text);
    statsPanel->setTextFormat(Qt::RichText);

    // Posisi panel — pastiin ga keluar layar
    int panelW = (int)(90 * W/800.0f);
    int panelH = (int)(65 * H/600.0f);
    int px = pos.x() - panelW - 5;  // ← kiri sprite
    int py = pos.y();                // ← sejajar atas sprite
    if (px < 0) px = pos.x() + (int)(70 * W/800.0f) + 5;  // fallback ke kanan kalau mepet kiri
    if (py + panelH > H) py = H - panelH;

    statsPanel->setGeometry(px, py, (int)(50 * W/800.0f), (int)(80 * H/600.0f));
    statsPanel->show();
    statsPanel->raise();
}

void BattleScreen::hideStatsPanel() {
    statsPanel->hide();
}

bool BattleScreen::isValidPartyIndex(int index) const
{
    return index >= 0 && index < (int)party.size() && index < MAX_PARTY_SIZE;
}

int BattleScreen::getActivePartySize() const
{
    return std::min((int)party.size(), MAX_PARTY_SIZE);
}

void BattleScreen::hideAllPartySlots()
{
    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        if (characterSprites[i]) characterSprites[i]->hide();
        if (characterPfp[i]) characterPfp[i]->hide();
        if (characterNameLabels[i]) characterNameLabels[i]->hide();
        if (characterHpBars[i]) characterHpBars[i]->hide();
        if (characterHpText[i]) characterHpText[i]->hide();
        if (skillCooldownLabels[i]) skillCooldownLabels[i]->hide();
    }
}

void BattleScreen::positionPartySprites()
{
    float sx = W / 800.0f;
    float sy = H / 600.0f;

    auto x = [&](int v){ return (int)(v * sx); };
    auto y = [&](int v){ return (int)(v * sy); };
    auto w = [&](int v){ return (int)(v * sx); };

    int spriteSize = w(65);
    int count = getActivePartySize();

    // Layout posisi asli 3 karakter
    QPoint topPos    = QPoint(x(280), y(80));
    QPoint middlePos = QPoint(x(370), y(160));
    QPoint bottomPos = QPoint(x(280), y(250));

    QVector<QPoint> pos;

    if (count == 1) {
        pos = { middlePos };
    }
    else if (count == 2) {
        pos = { middlePos, topPos };
    }
    else {
        pos = { middlePos, topPos, bottomPos };
    }

    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        if (!characterSprites[i]) continue;

        if (i < count) {
            characterSprites[i]->setGeometry(
                pos[i].x() - spriteSize / 2,      // offset biar center
                pos[i].y() - spriteSize / 2,      // offset naik
                spriteSize * 2,
                spriteSize * 2
                );
            characterSprites[i]->show();
        } else {
            characterSprites[i]->hide();
        }
    }
}

void BattleScreen::positionPartyHUD()
{
    float sx = W / 800.0f;
    float sy = H / 600.0f;

    auto x = [&](int v){ return (int)(v * sx); };
    auto y = [&](int v){ return (int)(v * sy); };

    int count = getActivePartySize();

    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        if (i >= count) continue;

        // Tiap slot punya posisi tetap
        int slotW = 1100 / 3;
        int baseX = i * slotW + x(25);

        characterPfp[i]->setGeometry(baseX, y(450), x(40), y(50));
        characterNameLabels[i]->setGeometry(baseX + x(50), y(453), x(85), y(18));
        skillCooldownLabels[i]->setGeometry(baseX + x(137), y(453), x(45), y(16));
        characterHpBars[i]->setGeometry(baseX + x(50), y(475), x(85), y(20));
        characterHpText[i]->setGeometry(baseX + x(140), y(475), x(55), y(20));
    }
}

void BattleScreen::updatePartySprites() {
    int spriteSize = (int)(65 * W / 800.0f);

    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        if (!characterSprites[i]) continue;
        if (i >= getActivePartySize()) {
            characterSprites[i]->hide();
            continue;
        }

        QString name = QString::fromStdString(party[i]->getName());

        QString spritePath = getIdleSpritePathByName(party[i]->getName());

        QPixmap spritePixmap(spritePath);
        if (!spritePixmap.isNull()) {
            characterSprites[i]->setStyleSheet("background:none; border:none;");
            characterSprites[i]->setPixmap(spritePixmap.scaled(
                spriteSize * 2, spriteSize * 2,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
        } else {
            // Fallback ke placeholder
            QString characterColors[MAX_PARTY_SIZE] = {
                "#1a4a2a", "#1a3a6a", "#3a2a1a"
            };
            characterSprites[i]->setText(name.left(1));
            characterSprites[i]->setAlignment(Qt::AlignCenter);
            characterSprites[i]->setStyleSheet(
                "background-color:" + characterColors[i] + ";"
                                                           "color:white;"
                                                           "border-radius:" + QString::number(spriteSize / 2) + "px;"
                                                    "border:2px solid #4a4a8a;"
                                                    "font-family:'Courier New'; font-weight:bold;");
        }

        characterSprites[i]->show();
    }

    positionPartySprites();
}

void BattleScreen::updatePartyHUD()
{
    QString defaultPfp = ":/assets/profilepic/mc_pfp.png";
    QString defaultDeadPfp = ":/assets/profilepic/mc_pfp_dead.png";

    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        if (i >= getActivePartySize()) {
            if (characterPfp[i]) characterPfp[i]->hide();
            if (characterNameLabels[i]) characterNameLabels[i]->hide();
            if (characterHpBars[i]) characterHpBars[i]->hide();
            if (characterHpText[i]) characterHpText[i]->hide();
            if (skillCooldownLabels[i]) skillCooldownLabels[i]->hide();
            continue;
        }

        Character* c = party[i];

        if (!c) {
            continue;
        }

        QString name = QString::fromStdString(c->getName());

        // Ambil PFP langsung dari object Character
        QString pfpPath = QString::fromStdString(c->getPfpPath());

        if (!c->isAlive()) {
            pfpPath = QString::fromStdString(c->getDeadPfpPath());
        }

        // Fallback kalau path kosong
        if (pfpPath.isEmpty()) {
            pfpPath = c->isAlive() ? defaultPfp : defaultDeadPfp;
        }

        QPixmap pfpImg(pfpPath);

        if (!pfpImg.isNull()) {
            characterPfp[i]->setPixmap(pfpImg.scaled(
                (int)(50 * W / 800.0f),
                (int)(50 * H / 600.0f),
                Qt::KeepAspectRatio,
                Qt::FastTransformation
                ));
            characterPfp[i]->setText("");
        } else {
            characterPfp[i]->clear();
            characterPfp[i]->setText(name.left(1));
        }

        characterNameLabels[i]->setText(name);

        int pct = 0;
        if (c->getMaxHP() > 0) {
            pct = (int)(100.0 * c->getHP() / c->getMaxHP());
        }

        characterHpBars[i]->setValue(pct);

        characterHpText[i]->setText(
            QString::number(c->getHP()) + "/" + QString::number(c->getMaxHP())
            );

        QString color = pct > 50 ? "#44cc44" : pct > 25 ? "#cccc44" : "#cc4444";

        characterHpBars[i]->setStyleSheet(
            "QProgressBar{background:#2a2a2a;border-radius:4px;}"
            "QProgressBar::chunk{background:" + color + ";border-radius:4px;}"
            );

        characterPfp[i]->show();
        characterNameLabels[i]->show();
        characterHpBars[i]->show();
        characterHpText[i]->show();
        skillCooldownLabels[i]->show();
    }

    positionPartyHUD();
}

void BattleScreen::updatePartyUI()
{
    updatePartySprites();
    updatePartyHUD();
    updateSkillCooldowns();
}

// ── INVENTORY ──
void BattleScreen::openInventoryPanel() {
    isInventoryOpen = true;
    selectedItemIndex = -1;

    setButtonsEnabled(false);

    inventoryPanel->show();
    btnCancelInventory->show();

    showItemChoices();

    // Put inventory UI above sprites
    inventoryPanel->raise();

    for (int i = 0; i < 3; i++) {
        itemButtons[i]->raise();
    }

    btnCancelInventory->raise();

    dialogueText->setText("Choose an item.");
}

void BattleScreen::closeInventoryPanel() {
    isInventoryOpen = false;
    selectedItemIndex = -1;
    inventoryPanel->hide();
    btnCancelInventory->hide();
    for (int i = 0; i < 3; i++) itemButtons[i]->hide();
    setButtonsEnabled(true);
    if (!combinedOrder.empty() && combinedIndex < (int)combinedOrder.size())
        dialogueText->setText(QString::fromStdString(
                                  combinedOrder[combinedIndex].name) + "'s turn!");
}

void BattleScreen::showItemChoices() {
    vector<Item> items = inventory->getItems();

    inventoryPanel->raise();

    for (int i = 0; i < 3; i++) {
        if (i >= (int)items.size()) {
            itemButtons[i]->hide();
            continue;
        }

        bool available = items[i].isOwned && items[i].quantity > 0;

        itemButtons[i]->setText(QString::fromStdString(items[i].name)
                                + " x" + QString::number(items[i].quantity));

        itemButtons[i]->setEnabled(available);

        itemButtons[i]->show();
        itemButtons[i]->raise();

        itemButtons[i]->setStyleSheet(available ?
                                          "QPushButton{background:#1a1a2e; color:white; border:1px solid #4a4a8a;"
                                          "font-family:'Courier New'; font-size:12px; text-align:left; padding-left:8px;}"
                                          "QPushButton:hover{background:#4a4a8a; color:#ffdd44;}" :
                                          "QPushButton{background:#111111; color:#555555; border:1px solid #333333;"
                                          "font-family:'Courier New'; font-size:12px; text-align:left; padding-left:8px;}");
    }

    btnCancelInventory->show();
    btnCancelInventory->raise();

    dialogueText->setText("Choose an item.");
}

void BattleScreen::showPartyTargetChoices(int itemIndex) {
    vector<Item> items = inventory->getItems();

    if (itemIndex < 0 || itemIndex >= (int)items.size()) return;

    if (!items[itemIndex].isOwned || items[itemIndex].quantity <= 0) {
        dialogueText->setText(QString::fromStdString(items[itemIndex].name) + " is not available.");
        return;
    }

    selectedItemIndex = itemIndex;
    selectionMode = SelectionMode::ITEM_ALLY;
    updateAllyHighlight();

    inventoryPanel->hide();

    btnCancelInventory->show();
    btnCancelInventory->raise();

    for (int i = 0; i < 3; i++) {
        itemButtons[i]->hide();
    }

    dialogueText->setText("Click an ally to use item on.");
}

// ── ALLY SELECTION ──
void BattleScreen::updateAllyHighlight()
{
    bool selectingReviveTarget = false;
    if (selectionMode == SelectionMode::ITEM_ALLY && selectedItemIndex >= 0) {
        vector<Item> items = inventory->getItems();
        if (selectedItemIndex < (int)items.size() &&
            items[selectedItemIndex].name == "Revive Stone") {
            selectingReviveTarget = true;
        }
    }

    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        if (!characterSprites[i]) continue;
        if (i >= getActivePartySize()) {
            characterSprites[i]->hide();
            continue;
        }

        bool alive = party[i]->isAlive();
        bool hasSprite = !QPixmap(getIdleSpritePathByName(party[i]->getName())).isNull();
        int r = (int)(32 * W / 800.0f);
        QString colors[MAX_PARTY_SIZE] = {"#1a4a2a", "#1a3a6a", "#3a2a1a"};

        if (selectionMode != SelectionMode::NONE) {
            if (alive || selectingReviveTarget) {
                if (hasSprite) {
                    characterSprites[i]->setStyleSheet("background:none; border:none;");
                    characterSprites[i]->setGraphicsEffect(nullptr);
                } else {
                    characterSprites[i]->setStyleSheet(
                        "background-color:" + (alive ? colors[i] : QString("#555555")) + ";"
                                                                                         "color:white;"
                                                                                         "border-radius:" + QString::number(r) + "px;"
                                               "border:2px solid #4a4a8a;"
                                               "font-family:'Courier New'; font-weight:bold;"
                        );
                }
            } else {
                if (hasSprite) {
                    QGraphicsOpacityEffect *dim = new QGraphicsOpacityEffect(characterSprites[i]);
                    dim->setOpacity(0.4);
                    characterSprites[i]->setGraphicsEffect(dim);
                } else {
                    characterSprites[i]->setStyleSheet(
                        "background-color:#555555;"
                        "color:white;"
                        "border-radius:" + QString::number(r) + "px;"
                                               "border:2px solid #888888;"
                                               "font-family:'Courier New'; font-weight:bold;"
                        );
                }
            }
        } else {
            if (hasSprite) {
                characterSprites[i]->setGraphicsEffect(nullptr);
                if (!alive) {
                    QGraphicsOpacityEffect *dim = new QGraphicsOpacityEffect(characterSprites[i]);
                    dim->setOpacity(0.4);
                    characterSprites[i]->setGraphicsEffect(dim);
                } else {
                    characterSprites[i]->setStyleSheet("background:none; border:none;");
                }
            } else {
                if (!alive) {
                    characterSprites[i]->setStyleSheet(
                        "background-color:#555555;"
                        "color:white;"
                        "border-radius:" + QString::number(r) + "px;"
                                               "border:2px solid #888888;"
                                               "font-family:'Courier New'; font-weight:bold;"
                        );
                } else {
                    characterSprites[i]->setStyleSheet(
                        "background-color:" + colors[i] + ";"
                                                          "color:white;"
                                                          "border-radius:" + QString::number(r) + "px;"
                                               "border:2px solid #4a4a8a;"
                                               "font-family:'Courier New'; font-weight:bold;"
                        );
                }
            }
        }
        characterSprites[i]->show();
    }
}

void BattleScreen::enlargeAllySprite(int index) {
    if (!isValidPartyIndex(index)) return;

    QLabel* sprite = characterSprites[index];

    QRect geo = sprite->geometry();

    // Simpan titik tengah sprite sekarang
    QPoint center = geo.center();

    // Enlarge kecil aja, misalnya +8% dari ukuran sekarang
    float scale = 1.15f;

    int newW = (int)(geo.width() * scale);
    int newH = (int)(geo.height() * scale);

    QRect newGeo(
        center.x() - newW / 2,
        center.y() - newH / 2,
        newW,
        newH
        );

    sprite->setGeometry(newGeo);
    sprite->raise();
}

void BattleScreen::resetAllySpriteSize(int index)
{
    if (!isValidPartyIndex(index)) return;

    positionPartySprites();
}

void BattleScreen::useAbilityOnAlly(int targetIndex) {
    if (targetIndex < 0 || targetIndex >= (int)party.size()) return;

    string log;

    // ── SKILL TO ALLY ──
    if (selectionMode == SelectionMode::SKILL_ALLY) {
        if (pendingSkillCharIndex < 0) return;
        Character* user = party[pendingSkillCharIndex];
        Character* target = party[targetIndex];
        if (!user->isAlive()) {
            selectionMode = SelectionMode::NONE;
            pendingSkillCharIndex = -1;
            updateAllyHighlight();
            advanceTurn();
            return;
        }
        if (!target->isAlive()) {
            dialogueText->setText("Choose a living ally.");
            return;
        }
        lockAllInput();

        // ── SKILL ANIMATION ──
        int spriteSize = (int)(65 * W / 800.0f);
        QLabel* sprite = characterSprites[pendingSkillCharIndex];
        string userName = user->getName();
        QStringList frames = getSkillFramesByName(userName);
        int charIdx = pendingSkillCharIndex;
        int targetIdx = targetIndex;
        int* frameIndex = new int(0);
        QTimer* frameTimer = new QTimer(this);

        connect(frameTimer, &QTimer::timeout, this, [=]() {
            if (*frameIndex < (int)frames.size()) {
                QPixmap px(frames[*frameIndex]);
                if (!px.isNull())
                    sprite->setPixmap(px.scaled(
                        spriteSize * 2, spriteSize * 2,
                        Qt::KeepAspectRatio, Qt::SmoothTransformation));
                (*frameIndex)++;
            } else {
                frameTimer->stop();
                delete frameIndex;
                frameTimer->deleteLater();

                // Restore idle
                QPixmap idle(getIdleSpritePathByName(userName));
                if (!idle.isNull())
                    sprite->setPixmap(idle.scaled(
                        spriteSize * 2, spriteSize * 2,
                        Qt::KeepAspectRatio, Qt::SmoothTransformation));

                // Skill setelah animasi selesai
                string log = battleSystem->characterUseSkill(
                    party[charIdx], 0, nullptr, party[targetIdx]);

                pendingSkillCharIndex = -1;
                selectionMode = SelectionMode::NONE;

                for (int i = 0; i < getActivePartySize(); i++)
                    resetAllySpriteSize(i);

                updateAllyHighlight();
                dialogueText->setText(QString::fromStdString(log));
                addBattleLog(QString::fromStdString(log));
                updatePartyUI();
                updateTurnOrder();
                QTimer::singleShot(700, this, [this]() { advanceTurn(); });
            }
        });
        frameTimer->start(120);
        return;
    }

    // ── ITEM TO ALLY ──
    else if (selectionMode == SelectionMode::ITEM_ALLY) {
        if (selectedItemIndex < 0) return;

        vector<Item> items = inventory->getItems();

        if (selectedItemIndex >= (int)items.size()) return;

        Item item = items[selectedItemIndex];
        bool isRevive = item.name == "Revive Stone";

        if (!item.isOwned || item.quantity <= 0) {
            dialogueText->setText(QString::fromStdString(item.name) + " is not available.");
            return;
        }

        if (isRevive && party[targetIndex]->isAlive()) {
            dialogueText->setText("Revive Stone can only target a defeated ally.");
            return;
        }

        if (!isRevive && !party[targetIndex]->isAlive()) {
            dialogueText->setText("Potion can only target an alive ally.");
            return;
        }

        lockAllInput();

        inventoryPanel->hide();
        btnCancelInventory->hide();

        for (int i = 0; i < 3; i++) {
            itemButtons[i]->hide();
        }

        isInventoryOpen = false;

        log = battleSystem->characterUseItem(
            party[targetIndex],
            selectedItemIndex
            );

        selectedItemIndex = -1;
    }

    else {
        return;
    }

    selectionMode = SelectionMode::NONE;

    for (int i = 0; i < getActivePartySize(); i++) {
        resetAllySpriteSize(i);
    }

    updateAllyHighlight();
    if (log.find("cooldown") != string::npos ||
        log == "Invalid skill." ||
        log == "Invalid item." ||
        log.find("not available") != string::npos ||
        log.find("No ally target") != string::npos ||
        log.empty()) {

        dialogueText->setText(QString::fromStdString(log));
        unlockAllInput();
        return;
    }

    dialogueText->setText(QString::fromStdString(log));
    addBattleLog(QString::fromStdString(log));

    updatePartyUI();

    // Kalau target di-revive, tambah balik ke combinedOrder
    if (party[targetIndex]->isAlive()) {
        // Cek apakah udah ada di combinedOrder
        bool alreadyInOrder = false;
        for (auto& entry : combinedOrder) {
            if (entry.isParty && entry.index == targetIndex) {
                alreadyInOrder = true;
                break;
            }
        }

        if (!alreadyInOrder) {
            combinedOrder.push_back({
                party[targetIndex]->getName(),
                party[targetIndex]->getSpeed(),
                true,
                targetIndex
            });
            // Re-sort
            sort(combinedOrder.begin(), combinedOrder.end(), [](auto& a, auto& b){
                return a.speed > b.speed;
            });
        }

        // Restore pfp
        updatePartyUI();
    }

    updateTurnOrder();

    QTimer::singleShot(700, this, [this]() {
        advanceTurn();
    });
}

void BattleScreen::updateEnemyUI() {
    if (currentEnemyIndex < 0 || currentEnemyIndex >= (int)enemies.size()) return;
    Enemy* e = enemies[currentEnemyIndex];
    enemyHpBar    = enemyHpBars3[currentEnemyIndex];
    enemyNameLabel = enemyNameLabels[currentEnemyIndex];
    int pct = (int)(100.0 * e->getHP() / e->getMaxHP());
    enemyHpBar->setValue(pct);
    enemyHpText->setText(QString::number(e->getHP()) + "/" + QString::number(e->getMaxHP()));
    enemyNameLabel->setText(QString::fromStdString(e->getName()));
}

void BattleScreen::addBattleLog(const QString& entry) {
    battleLog->addItem(entry);
    battleLog->scrollToBottom();
    if (battleLog->count() > 8) delete battleLog->takeItem(0);
}

// ── LOAD NEXT ENEMY ──
void BattleScreen::loadNextEnemy()
{
    if (currentEnemyIndex < 0 || currentEnemyIndex >= (int)enemies.size()) {
        unlockAllInput();
        return;
    }

    int defeatedIndex = currentEnemyIndex;

    if (!enemies[defeatedIndex]) {
        unlockAllInput();
        return;
    }

    int coinDrop = enemies[defeatedIndex]->getCoinDrop();
    totalCoinsEarned += coinDrop;
    inventory->addCoins(coinDrop);

    if (enemySprites3[defeatedIndex]) enemySprites3[defeatedIndex]->hide();
    if (enemyNameLabels[defeatedIndex]) enemyNameLabels[defeatedIndex]->hide();
    if (enemyHpBars3[defeatedIndex]) enemyHpBars3[defeatedIndex]->hide();

    // Remove dead enemies from turn order
    for (int i = (int)combinedOrder.size() - 1; i >= 0; --i) {
        const TurnEntry& e = combinedOrder[i];

        if (!e.isParty &&
            e.index >= 0 &&
            e.index < (int)enemies.size() &&
            enemies[e.index]->isDead()) {

            if (i < combinedIndex) {
                combinedIndex--;
            }

            combinedOrder.erase(combinedOrder.begin() + i);
        }
    }

    if (combinedIndex < 0) {
        combinedIndex = 0;
    }

    if (!combinedOrder.empty()) {
        combinedIndex %= (int)combinedOrder.size();
    }

    selectedEnemyIndex = getFirstAliveEnemyIndex();
    qDebug() << "[DEBUG] selectedEnemyIndex after getFirstAliveEnemyIndex:"
             << selectedEnemyIndex;

    if (selectedEnemyIndex == -1) {
        QTimer::singleShot(1000, this, [this]() {
            loadNextWave();
        });
        return;
    }

    currentEnemyIndex = selectedEnemyIndex;

    if (combinedOrder.empty()) {
        QTimer::singleShot(1000, this, [this]() {
            loadNextWave();
        });
        return;
    }

    enemySprite    = enemySprites3[currentEnemyIndex];
    enemyHpBar     = enemyHpBars3[currentEnemyIndex];
    enemyNameLabel = enemyNameLabels[currentEnemyIndex];

    if (enemySprite) {
        enemySprite->setGraphicsEffect(nullptr);
        enemySprite->setStyleSheet(
            "background-color:#2a1a1a; color:#ff8888;"
            "border-radius:" + QString::number((int)(65 * W / 800.0f) / 2) + "px;"
                                                             "border:2px solid #cc4444; font-family:'Courier New';"
            );
    }

    updateEnemyUI();
    updateEnemyTargetHighlight();
    updateTurnOrder();

    advanceTurn();
}

// ── LOAD NEXT WAVE ──
void BattleScreen::loadNextWave() {
    currentWave++;

    if (currentWave >= (int)waves.size()) {
        showBattleEndBanner(true);
        return;
    }

    // 1. Clear enemies lama
    for (auto e : enemies) delete e;
    enemies.clear();

    // 2. Spawn enemies wave baru
    for (const string& name : waves[currentWave]) {
        Enemy* e = EnemySpawner::createEnemy(name);
        if (e) enemies.push_back(e);
    }

    // 3. Recreate BattleSystem
    if (battleSystem) delete battleSystem;
    battleSystem = new BattleSystem(party, enemies, *inventory, mode);

    // 4. Reset UI enemies
    for (int i = 0; i < 3; i++) {
        if (i < (int)enemies.size()) {
            int enemySpriteSize = (int)(80 * W / 800.0f);
            enemyNameLabels[i]->setText(QString::fromStdString(enemies[i]->getName()));
            enemyHpBars3[i]->setValue(100);
            enemySprites3[i]->setGraphicsEffect(nullptr);

            QString idlePath = getEnemyIdleSpritePath(enemies[i]->getName());
            QPixmap idle(idlePath);
            if (!idle.isNull()) {
                enemySprites3[i]->setStyleSheet("background:none; border:none;");
                enemySprites3[i]->setPixmap(idle.scaled(
                    enemySpriteSize, enemySpriteSize,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
                enemySprites3[i]->setText("");
            } else {
                enemySprites3[i]->setStyleSheet(
                    "background-color:#2a1a1a; color:#ff8888;"
                    "border-radius:" + QString::number((int)(65 * W/800.0f)/2) + "px;"
                                                                     "border:2px solid #cc4444;"
                                                                     "font-family:'Courier New'; font-size:" +
                    QString::number((int)(14 * W/800.0f)) + "px;");
                enemySprites3[i]->setText("E");
            }

            enemySprites3[i]->show();
            enemyNameLabels[i]->show();
            enemyHpBars3[i]->show();
        } else {
            enemySprites3[i]->hide();
            enemyNameLabels[i]->hide();
            enemyHpBars3[i]->hide();
        }
    }

    // 5. Reset index dan pointer
    currentEnemyIndex = 0;
    enemySprite    = enemySprites3[0];
    enemyHpBar     = enemyHpBars3[0];
    enemyNameLabel = enemyNameLabels[0];

    // 6. Update wave label
    waveLabel->setText("👾 Wave " + QString::number(currentWave + 1) +
                       "/" + QString::number(waves.size()));

    // 7. Rebuild turn order
    buildTurnOrder();
    updateTurnOrder();
    updateEnemyUI();
    updateEnemyTargetHighlight();

    selectedEnemyIndex = getFirstAliveEnemyIndex();
    if (selectedEnemyIndex != -1) selectEnemyTarget(selectedEnemyIndex);

    dialogueText->setText("Wave " + QString::number(currentWave + 1) + " begins!");

    // 8. Cek siapa giliran pertama
    auto& first = combinedOrder[combinedIndex];
    if (!first.isParty) {
        QTimer::singleShot(1000, this, [this]() { playEnemyTurn(); });
    } else {
        unlockAllInput();
        updatePortraitByName(first.name);

        // Kalau autoplay ON, trigger auto turn setelah wave baru
        if (isAutoplay) {
            QTimer::singleShot(1000, this, [this]() { executeAutoTurn(); });
        }
    }
}

// ── ANIMATIONS ──
void BattleScreen::playAttackAnimation(int characterIndex)
{
    if (characterIndex < 0 || characterIndex >= (int)party.size()) { unlockAllInput(); return; }
    if (currentEnemyIndex < 0 || currentEnemyIndex >= (int)enemies.size()) { unlockAllInput(); return; }
    if (!party[characterIndex] || !enemies[currentEnemyIndex]) { unlockAllInput(); return; }

    updatePortraitByName(party[characterIndex]->getName());

    QLabel* sprite = characterSprites[characterIndex];
    if (!sprite) { unlockAllInput(); return; }

    int spriteSize = (int)(65 * W / 800.0f);
    QStringList frames = getBasicAttackFramesByName(
        party[characterIndex]->getName()
    );

    int* frameIndex = new int(0);
    QTimer* frameTimer = new QTimer(this);

    connect(frameTimer, &QTimer::timeout, this, [=]() {
        if (*frameIndex < (int)frames.size()) {
            QPixmap px(frames[*frameIndex]);
            if (!px.isNull())
                sprite->setPixmap(px.scaled(
                    spriteSize * 2, spriteSize * 2,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
            (*frameIndex)++;
        } else {
            frameTimer->stop();
            delete frameIndex;
            frameTimer->deleteLater();

            // Restore idle
            QPixmap idle(getIdleSpritePathByName(
                party[characterIndex]->getName()
                ));
            if (!idle.isNull())
                sprite->setPixmap(idle.scaled(
                    spriteSize * 2, spriteSize * 2,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));

            // ── DAMAGE setelah animasi selesai ──
            if (characterIndex < 0 || characterIndex >= (int)party.size()) { unlockAllInput(); return; }
            if (currentEnemyIndex < 0 || currentEnemyIndex >= (int)enemies.size()) { unlockAllInput(); return; }

            party[characterIndex]->tickSkills();
            updateSkillCooldowns();

            string log = battleSystem->characterAttack(
                party[characterIndex], enemies[currentEnemyIndex]);

            if (enemySprite) {
                playShakeAnimation(enemySprite);
            }

            updateEnemyUI();
            addBattleLog(QString::fromStdString(log));
            dialogueText->setText(QString::fromStdString(log));

            if (currentEnemyIndex >= 0 &&
                currentEnemyIndex < (int)enemies.size() &&
                enemies[currentEnemyIndex]->isDead()) {
                QTimer::singleShot(600, this, [this]() { playDefeatAnimation(); });
            } else {
                advanceTurn();
            }
        }
    });
    frameTimer->start(120);
}

void BattleScreen::playEnemyTurn() {
    lockAllInput();

    if (combinedOrder.empty() || combinedIndex >= (int)combinedOrder.size()) {
        unlockAllInput(); return;
    }

    auto& current = combinedOrder[combinedIndex];

    if (current.isParty) { unlockAllInput(); return; }

    if (current.index < 0 || current.index >= (int)enemies.size()) {
        unlockAllInput(); return;
    }

    Enemy* enemy = enemies[current.index];
    if (!enemy) { unlockAllInput(); return; }

    if (enemy->isDead()) {
        unlockAllInput();
        advanceTurn();
        return;
    }

    currentEnemyIndex = current.index;
    enemySprite    = enemySprites3[currentEnemyIndex];
    enemyHpBar     = enemyHpBars3[currentEnemyIndex];
    enemyNameLabel = enemyNameLabels[currentEnemyIndex];

    updatePortraitByName(current.name);

    // ── ENEMY ATTACK ANIMATION ──
    string enemyName = enemy->getName();
    QStringList enemyFrames = getEnemyAttackFrames(enemyName);
    QLabel* eSprite = enemySprites3[current.index];
    int spriteSize = (int)(80 * W / 800.0f);

    int* frameIndex = new int(0);
    QTimer* frameTimer = new QTimer(this);

    connect(frameTimer, &QTimer::timeout, this, [=]() {
        if (*frameIndex < (int)enemyFrames.size()) {
            QPixmap px(enemyFrames[*frameIndex]);
            if (!px.isNull())
                eSprite->setPixmap(px.scaled(
                    spriteSize, spriteSize,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
            (*frameIndex)++;
        } else {
            frameTimer->stop();
            delete frameIndex;
            frameTimer->deleteLater();

            // Restore idle
            QPixmap idle(getEnemyIdleSpritePath(enemyName));
            if (!idle.isNull())
                eSprite->setPixmap(idle.scaled(
                    spriteSize, spriteSize,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));

            // ── ACTUAL ENEMY TURN LOGIC ──
            if (!enemy || current.index >= (int)enemies.size()) {
                updatePartyUI();
                updateTurnOrder();
                QTimer::singleShot(1000, this, [this]() { advanceTurn(); });
                return;
            }

            // Simpan HP sebelum
            vector<int> hpBefore;
            for (auto c : party) hpBefore.push_back(c->getHP());

            int enemyHpBefore = enemy->getHP();
            battleSystem->enemyTurn(enemy);
            int enemyHpAfter = enemy->getHP();
            int healAmount = enemyHpAfter - enemyHpBefore;

            updateEnemyUI();

            // Floating heal
            if (healAmount > 0) {
                QLabel *floatingHeal = new QLabel("💚 +" + QString::number(healAmount), this);
                floatingHeal->setGeometry(eSprite->x(), eSprite->y() - 20, 100, 20);
                floatingHeal->setStyleSheet("color:#44cc44; font-size:20px; font-family:'Courier New'; font-weight:bold;");
                floatingHeal->show();
                QPropertyAnimation *floatUp = new QPropertyAnimation(floatingHeal, "pos");
                floatUp->setDuration(800);
                floatUp->setStartValue(floatingHeal->pos());
                floatUp->setEndValue(floatingHeal->pos() + QPoint(0, -40));
                floatUp->start();
                QGraphicsOpacityEffect *healEffect = new QGraphicsOpacityEffect(floatingHeal);
                floatingHeal->setGraphicsEffect(healEffect);
                QPropertyAnimation *fade = new QPropertyAnimation(healEffect, "opacity");
                fade->setDuration(800);
                fade->setStartValue(1.0);
                fade->setEndValue(0.0);
                fade->start();
                connect(fade, &QPropertyAnimation::finished, [=]() { floatingHeal->deleteLater(); });
            }

            updateSkillCooldowns();

            string fullLog = battleSystem->getBattleLog();
            size_t lastNewline = fullLog.rfind('\n', fullLog.size() - 2);
            string lastLine = (lastNewline != string::npos) ?
                                  fullLog.substr(lastNewline + 1) : fullLog;
            while (!lastLine.empty() && lastLine.back() == '\n') lastLine.pop_back();
            dialogueText->setText(QString::fromStdString(lastLine));

            // Shake hanya character yang kena
            for (int i = 0; i < (int)party.size() && i < MAX_PARTY_SIZE; i++) {
                if (party[i]->getHP() < hpBefore[i]) {
                    if (characterSprites[i] && characterSprites[i]->isVisible())
                        playShakeAnimation(characterSprites[i]);
                }
            }

            updatePartyUI();
            updateTurnOrder();

            if (battleSystem->allCharactersDead()) { showBattleEndBanner(false); return; }
            QTimer::singleShot(1000, this, [this]() { advanceTurn(); });
        }
    });
    frameTimer->start(120);
}

void BattleScreen::playShakeAnimation(QLabel* sprite) {
    QPropertyAnimation *shake = new QPropertyAnimation(sprite, "pos");
    shake->setDuration(300);
    QPoint origin = sprite->pos();
    shake->setKeyValueAt(0,    origin);
    shake->setKeyValueAt(0.25, origin + QPoint(-8, 0));
    shake->setKeyValueAt(0.5,  origin + QPoint(8, 0));
    shake->setKeyValueAt(0.75, origin + QPoint(-4, 0));
    shake->setKeyValueAt(1,    origin);
    shake->start();
}

void BattleScreen::playDefeatAnimation()
{
    if (currentEnemyIndex < 0 || currentEnemyIndex >= (int)enemies.size()) {
        qDebug() << "[Battle] Invalid currentEnemyIndex in playDefeatAnimation:"
                 << currentEnemyIndex;
        unlockAllInput();
        return;
    }

    if (!enemies[currentEnemyIndex]) {
        qDebug() << "[Battle] Null enemy in playDefeatAnimation";
        unlockAllInput();
        return;
    }

    if (!enemySprite) {
        qDebug() << "[Battle] Null enemySprite in playDefeatAnimation";
        unlockAllInput();
        return;
    }

    int defeatedIndex = currentEnemyIndex;
    int coinDrop = enemies[defeatedIndex]->getCoinDrop();
    QLabel *floatingCoins = new QLabel("🪙 +" + QString::number(coinDrop), this);
    floatingCoins->setGeometry(enemySprite->x(), enemySprite->y() - 20, 120, 20);
    floatingCoins->setStyleSheet("color:#ffdd44; background: none; font-size:20px; font-family:'Courier New'; font-weight:bold;");
    floatingCoins->show();

    // Float ke atas lalu fade
    QPropertyAnimation *floatUp = new QPropertyAnimation(floatingCoins, "pos");
    floatUp->setDuration(800);
    floatUp->setStartValue(floatingCoins->pos());
    floatUp->setEndValue(floatingCoins->pos() + QPoint(0, -40));
    floatUp->start();

    QGraphicsOpacityEffect *effect2 = new QGraphicsOpacityEffect(floatingCoins);
    floatingCoins->setGraphicsEffect(effect2);
    QPropertyAnimation *fadeCoins = new QPropertyAnimation(effect2, "opacity");
    fadeCoins->setDuration(800);
    fadeCoins->setStartValue(1.0);
    fadeCoins->setEndValue(0.0);
    fadeCoins->start();

    connect(fadeCoins, &QPropertyAnimation::finished, [=]() {
        floatingCoins->deleteLater();
    });

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(enemySprite);
    enemySprite->setGraphicsEffect(effect);
    QPropertyAnimation *fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(500);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->start();
    enemyNameLabels[defeatedIndex]->hide();
    enemyHpBars3[defeatedIndex]->hide();
    addBattleLog(QString::fromStdString(enemies[defeatedIndex]->getName()) + " defeated!");
    connect(fadeOut, &QPropertyAnimation::finished, this, [this, defeatedIndex]() {
        currentEnemyIndex = defeatedIndex;
        loadNextEnemy();
    });
}

void BattleScreen::playHpBarAnimation(QProgressBar* bar, int oldVal, int newVal) {
    QPropertyAnimation *anim = new QPropertyAnimation(bar, "value");
    anim->setDuration(500);
    anim->setStartValue(oldVal);
    anim->setEndValue(newVal);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start();
}

void BattleScreen::playMageAnimation(int) {}

// ── BUTTONS ──
void BattleScreen::setButtonsEnabled(bool enabled) {
    btnFight->setEnabled(enabled);
    btnSkill->setEnabled(enabled);
    btnItem->setEnabled(enabled);
    btnFlee->setEnabled(enabled);
}

// ── EVENT FILTER ──
bool BattleScreen::eventFilter(QObject *obj, QEvent *event) {
    if (isAnimating) {
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseButtonDblClick)
            return true;
    }

    // Hover enlarge only when choosing ally target
    if (selectionMode != SelectionMode::NONE) {
        for (int i = 0; i < (int)party.size() && i < 3; i++) {
            if (obj == characterSprites[i]) {
                if (event->type() == QEvent::Enter) {
                    enlargeAllySprite(i);
                    if (btnCancelInventory && btnCancelInventory->isVisible())
                        btnCancelInventory->raise();
                    return false;
                }
                if (event->type() == QEvent::Leave) {
                    resetAllySpriteSize(i);
                    updateAllyHighlight();
                    if (btnCancelInventory && btnCancelInventory->isVisible())
                        btnCancelInventory->raise();
                    return false;
                }
            }
        }
    }

    if (!isAnimating && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            hideStatsPanel();
            return true;
        }
    }

    if (!isAnimating && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::RightButton) {
            // Enemy stats
            for (int i = 0; i < (int)enemies.size() && i < 3; i++) {
                if (obj == enemySprites3[i] && !enemies[i]->isDead()) {
                    showStatsPanel(
                        enemies[i]->getHP(), enemies[i]->getMaxHP(),
                        enemies[i]->getAttack(), enemies[i]->getDefense(),
                        enemies[i]->getSpeed(),
                        QString::fromStdString(enemies[i]->getName()),
                        enemySprites3[i]->mapToParent(QPoint(0, 0))
                        );
                    return true;
                }
            }
            // Party stats
            for (int i = 0; i < getActivePartySize(); i++) {
                if (obj == characterSprites[i]) {
                    showStatsPanel(
                        party[i]->getHP(),
                        party[i]->getMaxHP(),
                        party[i]->getAttack(),
                        party[i]->getDefense(),
                        party[i]->getSpeed(),
                        QString::fromStdString(party[i]->getName()),
                        characterSprites[i]->mapToParent(QPoint(0, 0))
                        );
                    return true;
                }
            }
            hideStatsPanel();
            return false;

        } else {
            // Left click — enemy targeting
            for (int i = 0; i < (int)enemies.size() && i < 3; i++) {
                if (obj == enemySprites3[i] && selectionMode == SelectionMode::NONE) {
                    selectEnemyTarget(i);
                    return true;
                }
            }
            // Ally targeting
            if (selectionMode != SelectionMode::NONE) {
                for (int i = 0; i < (int)party.size() && i < 3; i++) {
                    if (obj == characterSprites[i]) {
                        useAbilityOnAlly(i);
                        return true;
                    }
                }
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

// ── TURN ORDER ──
void BattleScreen::buildTurnOrder() {
    combinedOrder.clear();

    for (int i = 0; i < (int)party.size(); i++)
        if (party[i]->isAlive())
            combinedOrder.push_back({party[i]->getName(), party[i]->getSpeed(), true, i});

    for (int i = 0; i < (int)enemies.size(); i++)
        if (!enemies[i]->isDead())
            combinedOrder.push_back({enemies[i]->getName(), enemies[i]->getSpeed(), false, i});

    sort(combinedOrder.begin(), combinedOrder.end(), [](auto& a, auto& b){
        return a.speed > b.speed;
    });

    combinedIndex = 0;
}

void BattleScreen::updateTurnOrder() {
    if (combinedOrder.empty()) {
        for (int i = 0; i < 4; i++) {
            turnOrderLabels[i]->setText("  —");
            turnOrderLabels[i]->setStyleSheet(
                "background:#0d0d1a; border:1px solid #2a2a4a; color:#888888;"
                + QString("font-size:%1px;").arg((int)(10 * W/800.0f)) +
                "font-family:'Courier New'; padding:2px 4px;");
        }
        return;
    }

    int shown = 0, offset = 0;
    int maxChecks = (int)combinedOrder.size() * 4;

    while (shown < 4 && offset < maxChecks) {
        int idx = (combinedIndex + offset) % (int)combinedOrder.size();
        const TurnEntry& entry = combinedOrder[idx];

        bool shouldShow = true;
        if (entry.isParty && !party[entry.index]->isAlive()) shouldShow = false;
        if (!entry.isParty && enemies[entry.index]->isDead()) shouldShow = false;

        if (shouldShow) {
            QString name = QString::fromStdString(entry.name);
            turnOrderLabels[shown]->setText(shown == 0 ? "▶ " + name : "  " + name);
            turnOrderLabels[shown]->setStyleSheet(shown == 0 ?
                                                      "background:#1a1a00; border:1px solid #ffdd44; color:#ffdd44;"
                                                          + QString("font-size:%1px;").arg((int)(10 * W/800.0f)) +
                                                          "font-family:'Courier New'; padding:2px 4px;" :
                                                      (entry.isParty ?
                                                           "background:#0d0d1a; border:1px solid #2a2a4a; color:#aaaaff;"
                                                               + QString("font-size:%1px;").arg((int)(10 * W/800.0f)) +
                                                               "font-family:'Courier New'; padding:2px 4px;" :
                                                           "background:#0d0d1a; border:1px solid #2a2a4a; color:#ff8888;"
                                                               + QString("font-size:%1px;").arg((int)(10 * W/800.0f)) +
                                                               "font-family:'Courier New'; padding:2px 4px;"));
            shown++;
        }
        offset++;
    }

    while (shown < 4) {
        turnOrderLabels[shown]->setText("  —");
        turnOrderLabels[shown]->setStyleSheet(
            "background:#0d0d1a; border:1px solid #2a2a4a; color:#888888;"
            + QString("font-size:%1px;").arg((int)(10 * W/800.0f)) +
            "font-family:'Courier New'; padding:2px 4px;");
        shown++;
    }
}

// ── TARGETING ──
int BattleScreen::getFirstAliveEnemyIndex() const {
    for (int i = 0; i < (int)enemies.size(); i++)
        if (!enemies[i]->isDead()) return i;
    return -1;
}

void BattleScreen::selectEnemyTarget(int enemyIndex) {
    if (enemyIndex < 0 || enemyIndex >= (int)enemies.size()) return;
    if (enemies[enemyIndex]->isDead()) {
        dialogueText->setText("That enemy is already defeated.");
        return;
    }
    selectedEnemyIndex = enemyIndex;
    currentEnemyIndex  = enemyIndex;
    enemySprite    = enemySprites3[selectedEnemyIndex];
    enemyHpBar     = enemyHpBars3[selectedEnemyIndex];
    enemyNameLabel = enemyNameLabels[selectedEnemyIndex];
    updateEnemyUI();
    updateEnemyTargetHighlight();
}

void BattleScreen::updateEnemyTargetHighlight() {
    for (int i = 0; i < MAX_ENEMY_SIZE; i++) {
        if (!enemySprites3[i] || !enemyTargetIndicators[i]) continue;

        if (i >= (int)enemies.size() || !enemies[i] || enemies[i]->isDead()) {
            enemySprites3[i]->setStyleSheet("background:none; border:none;");
            enemyTargetIndicators[i]->hide();
            continue;
        }

        QLabel* sprite = enemySprites3[i];
        QLabel* indicator = enemyTargetIndicators[i];

        if (i == selectedEnemyIndex) {
            sprite->setStyleSheet("background:none; border:none;");

            int indicatorW = (int)(sprite->width() * 0.85);
            int indicatorH = (int)(sprite->height() * 0.30);
            if (indicatorH < 10) indicatorH = 10;

            int indicatorX = sprite->x() + (sprite->width() - indicatorW) / 2;
            int indicatorY = sprite->y() + (int)(sprite->height() * 0.76);

            indicator->setGeometry(indicatorX, indicatorY, indicatorW, indicatorH);
            indicator->setStyleSheet(
                "background: rgba(255,221,68,115);"
                "border: 2px solid rgba(255,235,120,230);"
                "border-radius:" + QString::number(indicatorH/2) + "px;"
                );

            indicator->show();
            indicator->raise();
            sprite->raise(); // sprite tetap di atas
        } else {
            enemyTargetIndicators[i]->hide();
        }
    }
}

// AUTOTURN
void BattleScreen::executeAutoTurn() {
    if (!isAutoplay) return;
    if (combinedOrder.empty() || combinedIndex >= (int)combinedOrder.size()) return;

    auto& current = combinedOrder[combinedIndex];
    if (!current.isParty) return;

    Character* character = party[current.index];
    if (!character->isAlive()) { advanceTurn(); return; }

    // Cek skill HEAL dulu kalau ada ally HP rendah
    auto& skills = character->getSkills();
    Character* hurt = battleSystem->getLowestHpCharacter();

    // Cek heal
    if (hurt && !skills.empty() && skills[0].getType() == HEAL && skills[0].isReady() &&
        hurt->getHP() < hurt->getMaxHP() * 0.3) {
        lockAllInput();
        string log = battleSystem->characterUseSkill(character, 0, nullptr, hurt);
        dialogueText->setText(QString::fromStdString(log));
        updatePartyUI();
        advanceTurn();
        return;
    }

    // Cek skill DAMAGE
    if (!skills.empty() && skills[0].getType() == DAMAGE && skills[0].isReady()) {
        if (selectedEnemyIndex < 0 || selectedEnemyIndex >= (int)enemies.size() ||
            enemies[selectedEnemyIndex]->isDead()) {
            selectedEnemyIndex = getFirstAliveEnemyIndex();
            if (selectedEnemyIndex == -1) { showBattleEndBanner(true); return; }
        }
        currentEnemyIndex = selectedEnemyIndex;

        lockAllInput();
        string log = battleSystem->characterUseSkill(
            character, 0, enemies[selectedEnemyIndex], nullptr);
        dialogueText->setText(QString::fromStdString(log));
        updateEnemyUI();

        if (enemies[currentEnemyIndex]->isDead()) {
            QTimer::singleShot(600, this, &BattleScreen::playDefeatAnimation);
        } else {
            advanceTurn();
        }
        return;
    }

    // Default — basic attack ke enemy HP terendah
    Enemy* weakest = battleSystem->getWeakestEnemy();
    if (weakest) {
        for (int i = 0; i < (int)enemies.size(); i++) {
            if (enemies[i] == weakest) {
                selectedEnemyIndex = i;
                currentEnemyIndex = i;
                break;
            }
        }
    }

    if (selectedEnemyIndex < 0 || selectedEnemyIndex >= (int)enemies.size() ||
        enemies[selectedEnemyIndex]->isDead()) {
        selectedEnemyIndex = getFirstAliveEnemyIndex();
        if (selectedEnemyIndex == -1) { showBattleEndBanner(true); return; }
    }

    currentEnemyIndex = selectedEnemyIndex;
    selectEnemyTarget(selectedEnemyIndex);
    playAttackAnimation(current.index);
}

// ── VICTORY / DEFEAT ──
void BattleScreen::showBattleEndBanner(bool victory)
{
    if (battleEnding) return;
    battleEnding = true;

    lockAllInput();

    QString titleText = victory ? "BATTLE CLEAR!" : "BATTLE ENDED...";
    QString textColor = victory ? "#e8f8ff" : "#ffe0e0";

    QLabel *overlay = new QLabel(this);
    overlay->setGeometry(0, 0, W, H);
    overlay->setStyleSheet("background: rgba(0, 0, 0, 120);");
    overlay->show();
    overlay->raise();

    QLabel *banner = new QLabel(this);
    banner->setGeometry(0, H / 2 - 75, W, 150);

    if (victory) {
        banner->setStyleSheet(
            "background: qlineargradient("
            "x1:0, y1:0, x2:1, y2:0,"
            "stop:0 rgba(8, 20, 45, 220),"
            "stop:0.5 rgba(80, 170, 220, 235),"
            "stop:1 rgba(8, 20, 45, 220)"
            ");"
            "border-top: 3px solid #9adfff;"
            "border-bottom: 3px solid #9adfff;"
            );
    } else {
        banner->setStyleSheet(
            "background: qlineargradient("
            "x1:0, y1:0, x2:1, y2:0,"
            "stop:0 rgba(45, 8, 12, 220),"
            "stop:0.5 rgba(150, 35, 45, 235),"
            "stop:1 rgba(45, 8, 12, 220)"
            ");"
            "border-top: 3px solid #ff8888;"
            "border-bottom: 3px solid #ff8888;"
            );
    }

    banner->show();
    banner->raise();

    QLabel *topLine = new QLabel(this);
    topLine->setGeometry(0, H / 2 - 66, W, 2);
    topLine->setStyleSheet(
        victory
            ? "background: rgba(170, 230, 255, 170);"
            : "background: rgba(255, 180, 180, 170);"
        );
    topLine->show();
    topLine->raise();

    QLabel *bottomLine = new QLabel(this);
    bottomLine->setGeometry(0, H / 2 + 66, W, 2);
    bottomLine->setStyleSheet(
        victory
            ? "background: rgba(170, 230, 255, 170);"
            : "background: rgba(255, 180, 180, 170);"
        );
    bottomLine->show();
    bottomLine->raise();

    QLabel *title = new QLabel(titleText, this);
    title->setGeometry(0, H / 2 - 24, W, 48);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "background:none;"
        "color:" + textColor + ";"
                      "font-size:34px;"
                      "font-family:'Courier New';"
                      "font-weight:bold;"
                      "letter-spacing:3px;"
        );
    title->show();
    title->raise();

    QGraphicsOpacityEffect *overlayEffect = new QGraphicsOpacityEffect(overlay);
    QGraphicsOpacityEffect *bannerEffect = new QGraphicsOpacityEffect(banner);
    QGraphicsOpacityEffect *topEffect = new QGraphicsOpacityEffect(topLine);
    QGraphicsOpacityEffect *bottomEffect = new QGraphicsOpacityEffect(bottomLine);
    QGraphicsOpacityEffect *titleEffect = new QGraphicsOpacityEffect(title);

    overlay->setGraphicsEffect(overlayEffect);
    banner->setGraphicsEffect(bannerEffect);
    topLine->setGraphicsEffect(topEffect);
    bottomLine->setGraphicsEffect(bottomEffect);
    title->setGraphicsEffect(titleEffect);

    overlayEffect->setOpacity(0.0);
    bannerEffect->setOpacity(0.0);
    topEffect->setOpacity(0.0);
    bottomEffect->setOpacity(0.0);
    titleEffect->setOpacity(0.0);

    QPropertyAnimation *fadeInOverlay = new QPropertyAnimation(overlayEffect, "opacity", this);
    fadeInOverlay->setDuration(250);
    fadeInOverlay->setStartValue(0.0);
    fadeInOverlay->setEndValue(1.0);

    QPropertyAnimation *fadeInBanner = new QPropertyAnimation(bannerEffect, "opacity", this);
    fadeInBanner->setDuration(250);
    fadeInBanner->setStartValue(0.0);
    fadeInBanner->setEndValue(1.0);

    QPropertyAnimation *fadeInTop = new QPropertyAnimation(topEffect, "opacity", this);
    fadeInTop->setDuration(250);
    fadeInTop->setStartValue(0.0);
    fadeInTop->setEndValue(1.0);

    QPropertyAnimation *fadeInBottom = new QPropertyAnimation(bottomEffect, "opacity", this);
    fadeInBottom->setDuration(250);
    fadeInBottom->setStartValue(0.0);
    fadeInBottom->setEndValue(1.0);

    QPropertyAnimation *fadeInTitle = new QPropertyAnimation(titleEffect, "opacity", this);
    fadeInTitle->setDuration(300);
    fadeInTitle->setStartValue(0.0);
    fadeInTitle->setEndValue(1.0);

    fadeInOverlay->start(QAbstractAnimation::DeleteWhenStopped);
    fadeInBanner->start(QAbstractAnimation::DeleteWhenStopped);
    fadeInTop->start(QAbstractAnimation::DeleteWhenStopped);
    fadeInBottom->start(QAbstractAnimation::DeleteWhenStopped);
    fadeInTitle->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(900, this, [=]() {
        QPropertyAnimation *fadeOutOverlay = new QPropertyAnimation(overlayEffect, "opacity", this);
        fadeOutOverlay->setDuration(300);
        fadeOutOverlay->setStartValue(1.0);
        fadeOutOverlay->setEndValue(0.0);

        QPropertyAnimation *fadeOutBanner = new QPropertyAnimation(bannerEffect, "opacity", this);
        fadeOutBanner->setDuration(300);
        fadeOutBanner->setStartValue(1.0);
        fadeOutBanner->setEndValue(0.0);

        QPropertyAnimation *fadeOutTop = new QPropertyAnimation(topEffect, "opacity", this);
        fadeOutTop->setDuration(300);
        fadeOutTop->setStartValue(1.0);
        fadeOutTop->setEndValue(0.0);

        QPropertyAnimation *fadeOutBottom = new QPropertyAnimation(bottomEffect, "opacity", this);
        fadeOutBottom->setDuration(300);
        fadeOutBottom->setStartValue(1.0);
        fadeOutBottom->setEndValue(0.0);

        QPropertyAnimation *fadeOutTitle = new QPropertyAnimation(titleEffect, "opacity", this);
        fadeOutTitle->setDuration(300);
        fadeOutTitle->setStartValue(1.0);
        fadeOutTitle->setEndValue(0.0);

        connect(fadeOutOverlay, &QPropertyAnimation::finished, this, [=]() {
            overlay->deleteLater();
            banner->deleteLater();
            topLine->deleteLater();
            bottomLine->deleteLater();
            title->deleteLater();

            if (victory) {
                showVictoryScreen();
            } else {
                showDefeatScreen();
            }
        });

        fadeOutOverlay->start(QAbstractAnimation::DeleteWhenStopped);
        fadeOutBanner->start(QAbstractAnimation::DeleteWhenStopped);
        fadeOutTop->start(QAbstractAnimation::DeleteWhenStopped);
        fadeOutBottom->start(QAbstractAnimation::DeleteWhenStopped);
        fadeOutTitle->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void BattleScreen::showVictoryScreen() {
    // Jangan panggil distributeRewards() — coins sudah di-add per enemy

    QLabel *overlay = new QLabel(this);
    overlay->setGeometry(0, 0, W, H);
    overlay->setStyleSheet("background:rgba(0,0,0,180);");
    overlay->show();

    QLabel *title = new QLabel("VICTORY!", this);
    title->setGeometry(W/2-200, H/2-100, 400, 80);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#ffdd44; background: none; font-size:52px; font-weight:bold; font-family:'Courier New';");
    title->show();

    QLabel *sub = new QLabel("All enemies defeated!", this);
    sub->setGeometry(W/2-200, H/2-20, 400, 40);
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color:white; background: none; font-size:18px; font-family:'Courier New';");
    sub->show();

    QLabel *coinsLabel = new QLabel("🪙 +" + QString::number(totalCoinsEarned), this);
    coinsLabel->setGeometry(W/2-200, H/2+20, 400, 30);
    coinsLabel->setAlignment(Qt::AlignCenter);
    coinsLabel->setStyleSheet("color:#ffdd44; background: none; font-size:20px; font-family:'Courier New'; font-weight:bold;");
    coinsLabel->show();

    QPushButton *btnContinue = new QPushButton("CONTINUE", this);
    btnContinue->setGeometry(W/2-120, H/2+80, 240, 50);
    btnContinue->setStyleSheet(
        "QPushButton{background:#1a1a2e; color:#ffdd44; border:2px solid #ffdd44;"
        "font-family:'Courier New'; font-size:16px; font-weight:bold;}"
        "QPushButton:hover{background:#ffdd44; color:#0d0d1a;}");
    btnContinue->show();
    connect(btnContinue, &QPushButton::clicked, this, [this]() { emit battleFinished(true); });
    setButtonsEnabled(false);
}

void BattleScreen::showDefeatScreen() {
    QLabel *overlay = new QLabel(this);
    overlay->setGeometry(0, 0, W, H);
    overlay->setStyleSheet("background:rgba(0,0,0,200);");
    overlay->show();

    QLabel *title = new QLabel("DEFEAT...", this);
    title->setGeometry(W/2-200, H/2-100, 400, 80);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#cc4444; font-size:52px; font-weight:bold; font-family:'Courier New';");
    title->show();

    QLabel *sub = new QLabel("Your party has been wiped out.", this);
    sub->setGeometry(W/2-200, H/2, 400, 40);
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color:white; font-size:18px; font-family:'Courier New';");
    sub->show();

    QPushButton *btnRetry = new QPushButton("RETRY", this);
    btnRetry->setGeometry(W/2-120, H/2+80, 240, 50);
    btnRetry->setStyleSheet(
        "QPushButton{background:#1a1a2e; color:#cc4444; border:2px solid #cc4444;"
        "font-family:'Courier New'; font-size:16px; font-weight:bold;}"
        "QPushButton:hover{background:#cc4444; color:white;}");
    btnRetry->show();
    connect(btnRetry, &QPushButton::clicked, this, [this]() { emit battleFinished(false); });
    setButtonsEnabled(false);
}
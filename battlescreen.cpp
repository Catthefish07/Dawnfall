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
                           BattleMode mode, int W, int H, QWidget *parent)
    : QWidget(parent), party(partyIn),
    inventory(inventoryIn),
    mode(mode), currentEnemyIndex(0), W(W), H(H),
    isAnimating(false), combinedIndex(0),
    waves(wavesIn), currentWave(0), totalCoinsEarned(0)
{
    // Spawn wave pertama
    for (const string& name : waves[0]) {
        Enemy* e = EnemySpawner::createEnemy(name);
        if (e) enemies.push_back(e);
    }

    battleSystem = new BattleSystem(party, enemies, *inventory, mode);
    setupUI();
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
    bgLabel->setGeometry(0, 0, W, y(460));
    bgLabel->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
                           "stop:0 #0a0a1a, stop:0.6 #1a1a3a, stop:1 #2a2a1a);");

    // TOP
    chapterLabel = new QLabel("Chapter 1 — The Forest", this);
    chapterLabel->setGeometry(0, y(8), W, y(20));
    chapterLabel->setAlignment(Qt::AlignCenter);
    chapterLabel->setStyleSheet("color:#8888cc;" + fs(11) + "font-family:'Courier New'; letter-spacing:3px;");

    waveLabel = new QLabel("⚔ Wave 1/" + QString::number(waves.size()), this);
    waveLabel->setGeometry(x(10), y(8), w(100), y(20));
    waveLabel->setStyleSheet("color:#8888cc;" + fs(10) + "font-family:'Courier New';");

    // HERO SPRITES
    QString characterColors[3] = {"#1a4a2a","#1a3a6a","#3a2a1a"};
    QString characterDefaultNames[3] = {"Ethan","MC","Hubert"};
    int characterX[3] = {x(280), x(370), x(280)};
    int characterY[3] = {y(80),  y(160), y(250)};
    int spriteSize = w(65);

    for (int i = 0; i < 3; i++) {
        characterSprites[i] = new QLabel(this);
        characterSprites[i]->setGeometry(characterX[i], characterY[i], spriteSize, spriteSize);
        characterSprites[i]->setStyleSheet(
            "background-color:" + characterColors[i] + ";"
                                                       "border-radius:" + QString::number(spriteSize/2) + "px;"
                                                "border:2px solid #4a4a8a;");
        characterSprites[i]->setAlignment(Qt::AlignCenter);
        characterSprites[i]->setText(characterDefaultNames[i].left(1));
        characterSprites[i]->setCursor(Qt::PointingHandCursor);
        characterSprites[i]->installEventFilter(this);

        characterHpBars[i] = nullptr;
        characterHpText[i] = nullptr;
    }

    // ENEMY SPRITES
    int enemyX[3] = {x(660), x(560), x(660)};
    int enemyY[3] = {y(80),  y(160), y(250)};

    for (int i = 0; i < 3; i++) {
        enemyNameLabels[i] = new QLabel(this);
        enemyNameLabels[i]->setGeometry(enemyX[i], enemyY[i] - y(30), spriteSize, y(14));
        enemyNameLabels[i]->setStyleSheet("background:none; color:#ff8888;" + fs(9) + "font-family:'Courier New';");
        enemyNameLabels[i]->setAlignment(Qt::AlignCenter);
        enemyNameLabels[i]->setText(i < (int)enemies.size() ?
                                        QString::fromStdString(enemies[i]->getName()) : "?");

        enemyHpBars3[i] = new QProgressBar(this);
        enemyHpBars3[i]->setGeometry(enemyX[i], enemyY[i] - y(14), spriteSize, y(6));
        enemyHpBars3[i]->setRange(0, 100);
        enemyHpBars3[i]->setValue(100);
        enemyHpBars3[i]->setTextVisible(false);
        enemyHpBars3[i]->setStyleSheet("QProgressBar{background:#2a2a2a;border-radius:3px;}"
                                       "QProgressBar::chunk{background:#cc4444;border-radius:3px;}");

        enemySprites3[i] = new QLabel(this);
        enemySprites3[i]->setGeometry(enemyX[i], enemyY[i], spriteSize, spriteSize);
        enemySprites3[i]->setAlignment(Qt::AlignCenter);
        enemySprites3[i]->setText("E");
        enemySprites3[i]->setCursor(Qt::PointingHandCursor);
        enemySprites3[i]->installEventFilter(this);
        enemySprites3[i]->setStyleSheet(
            "background-color:#2a1a1a; color:#ff8888;"
            "border-radius:" + QString::number(spriteSize/2) + "px;"
                                                "border:2px solid #cc4444;"
                                                "font-family:'Courier New';" + fs(14));

        if (i == 0) {
            enemySprite    = enemySprites3[i];
            enemyHpBar     = enemyHpBars3[i];
            enemyNameLabel = enemyNameLabels[i];
        }
    }

    enemyHpText = new QLabel("", this);
    enemyHpText->hide();

    // BOTTOM PANEL
    QLabel *bottomPanel = new QLabel(this);
    bottomPanel->setGeometry(0, y(370), W, y(300));
    bottomPanel->setStyleSheet("background:#0d0d1a;");

    QLabel *dlgBox = new QLabel(this);
    dlgBox->setGeometry(0, y(365), W, y(70));
    dlgBox->setStyleSheet("background:#0d0d1a; border-top:2px solid black; border-bottom:4px solid #4a4a8a;");

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
    QString pnames[3] = {"Ethan","MC","Hubert"};
    int slotW = 1100 / 3;

    for (int i = 0; i < 3; i++) {
        characterPfp[i] = new QLabel(this);
        characterPfp[i]->setGeometry(i * slotW + x(25), y(450), x(40), y(50));
        characterPfp[i]->setAlignment(Qt::AlignCenter);

        QString pfpPaths[3] = {
            ":/assets/profilepic/ethan_pfp.png",
            ":/assets/profilepic/mc_pfp.png",
            ":/assets/profilepic/hubert_pfp.png"
        };
        QPixmap pfpImg(pfpPaths[i]);
        if (!pfpImg.isNull())
            characterPfp[i]->setPixmap(pfpImg.scaled(x(50), y(50), Qt::KeepAspectRatio, Qt::FastTransformation));
        else
            characterPfp[i]->setText(pnames[i].left(1));

        // Name
        QLabel *pname = new QLabel(i < (int)party.size() ?
                                       QString::fromStdString(party[i]->getName()) : pnames[i], this);
        pname->setGeometry(i * slotW + x(75), y(453), x(85), y(18));
        pname->setStyleSheet("color:white;" + fs(12) + "font-family:'Courier New'; font-weight:bold;");

        // Skill cooldown label
        skillCooldownLabels[i] = new QLabel("", this);
        skillCooldownLabels[i]->setGeometry(i * slotW + x(162), y(453), x(45), y(16));
        skillCooldownLabels[i]->setStyleSheet("color:#ffaa44;" + fs(9) + "font-family:'Courier New'; letter-spacing:-1px;");
        skillCooldownLabels[i]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        // HP bars
        characterHpBars[i] = new QProgressBar(this);
        characterHpBars[i]->setRange(0, 100);
        characterHpBars[i]->setValue(100);
        characterHpBars[i]->setGeometry(i * slotW + x(75), y(475), x(85), y(20));
        characterHpBars[i]->setTextVisible(false);
        characterHpBars[i]->setStyleSheet(
            "QProgressBar{background:#0d0d1a; border:2px solid #44cc44; border-radius:3px;}"
            "QProgressBar::chunk{background:#44cc44; border-radius:2px;}");

        // HP bars text
        characterHpText[i] = new QLabel(this);
        characterHpText[i]->setGeometry(i * slotW + x(165), y(475), x(40), y(20));
        characterHpText[i]->setStyleSheet("color:#aaaaaa;" + fs(9) + "font-family:'Courier New'; font-weight:bold;");
        characterHpText[i]->setText("100/100");
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
            if (selectedEnemyIndex == -1) { showVictoryScreen(); return; }
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
                skills[0].name + " is still on cooldown!"
                ));
            return;
        }

        if (skills[0].type == DAMAGE) {
            if (selectedEnemyIndex < 0 ||
                selectedEnemyIndex >= (int)enemies.size() ||
                enemies[selectedEnemyIndex]->isDead()) {

                selectedEnemyIndex = getFirstAliveEnemyIndex();

                if (selectedEnemyIndex == -1) {
                    showVictoryScreen();
                    return;
                }
            }

            currentEnemyIndex = selectedEnemyIndex;

            updatePortraitByName(party[current.index]->getName());

            string log = battleSystem->characterUseSkill(
                party[current.index], 0, enemies[selectedEnemyIndex], nullptr);

            lockAllInput();

            dialogueText->setText(QString::fromStdString(log));
            addBattleLog(QString::fromStdString(log));

            updateEnemyUI();

            if (enemies[currentEnemyIndex]->isDead()) {
                QTimer::singleShot(600, this, &BattleScreen::playDefeatAnimation);
                return;
            }

            advanceTurn();

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

// ── ADVANCE TURN ──
void BattleScreen::advanceTurn() {
    if (combinedOrder.empty()) { showVictoryScreen(); return; }

    combinedIndex = (combinedIndex + 1) % (int)combinedOrder.size();

    int tries = 0;
    while (tries < (int)combinedOrder.size()) {
        TurnEntry& entry = combinedOrder[combinedIndex];

        if (entry.isParty && !party[entry.index]->isAlive()) {
            combinedIndex = (combinedIndex + 1) % (int)combinedOrder.size();
            tries++; continue;
        }
        if (!entry.isParty && enemies[entry.index]->isDead()) {
            combinedIndex = (combinedIndex + 1) % (int)combinedOrder.size();
            tries++; continue;
        }
        break;
    }

    if (tries >= (int)combinedOrder.size()) {
        battleSystem->allCharactersDead() ? showDefeatScreen() : showVictoryScreen();
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
            }
        });
    }
}

// ── SKILL COOLDOWN ──
void BattleScreen::updateSkillCooldowns() {
    for (int i = 0; i < (int)party.size() && i < 3; i++) {
        auto& skills = party[i]->getSkills();
        if (skills.empty() || skills[0].isReady()) {
            skillCooldownLabels[i]->setText("");
        } else {
            skillCooldownLabels[i]->setText("⏳ " + QString::number(skills[0].currentCooldown));
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
void BattleScreen::updateAllyHighlight() {
    QString colors[3] = {"#1a4a2a","#1a3a6a","#3a2a1a"};
    int r = (int)(32 * W / 800.0f);

    bool selectingReviveTarget = false;

    if (selectionMode == SelectionMode::ITEM_ALLY && selectedItemIndex >= 0) {
        vector<Item> items = inventory->getItems();

        if (selectedItemIndex < (int)items.size() &&
            items[selectedItemIndex].name == "Revive Stone") {
            selectingReviveTarget = true;
        }
    }

    for (int i = 0; i < (int)party.size() && i < 3; i++) {
        bool alive = party[i]->isAlive();

        if (selectionMode != SelectionMode::NONE) {
            if (alive || selectingReviveTarget) {
                characterSprites[i]->setStyleSheet(
                    "background-color:" + (alive ? colors[i] : QString("#555555")) + ";"
                                                                                     "border-radius:" + QString::number(r) + "px;"
                                           "border:3px solid #ffdd44;"
                    );
            } else {
                characterSprites[i]->setStyleSheet(
                    "background-color:#555555;"
                    "border-radius:" + QString::number(r) + "px;"
                                           "border:2px solid #888888;"
                    );
            }
        } else {
            if (!alive) {
                characterSprites[i]->setStyleSheet(
                    "background-color:#555555;"
                    "border-radius:" + QString::number(r) + "px;"
                                           "border:2px solid #888888;"
                    );
            } else {
                characterSprites[i]->setStyleSheet(
                    "background-color:" + colors[i] + ";"
                                                      "border-radius:" + QString::number(r) + "px;"
                                           "border:2px solid #4a4a8a;"
                    );
            }
        }
    }
}

void BattleScreen::enlargeAllySprite(int index) {
    if (index < 0 || index >= 3) return;

    int baseSize = (int)(65 * W / 800.0f);
    int enlargedSize = (int)(78 * W / 800.0f);

    int diff = enlargedSize - baseSize;

    QRect geo = characterSprites[index]->geometry();

    characterSprites[index]->setGeometry(
        geo.x() - diff / 2,
        geo.y() - diff / 2,
        enlargedSize,
        enlargedSize
        );

    characterSprites[index]->raise();

    // Important: cancel button must stay above enlarged sprite
    if (btnCancelInventory && btnCancelInventory->isVisible()) {
        btnCancelInventory->raise();
    }
}

void BattleScreen::resetAllySpriteSize(int index) {
    if (index < 0 || index >= 3) return;

    float sx = W / 800.0f;
    float sy = H / 600.0f;

    auto x = [&](int v){ return (int)(v * sx); };
    auto y = [&](int v){ return (int)(v * sy); };
    auto w = [&](int v){ return (int)(v * sx); };

    int characterX[3] = {x(280), x(370), x(280)};
    int characterY[3] = {y(80),  y(160), y(250)};
    int spriteSize = w(65);

    characterSprites[index]->setGeometry(
        characterX[index],
        characterY[index],
        spriteSize,
        spriteSize
        );
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

        // DEFEND / HEAL should target living ally
        if (!target->isAlive()) {
            dialogueText->setText("Choose a living ally.");
            return;
        }

        lockAllInput();

        log = battleSystem->characterUseSkill(
            user,
            0,
            nullptr,
            target
            );

        pendingSkillCharIndex = -1;
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

    for (int i = 0; i < 3; i++) {
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
        QString normalPfpPaths[3] = {
            ":/assets/profilepic/ethan_pfp.png",
            ":/assets/profilepic/mc_pfp.png",
            ":/assets/profilepic/hubert_pfp.png"
        };
        QPixmap pfp(normalPfpPaths[targetIndex]);
        if (!pfp.isNull())
            characterPfp[targetIndex]->setPixmap(pfp.scaled(
                (int)(50 * W/800.0f), (int)(50 * H/600.0f),
                Qt::KeepAspectRatio, Qt::FastTransformation));
    }

    updateTurnOrder();

    QTimer::singleShot(700, this, [this]() {
        advanceTurn();
    });
}

// ── UPDATE UI ──
void BattleScreen::updatePartyUI() {
    for (int i = 0; i < (int)party.size() && i < 3; i++) {
        if (!characterHpBars[i] || !characterHpText[i]) continue;
        int pct = (int)(100.0 * party[i]->getHP() / party[i]->getMaxHP());
        characterHpBars[i]->setValue(pct);
        characterHpText[i]->setText(QString::number(party[i]->getHP()) + "/" +
                                    QString::number(party[i]->getMaxHP()));
        QString color = pct > 50 ? "#44cc44" : pct > 25 ? "#cccc44" : "#cc4444";
        characterHpBars[i]->setStyleSheet("QProgressBar{background:#2a2a2a;border-radius:4px;}"
                                          "QProgressBar::chunk{background:" + color + ";border-radius:4px;}");
    }
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
void BattleScreen::loadNextEnemy() {
    totalCoinsEarned += enemies[currentEnemyIndex]->getCoinDrop();

    enemySprites3[currentEnemyIndex]->hide();
    enemyNameLabels[currentEnemyIndex]->hide();
    enemyHpBars3[currentEnemyIndex]->hide();

    for (int i = (int)combinedOrder.size() - 1; i >= 0; --i) {
        const TurnEntry& e = combinedOrder[i];
        if (!e.isParty && enemies[e.index]->isDead()) {
            if (i < combinedIndex) combinedIndex--;
            combinedOrder.erase(combinedOrder.begin() + i);
        }
    }

    selectedEnemyIndex = getFirstAliveEnemyIndex();
    if (selectedEnemyIndex == -1) {
        // Semua enemy wave ini mati — cek apakah ada wave berikutnya
        QTimer::singleShot(1000, this, [this]() {
            loadNextWave();
        });
        return;
    }

    currentEnemyIndex = selectedEnemyIndex;
    if (combinedOrder.empty()) {
        QTimer::singleShot(1000, this, [this]() { loadNextWave(); });
        return;
    }

    enemySprite    = enemySprites3[currentEnemyIndex];
    enemyHpBar     = enemyHpBars3[currentEnemyIndex];
    enemyNameLabel = enemyNameLabels[currentEnemyIndex];

    enemySprite->setStyleSheet(
        "background-color:#2a1a1a; color:#ff8888;"
        "border-radius:" + QString::number((int)(65 * W/800.0f)/2) + "px;"
                                                         "border:2px solid #cc4444; font-family:'Courier New';");

    updateEnemyUI();
    updateEnemyTargetHighlight();
    advanceTurn();
}

// ── LOAD NEXT WAVE ──
void BattleScreen::loadNextWave() {
    currentWave++;

    if (currentWave >= (int)waves.size()) {
        showVictoryScreen();
        return;
    }

    // Clear enemies lama
    for (auto e : enemies) delete e;
    enemies.clear();

    // Spawn enemies wave baru
    for (const string& name : waves[currentWave]) {
        Enemy* e = EnemySpawner::createEnemy(name);
        if (e) enemies.push_back(e);
    }

    // Reset UI enemies
    for (int i = 0; i < 3; i++) {
        if (i < (int)enemies.size()) {
            enemyNameLabels[i]->setText(QString::fromStdString(enemies[i]->getName()));
            enemyHpBars3[i]->setValue(100);
            enemySprites3[i]->show();
            enemyNameLabels[i]->show();
            enemyHpBars3[i]->show();

            // ← tambah ini — reset style dan opacity
            enemySprites3[i]->setGraphicsEffect(nullptr);  // hapus fade effect
            enemySprites3[i]->setStyleSheet(
                "background-color:#2a1a1a; color:#ff8888;"
                "border-radius:" + QString::number((int)(65 * W/800.0f)/2) + "px;"
                                                                 "border:2px solid #cc4444;"
                                                                 "font-family:'Courier New'; font-size:" +
                QString::number((int)(14 * W/800.0f)) + "px;");
            enemySprites3[i]->setText("E");
        } else {
            enemySprites3[i]->hide();            enemyNameLabels[i]->hide();
            enemyHpBars3[i]->hide();
        }
    }

    // Reset index dan pointer
    currentEnemyIndex = 0;
    enemySprite    = enemySprites3[0];
    enemyHpBar     = enemyHpBars3[0];
    enemyNameLabel = enemyNameLabels[0];

    // Update wave label
    waveLabel->setText("👾 Wave " + QString::number(currentWave + 1) +
                       "/" + QString::number(waves.size()));

    // Rebuild turn order dengan enemies baru
    buildTurnOrder();
    updateTurnOrder();
    updateEnemyUI();
    updateEnemyTargetHighlight();

    selectedEnemyIndex = getFirstAliveEnemyIndex();
    if (selectedEnemyIndex != -1) selectEnemyTarget(selectedEnemyIndex);

    dialogueText->setText("Wave " + QString::number(currentWave + 1) + " begins!");

    // Cek siapa yang giliran pertama
    auto& first = combinedOrder[combinedIndex];
    if (!first.isParty) {
        QTimer::singleShot(1000, this, [this]() {
            playEnemyTurn();
        });
    } else {
        unlockAllInput();
        if (!combinedOrder.empty()) {
            auto& first = combinedOrder[combinedIndex];
            if (first.isParty)
                updatePortraitByName(first.name);
            else
                updatePortraitByName(first.name); // enemy portrait
        }
    }
}

// ── ANIMATIONS ──
void BattleScreen::playAttackAnimation(int characterIndex) {
    if (characterIndex >= (int)party.size()) { unlockAllInput(); return; }

    updatePortraitByName(party[characterIndex]->getName());

    QLabel* sprite = characterSprites[characterIndex];
    QPoint origin  = sprite->pos();

    QPropertyAnimation *lunge = new QPropertyAnimation(sprite, "pos");
    lunge->setDuration(150);
    lunge->setStartValue(origin);
    lunge->setEndValue(origin + QPoint(20, 0));
    lunge->start();

    connect(lunge, &QPropertyAnimation::finished, [=]() {
        QPropertyAnimation *back = new QPropertyAnimation(sprite, "pos");
        back->setDuration(150);
        back->setStartValue(sprite->pos());
        back->setEndValue(origin);
        back->start();

        connect(back, &QPropertyAnimation::finished, [=]() {
            party[characterIndex]->tickSkills();
            updateSkillCooldowns();
            int dmg = party[characterIndex]->dealDamage();
            if (currentEnemyIndex < (int)enemies.size()) {
                enemies[currentEnemyIndex]->takeDamage(dmg);
                playEnemyHitFlash();
                playShakeAnimation(enemySprite);
                updateEnemyUI();
                addBattleLog(QString::fromStdString(party[characterIndex]->getName()) +
                             " attacks for " + QString::number(dmg) + " dmg!");
                dialogueText->setText(QString::fromStdString(party[characterIndex]->getName()) +
                                      " attacks for <b>" + QString::number(dmg) + " damage!</b>");

                if (enemies[currentEnemyIndex]->isDead()) {
                    QTimer::singleShot(600, this, &BattleScreen::playDefeatAnimation);
                } else {
                    advanceTurn();
                }
            }
        });
    });
}

void BattleScreen::playEnemyTurn() {
    lockAllInput();

    if (combinedOrder.empty() || combinedIndex >= (int)combinedOrder.size()) {
        unlockAllInput(); return;
    }

    auto& current = combinedOrder[combinedIndex];
    if (current.isParty) { unlockAllInput(); return; }

    Enemy* enemy = enemies[current.index];
    if (enemy->isDead()) { unlockAllInput(); return; }

    updatePortraitByName(current.name);

    int hpBefore = enemy->getHP();
    battleSystem->enemyTurn(enemy);
    int hpAfter = enemy->getHP();
    int healAmount = hpAfter - hpBefore;

    updateEnemyUI();

    // Floating heal kalau enemy heal diri sendiri
    if (healAmount > 0) {
        QLabel *floatingHeal = new QLabel("💚 +" + QString::number(healAmount), this);
        floatingHeal->setGeometry(enemySprite->x(), enemySprite->y() - 20, 100, 20);
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

        connect(fade, &QPropertyAnimation::finished, [=]() {
            floatingHeal->deleteLater();
        });
    }
    updateSkillCooldowns();

    string fullLog = battleSystem->getBattleLog();
    size_t lastNewline = fullLog.rfind('\n', fullLog.size() - 2);
    string lastLine = (lastNewline != string::npos) ?
                          fullLog.substr(lastNewline + 1) : fullLog;
    while (!lastLine.empty() && lastLine.back() == '\n') lastLine.pop_back();
    dialogueText->setText(QString::fromStdString(lastLine));

    playShakeAnimation(characterSprites[0]);
    updatePartyUI();

    for (int i = 0; i < (int)party.size(); i++) {
        if (!party[i]->isAlive()) {
            QString deadPfpPaths[3] = {
                ":/assets/profilepic/ethan_pfp_dead.png",
                ":/assets/profilepic/mc_pfp_dead.png",
                ":/assets/profilepic/hubert_pfp_dead.png"
            };
            QPixmap deadPfp(deadPfpPaths[i]);
            if (!deadPfp.isNull())
                characterPfp[i]->setPixmap(deadPfp.scaled(
                    (int)(40 * W/800.0f), (int)(50 * H/600.0f),
                    Qt::KeepAspectRatio, Qt::FastTransformation));
        }
    }

    updateTurnOrder();
    if (battleSystem->allCharactersDead()) { showDefeatScreen(); return; }

    QTimer::singleShot(1000, this, [this]() { advanceTurn(); });
}

void BattleScreen::playEnemyHitFlash() {
    enemySprite->setStyleSheet(
        "background-color:#cc4444; color:#ff8888;"
        "border-radius:" + QString::number((int)(65 * W/800.0f)/2) + "px;"
                                                         "border:3px solid #ffdd44; font-family:'Courier New';");
    QTimer::singleShot(200, this, [this]() { updateEnemyTargetHighlight(); });
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

void BattleScreen::playDefeatAnimation() {

    int coinDrop = enemies[currentEnemyIndex]->getCoinDrop();
    QLabel *floatingCoins = new QLabel("🪙 +" + QString::number(coinDrop), this);
    floatingCoins->setGeometry(enemySprite->x(), enemySprite->y() - 20, 120, 20);
    floatingCoins->setStyleSheet("color:#ffdd44; font-size:20px; font-family:'Courier New'; font-weight:bold;");
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
    enemyNameLabels[currentEnemyIndex]->hide();
    enemyHpBars3[currentEnemyIndex]->hide();
    addBattleLog(QString::fromStdString(enemies[currentEnemyIndex]->getName()) + " defeated!");
    connect(fadeOut, &QPropertyAnimation::finished, this, &BattleScreen::loadNextEnemy);
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
            for (int i = 0; i < 3; i++) {
                if (obj == characterSprites[i]) {
                    showStatsPanel(
                        party[i]->getHP(), party[i]->getMaxHP(),
                        party[i]->getAttack(), party[i]->getDefense(),
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
    int spriteSize = (int)(65 * W/800.0f);
    for (int i = 0; i < (int)enemies.size() && i < 3; i++) {
        if (enemies[i]->isDead()) {
            enemySprites3[i]->setStyleSheet(
                "background-color:#111111; color:#555555;"
                "border-radius:" + QString::number(spriteSize/2) + "px;"
                                                    "border:2px solid #333333; font-family:'Courier New';");
        } else if (i == selectedEnemyIndex) {
            enemySprites3[i]->setStyleSheet(
                "background-color:#2a1a1a; color:#ff8888;"
                "border-radius:" + QString::number(spriteSize/2) + "px;"
                                                    "border:3px solid #ffdd44; font-family:'Courier New';");
        } else {
            enemySprites3[i]->setStyleSheet(
                "background-color:#2a1a1a; color:#ff8888;"
                "border-radius:" + QString::number(spriteSize/2) + "px;"
                                                    "border:2px solid #cc4444; font-family:'Courier New';");
        }
    }
}

// ── VICTORY / DEFEAT ──
void BattleScreen::showVictoryScreen() {
    QLabel *overlay = new QLabel(this);
    overlay->setGeometry(0, 0, W, H);
    overlay->setStyleSheet("background:rgba(0,0,0,180);");
    overlay->show();

    QLabel *title = new QLabel("VICTORY!", this);
    title->setGeometry(W/2-200, H/2-100, 400, 80);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#ffdd44; font-size:52px; font-weight:bold; font-family:'Courier New';");
    title->show();

    QLabel *sub = new QLabel("All enemies defeated!", this);
    sub->setGeometry(W/2-200, H/2-20, 400, 40);
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color:white; font-size:18px; font-family:'Courier New';");
    sub->show();

    QLabel *coinsLabel = new QLabel("🪙 +" + QString::number(totalCoinsEarned), this);
    coinsLabel->setGeometry(W/2-200, H/2 + 20, 400, 30);
    coinsLabel->setAlignment(Qt::AlignCenter);
    coinsLabel->setStyleSheet("color:#ffdd44; font-size:20px; font-family:'Courier New'; font-weight:bold;");
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
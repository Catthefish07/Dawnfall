#include "battlescreen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>

// Member 4 — Frontend 2: Battle UI, Animations & Dialogue
// This is your main file to implement!

BattleScreen::BattleScreen(vector<Hero*> partyIn, vector<Enemy*> enemiesIn,
                           BattleMode mode, int W, int H, QWidget *parent)
    : QWidget(parent), party(partyIn), enemies(enemiesIn),
    mode(mode), currentEnemyIndex(0), W(W), H(H), isAnimating(false)
{
    battleSystem = new BattleSystem(partyIn, enemiesIn, *new Inventory(), mode);
    setupUI();
    setupConnections();
}

void BattleScreen::setupUI() {
    setFixedSize(W, H);
    setStyleSheet("background-color: #0d0d1a;");

    float sx = W / 800.0f;
    float sy = H / 600.0f;

    auto x = [&](int v){ return (int)(v * sx); };
    auto y = [&](int v){ return (int)(v * sy); };
    auto w = [&](int v){ return (int)(v * sx); };
    auto fs = [&](int v){ return QString("font-size:%1px;").arg((int)(v * sx)); };

    // ── BACKGROUND ──
    bgLabel = new QLabel(this);
    bgLabel->setGeometry(0, 0, W, y(460));
    bgLabel->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
                           "stop:0 #0a0a1a, stop:0.6 #1a1a3a, stop:1 #2a2a1a);");

    // ── TOP ──
    chapterLabel = new QLabel("Chapter 1 — The Forest", this);
    chapterLabel->setGeometry(0, y(8), W, y(20));
    chapterLabel->setAlignment(Qt::AlignCenter);
    chapterLabel->setStyleSheet("color:#8888cc;" + fs(11) + "font-family:'Courier New'; letter-spacing:3px;");

    turnLabel = new QLabel("Turn 1", this);
    turnLabel->setGeometry(x(720), y(8), w(70), y(20));
    turnLabel->setStyleSheet("color:#4a4a8a;" + fs(10) + "font-family:'Courier New';");

    // ── HERO SPRITES (diagonal, NO names/hp above) ──
    QString heroColors[3] = {"#1a4a2a","#1a3a6a","#3a2a1a"};
    QString heroDefaultNames[3] = {"Ethan","MC","Hubert"};
    int heroX[3] = {x(280), x(370), x(280)};
    int heroY[3] = {y(80),  y(160), y(250)};
    int spriteSize = w(65);

    for (int i = 0; i < 3; i++) {
        heroSprites[i] = new QLabel(this);
        heroSprites[i]->setGeometry(heroX[i], heroY[i], spriteSize, spriteSize);
        heroSprites[i]->setStyleSheet("background-color:" + heroColors[i] + ";"
                                                                            "border-radius:" + QString::number(spriteSize/2) + "px;"
                                                                          "border:2px solid #4a4a8a;");
        heroSprites[i]->setAlignment(Qt::AlignCenter);
        heroSprites[i]->setText(heroDefaultNames[i].left(1));

        heroHpBars[i] = nullptr;
        heroHpText[i] = nullptr;
    }

    // ── ENEMY SPRITES (diagonal, mirror) ──
    int enemyX[3] = {x(660), x(560), x(660)};
    int enemyY[3] = {y(80),  y(160), y(250)};

    for (int i = 0; i < 3; i++) {
        enemyNameLabels[i] = new QLabel(this);
        enemyNameLabels[i]->setGeometry(enemyX[i], enemyY[i] - y(30), spriteSize, y(14));
        enemyNameLabels[i]->setStyleSheet("background: none; color:#ff8888;" + fs(9) + "font-family:'Courier New';");
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
        enemySprites3[i]->setStyleSheet("background-color:#2a1a1a; color:#ff8888;"
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

    // ── BOTTOM PANEL ──
    QLabel *bottomPanel = new QLabel(this);
    bottomPanel->setGeometry(0, y(370), W, y(300));
    bottomPanel->setStyleSheet("background:#0d0d1a; border-top: none;");

    // ── DIALOGUE BOX ──
    QLabel *dlgBox = new QLabel(this);
    dlgBox->setGeometry(0, y(365), W, y(70));
    dlgBox->setStyleSheet("background:#0d0d1a; border-top:2px solid black; border-bottom: 4px solid #4a4a8a;");

    // ── PORTRAIT ──
    dialoguePortrait = new QLabel(this);
    dialoguePortrait->setGeometry(-15, y(52), x(250), y(380));
    dialoguePortrait->setStyleSheet("background:none; border:none;");
    dialoguePortrait->setAlignment(Qt::AlignCenter);

    QPixmap portrait(":/assets/portraits/mc_neutral.PNG");
    if (!portrait.isNull())
        dialoguePortrait->setPixmap(portrait.scaled(x(420), y(600),
                                                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        dialoguePortrait->setText("MC");

    // ── DIALOGUE TEXT ──
    dialogueText = new QLabel("Select an action...", this);
    dialogueText->setGeometry(x(270), y(370), x(450), y(56));
    dialogueText->setStyleSheet("background: none;" "color:white;" + fs(13) + "font-family:'Courier New';");
    dialogueText->setWordWrap(true);

    // ── PARTY HP BARS (bottom, 3 slots horizontal) ──
    QString pnames[3] = {"Ethan","MC","Hubert"};
    int slotW = 1100 / 3;

    for (int i = 0; i < 3; i++) {
        // Pixel PFP
        heroPfp[i]= new QLabel(this);
        heroPfp[i]->setGeometry(i * slotW + x(25), y(450), x(40), y(50));
        heroPfp[i]->setAlignment(Qt::AlignCenter);
        QString pfpPaths[3] = {
            ":/assets/profilepic/ethan_pfp.png",
            ":/assets/profilepic/mc_pfp.png",
            ":/assets/profilepic/hubert_pfp.png"
        };

        QPixmap pfpImg(pfpPaths[i]);
        if (!pfpImg.isNull())
            heroPfp[i]->setPixmap(pfpImg.scaled(x(50), y(50), Qt::KeepAspectRatio, Qt::FastTransformation));
        else
            heroPfp[i]->setText(pnames[i].left(1));

        // Hero name
        QLabel *pname = new QLabel(i < (int)party.size() ?
                                       QString::fromStdString(party[i]->getName()) :
                                       pnames[i], this);
        pname->setGeometry(i * slotW + x(75), y(453), x(160), y(18));
        pname->setStyleSheet("color:white;" + fs(12) + "font-family:'Courier New'; font-weight:bold;");

        // HP bar — visible version
        heroHpBars[i] = new QProgressBar(this);
        heroHpBars[i]->setRange(0, 100);
        heroHpBars[i]->setValue(100);
        heroHpBars[i]->setGeometry(i * slotW + x(75), y(475), x(85), y(20));
        heroHpBars[i]->show();
        heroHpBars[i]->setTextVisible(false);
        heroHpBars[i]->setStyleSheet(
            "QProgressBar{background:#0d0d1a; border:2px solid #44cc44; border-radius:3px;}"
            "QProgressBar::chunk{background:#44cc44; border-radius:2px;}");

        // HP text
        heroHpText[i] = new QLabel(this);
        heroHpText[i]->setParent(this);
        heroHpText[i]->setGeometry(i * slotW + x(165), y(475), x(40), y(20));
        heroHpText[i]->setStyleSheet("color:#aaaaaa;" + fs(9) + "font-family:'Courier New';" "font-weight:bold;");
        heroHpText[i]->setText("100/100");
        heroHpText[i]->show();
    }

    // ── TURN ORDER PANEL ──
    QLabel *turnPanel = new QLabel(this);
    turnPanel->setGeometry(x(605), y(380), x(180), y(175));
    turnPanel->setStyleSheet("background:#0d0d1a; border:2px solid #4a4a8a; border-radius:6px;");

    // ── TURN ORDER (kanan) ──
    QLabel *turnTitle = new QLabel("TURN ORDER", this);
    turnTitle->setGeometry(x(605), y(390), x(180), y(16));
    turnTitle->setAlignment(Qt::AlignCenter);
    turnTitle->setStyleSheet("background: none;" "color:#666688;" + fs(9) + "font-family:'Courier New'; letter-spacing:2px;");

    for (int i = 0; i < 4; i++) {
        turnOrderLabels[i] = new QLabel(this);
        turnOrderLabels[i]->setGeometry(x(620), y(415) + i*y(32), x(150), y(24));
        turnOrderLabels[i]->setStyleSheet(i == 0 ?
                                              "background:#1a1a00; border:1px solid #ffdd44; color:#ffdd44;"
                                                  + fs(10) + "font-family:'Courier New'; padding:2px 4px;" :
                                              "background:#0d0d1a; border:1px solid #2a2a4a; color:#888888;"
                                                  + fs(10) + "font-family:'Courier New'; padding:2px 4px;");
        turnOrderLabels[i]->setText(i == 0 ? "▶ MC" : "  —");
    }

    // ── 4 BUTTONS HORIZONTAL ──
    QString btnStyle = "QPushButton{background:#1a1a2e;color:white;border:2px solid #4a4a8a;"
                       "font-family:'Courier New';" + fs(13) + "font-weight:bold;}"
                                  "QPushButton:hover{background:#4a4a8a;color:#ffdd44;}";

    int btnW = (1100 - x(5)) / 4;
    int btnY = y(515);
    int btnH = y(40);

    btnFight  = new QPushButton("FIGHT", this);
    btnFight->setGeometry(x(25), btnY, btnW, btnH);
    btnSkill  = new QPushButton("SKILL", this);
    btnSkill->setGeometry(x(25) + btnW, btnY, btnW, btnH);
    btnItem   = new QPushButton("ITEM",  this);
    btnItem->setGeometry(x(25) + btnW*2, btnY, btnW, btnH);
    btnDefend = new QPushButton("FLEE",  this);
    btnDefend->setGeometry(x(25) + btnW*3, btnY, btnW, btnH);

    btnFight->setStyleSheet(btnStyle);  btnSkill->setStyleSheet(btnStyle);
    btnItem->setStyleSheet(btnStyle);   btnDefend->setStyleSheet(btnStyle);

    // ── BATTLE LOG (hidden) ──
    battleLog = new QListWidget(this);
    battleLog->hide();

    updateTurnOrder();
}

void BattleScreen::setupConnections() {
    connect(btnFight, &QPushButton::clicked, this, [=]() {
        if (isAnimating) return;
        isAnimating = true;
        btnFight->blockSignals(true);
        btnSkill->blockSignals(true);
        btnItem->blockSignals(true);
        btnDefend->blockSignals(true);
        setButtonsEnabled(false);
        if (party.empty()) return;
        if (!enemies.empty() && !enemies[currentEnemyIndex]->isDead()) {
            playAttackAnimation(0);
        }
    });

    connect(btnSkill, &QPushButton::clicked, this, [=]() {
        if (isAnimating) return;
        if (party.empty()) return;
        isAnimating = true;
        setButtonsEnabled(false);
        dialogueText->setText(QString::fromStdString(party[0]->getName()) +
                              " uses their skill!");
        // TODO: implement skill animation
        isAnimating = false;
        setButtonsEnabled(true);
    });

    connect(btnItem, &QPushButton::clicked, this, [=]() {
        if (isAnimating) return;
        dialogueText->setText("Opening inventory...");
    });

    connect(btnDefend, &QPushButton::clicked, this, [=]() {
        if (isAnimating) return;
        isAnimating = true;
        setButtonsEnabled(false);
        dialogueText->setText("Fleeing from battle...");
        QTimer::singleShot(1000, [=]() {
            emit battleFinished(false);
        });
    });

    QShortcut *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(esc, &QShortcut::activated, [=]() {
        if (window()->isFullScreen())
            window()->showNormal();
        else
            window()->showFullScreen();
    });
}


void BattleScreen::updatePartyUI() {
    for (int i = 0; i < (int)party.size() && i < 3; i++) {
        if (!heroHpBars[i] || !heroHpText[i]) continue; // ← tambah ini
        int pct = (int)(100.0 * party[i]->getHp() / party[i]->getMaxHp());
        heroHpBars[i]->setValue(pct);
        heroHpText[i]->setText(QString::number(party[i]->getHp()) + "/" +
                               QString::number(party[i]->getMaxHp()));
        QString color = pct > 50 ? "#44cc44" : pct > 25 ? "#cccc44" : "#cc4444";
        heroHpBars[i]->setStyleSheet("QProgressBar{background:#2a2a2a;border-radius:4px;}"
                                     "QProgressBar::chunk{background:" + color + ";border-radius:4px;}");
    }
}

void BattleScreen::updateEnemyUI() {
    if (currentEnemyIndex >= (int)enemies.size()) return;
    Enemy* e = enemies[currentEnemyIndex];
    int pct = (int)(100.0 * e->getHp() / e->getMaxHp());
    enemyHpBar->setValue(pct);
    enemyHpText->setText(QString::number(e->getHp()) + "/" + QString::number(e->getMaxHp()));
    enemyNameLabel->setText(QString::fromStdString(e->getName()));
}

void BattleScreen::addBattleLog(const QString& entry) {
    battleLog->addItem(entry);
    battleLog->scrollToBottom();
    if (battleLog->count() > 8) delete battleLog->takeItem(0);
}

void BattleScreen::loadNextEnemy() {
    // Hide enemy yang baru mati
    enemySprites3[currentEnemyIndex - 0]->hide();  // hide sebelum increment
    enemyNameLabels[currentEnemyIndex]->hide();
    enemyHpBars3[currentEnemyIndex]->hide();

    isAnimating = false;
    setButtonsEnabled(true);
    btnFight->blockSignals(false);
    btnSkill->blockSignals(false);
    btnItem->blockSignals(false);
    btnDefend->blockSignals(false);

    currentEnemyIndex++;
    if (currentEnemyIndex >= (int)enemies.size()) {
        showVictoryScreen();  // ← ganti dari dialogueText
        return;
    }

    // Update pointer ke enemy baru
    enemySprite    = enemySprites3[currentEnemyIndex];
    enemyHpBar     = enemyHpBars3[currentEnemyIndex];
    enemyNameLabel = enemyNameLabels[currentEnemyIndex];

    enemySprite->setStyleSheet("background-color:#2a1a1a; color:#ff8888;"
                               "border-radius:" + QString::number((int)(65 * W/800.0f)/2) + "px;"
                                                                                "border:2px solid #cc4444; font-family:'Courier New';");
    updateEnemyUI();
    updateTurnOrder();
    addBattleLog("New enemy appeared: " + QString::fromStdString(enemies[currentEnemyIndex]->getName()));
}

// ── Animations ──
void BattleScreen::playAttackAnimation(int heroIndex) {

    qDebug() << "Party size:" << party.size() << "heroIndex:" << heroIndex;  // ← tambah ini
    if (heroIndex >= (int)party.size()) return;
    QLabel* sprite = heroSprites[heroIndex];
    QPoint origin  = sprite->pos();

    // Lunge forward
    QPropertyAnimation *lunge = new QPropertyAnimation(sprite, "pos");
    lunge->setDuration(150);
    lunge->setStartValue(origin);
    lunge->setEndValue(origin + QPoint(20, 0));
    lunge->start();

    connect(lunge, &QPropertyAnimation::finished, [=]() {
        // Return
        QPropertyAnimation *back = new QPropertyAnimation(sprite, "pos");
        back->setDuration(150);
        back->setStartValue(sprite->pos());
        back->setEndValue(origin);
        back->start();

        connect(back, &QPropertyAnimation::finished, [=]() {
            int dmg = party[heroIndex]->dealDamage();
            if (currentEnemyIndex < (int)enemies.size()) {
                enemies[currentEnemyIndex]->takeDamage(dmg);
                playEnemyHitFlash();
                playShakeAnimation(enemySprite);
                updateEnemyUI();
                updateTurnOrder();
                addBattleLog(QString::fromStdString(party[heroIndex]->getName()) +
                             " attacks for " + QString::number(dmg) + " dmg!");
                dialogueText->setText(QString::fromStdString(party[heroIndex]->getName()) +
                                      " attacks for <b>" + QString::number(dmg) + " damage!</b>");
                isAnimating = false;
                setButtonsEnabled(true);
                if (enemies[currentEnemyIndex]->isDead()) {
                    QTimer::singleShot(600, this, &BattleScreen::playDefeatAnimation);
                } else {
                    QTimer::singleShot(1000, this, [=]() {
                        playEnemyTurn();  // ← ganti ini, jangan enable buttons dulu
                    });
                }
            }
        });
    });
}

void BattleScreen::playEnemyTurn() {
    if (currentEnemyIndex >= (int)enemies.size()) return;
    Enemy* enemy = enemies[currentEnemyIndex];
    if (enemy->isDead()) return;

    // Pilih hero random yang masih hidup
    vector<int> aliveIndices;
    for (int i = 0; i < (int)party.size(); i++)
        if (!party[i]->isDead()) aliveIndices.push_back(i);
    if (aliveIndices.empty()) return;

    int targetIdx = aliveIndices[rand() % aliveIndices.size()];
    int dmg = enemy->dealDamage();
    party[targetIdx]->takeDamage(dmg);

    // Update dialogue
    dialogueText->setText(QString::fromStdString(enemy->getName()) +
                          " attacks <b>" + QString::fromStdString(party[targetIdx]->getName()) +
                          "</b> for <b>" + QString::number(dmg) + " damage!</b>");

    // Shake hero sprite
    playShakeAnimation(heroSprites[targetIdx]);

    // Update HP bar
    updatePartyUI();

    for (int i = 0; i < (int)party.size(); i++) {
        if (party[i]->isDead()) {
            QString deadPfpPaths[3] = {
                ":/assets/profilepic/ethan_pfp_dead.png",
                ":/assets/profilepic/mc_pfp_dead.png",
                ":/assets/profilepic/hubert_pfp_dead.png"
            };
            QPixmap deadPfp(deadPfpPaths[i]);
            if (!deadPfp.isNull())
                heroPfp[i]->setPixmap(deadPfp.scaled((int)(40 * W/800.0f), (int)(50 * H/600.0f),
                                                     Qt::KeepAspectRatio, Qt::FastTransformation));
        }
    }

    // Check semua hero mati
    bool allDead = true;
    for (auto h : party) if (!h->isDead()) { allDead = false; break; }
    if (allDead) {
        showDefeatScreen();
        return;
    }

    // Update Turn Order
    updateTurnOrder();

    // Jeda lalu enable buttons
    QTimer::singleShot(1000, this, [=]() {
        isAnimating = false;
        setButtonsEnabled(true);
        btnFight->blockSignals(false);
        btnSkill->blockSignals(false);
        btnItem->blockSignals(false);
        btnDefend->blockSignals(false);
    });
}

void BattleScreen::playEnemyHitFlash() {
    enemySprite->setStyleSheet("background-color:#cc4444; border-radius:6px;");
    QTimer::singleShot(200, [=]() {
        enemySprite->setStyleSheet("background-color:#2a1a1a; border-radius:6px;");
    });
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

void BattleScreen::setButtonsEnabled(bool enabled) {
    btnFight->setEnabled(enabled);
    btnSkill->setEnabled(enabled);
    btnItem->setEnabled(enabled);
    btnDefend->setEnabled(enabled);
}

void BattleScreen::updateTurnOrder() {

    vector<pair<QString, int>> order; // name, agi

    for (int i = 0; i < (int)party.size(); i++)
        if (!party[i]->isDead())
            order.push_back({QString::fromStdString(party[i]->getName()), party[i]->getAgi()});

    if (currentEnemyIndex < (int)enemies.size())
        if (!enemies[currentEnemyIndex]->isDead())
            order.push_back({QString::fromStdString(enemies[currentEnemyIndex]->getName()), enemies[currentEnemyIndex]->getAgi()});

    // Sort by AGI descending
    sort(order.begin(), order.end(), [](auto& a, auto& b){
        return a.second > b.second;
    });

    // Update labels
    for (int i = 0; i < 4; i++) {
        if (i < (int)order.size()) {
            turnOrderLabels[i]->setText(i == 0 ? "▶ " + order[i].first : "  " + order[i].first);
            turnOrderLabels[i]->setStyleSheet(i == 0 ?
                                                  "background:#1a1a00; border:1px solid #ffdd44; color:#ffdd44;"
                                                      + QString("font-size:%1px;").arg((int)(10 * W/800.0f)) + "font-family:'Courier New'; padding:2px 4px;" :
                                                  "background:#0d0d1a; border:1px solid #2a2a4a; color:#888888;"
                                                      + QString("font-size:%1px;").arg((int)(10 * W/800.0f)) + "font-family:'Courier New'; padding:2px 4px;");
        } else {
            turnOrderLabels[i]->setText("  —");
        }
    }
}
void BattleScreen::playMageAnimation(int heroIndex) { /* TODO: Member 4 */ }

void BattleScreen::showVictoryScreen() {
    // Overlay gelap
    QLabel *overlay = new QLabel(this);
    overlay->setGeometry(0, 0, W, H);
    overlay->setStyleSheet("background: rgba(0,0,0,180);");
    overlay->show();

    // Victory text
    QLabel *title = new QLabel("VICTORY!", this);
    title->setGeometry(W/2 - 200, H/2 - 100, 400, 80);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#ffdd44; font-size:52px; font-weight:bold; font-family:'Courier New';");
    title->show();

    QLabel *sub = new QLabel("All enemies defeated!", this);
    sub->setGeometry(W/2 - 200, H/2, 400, 40);
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color:white; font-size:18px; font-family:'Courier New';");
    sub->show();

    QPushButton *btnContinue = new QPushButton("CONTINUE", this);
    btnContinue->setGeometry(W/2 - 120, H/2 + 80, 240, 50);
    btnContinue->setStyleSheet("QPushButton{background:#1a1a2e; color:#ffdd44; border:2px solid #ffdd44;"
                               "font-family:'Courier New'; font-size:16px; font-weight:bold;}"
                               "QPushButton:hover{background:#ffdd44; color:#0d0d1a;}");
    btnContinue->show();

    connect(btnContinue, &QPushButton::clicked, [=]() {
        emit battleFinished(true);
    });

    setButtonsEnabled(false);
}

void BattleScreen::showDefeatScreen() {
    QLabel *overlay = new QLabel(this);
    overlay->setGeometry(0, 0, W, H);
    overlay->setStyleSheet("background: rgba(0,0,0,200);");
    overlay->show();

    QLabel *title = new QLabel("DEFEAT...", this);
    title->setGeometry(W/2 - 200, H/2 - 100, 400, 80);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#cc4444; font-size:52px; font-weight:bold; font-family:'Courier New';");
    title->show();

    QLabel *sub = new QLabel("Your party has been wiped out.", this);
    sub->setGeometry(W/2 - 200, H/2, 400, 40);
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color:white; font-size:18px; font-family:'Courier New';");
    sub->show();

    QPushButton *btnRetry = new QPushButton("RETRY", this);
    btnRetry->setGeometry(W/2 - 120, H/2 + 80, 240, 50);
    btnRetry->setStyleSheet("QPushButton{background:#1a1a2e; color:#cc4444; border:2px solid #cc4444;"
                            "font-family:'Courier New'; font-size:16px; font-weight:bold;}"
                            "QPushButton:hover{background:#cc4444; color:white;}");
    btnRetry->show();

    connect(btnRetry, &QPushButton::clicked, [=]() {
        emit battleFinished(false);
    });

    setButtonsEnabled(false);
}


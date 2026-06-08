#include "lobbyscreen.h"
#include "inventoryscreen.h"
#include "partysetupscreen.h"

#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QImage>

// A static helper function - This will help the paintEvent function
static QPixmap cropTransparentMargin(const QPixmap &src)
{
    if (src.isNull()) return src;
    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32);
    int minX = img.width(), minY = img.height(), maxX = -1, maxY = -1;
    for (int y = 0; y < img.height(); ++y) {
        const QRgb *row = reinterpret_cast<const QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            if (qAlpha(row[x]) > 8) {            // pixel is visible
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
    }
    if (maxX < minX) return src;
    return src.copy(minX, minY, maxX - minX + 1, maxY - minY + 1);
}

// PixmapButton
PixmapButton::PixmapButton(const QString &assetPath, QWidget *parent)
    : QPushButton(parent)
{
    setFlat(true);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background:transparent;border:none;");
    if (!assetPath.isEmpty()) setAsset(assetPath);
}

void PixmapButton::setAsset(const QString &assetPath)
{
    m_pixmap = cropTransparentMargin(loadAsset(assetPath));
    update();
}

void PixmapButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if (m_pixmap.isNull()) return;
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QSize target = m_pixmap.size().scaled(size(), Qt::KeepAspectRatio);
    QRect dest(QPoint(0, 0), target);
    dest.moveCenter(rect().center());
    p.drawPixmap(dest, m_pixmap);

    if (m_hovered && m_hoverOverlay)
        p.fillRect(rect(), QColor(255, 255, 255, 30));
}

void PixmapButton::enterEvent(QEnterEvent *e) { Q_UNUSED(e) m_hovered=true;  update(); }
void PixmapButton::leaveEvent(QEvent      *e) { Q_UNUSED(e) m_hovered=false; update(); }

// ═════════════════════════════════════════════════════════════════════════════
// LocationButton
// ═════════════════════════════════════════════════════════════════════════════

LocationButton::LocationButton(const LocationInfo &info, QWidget *parent)
    : QWidget(parent), m_info(info)
{
    setFixedSize(170, 170);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_TranslucentBackground);
    m_locationPx = loadAsset(info.assetPath);
    m_lockPx     = loadAsset("assets/lobbyscreen/lockDraw");
    m_arrowPx    = loadAsset("assets/lobbyscreen/tandaPanah");
    m_hoverAnim  = new QPropertyAnimation(this, "hoverAlpha", this);
    m_hoverAnim->setDuration(200);
    m_hoverAnim->setEasingCurve(QEasingCurve::InOutSine);
}

void LocationButton::setLocked(bool locked)   { m_locked=locked;     update(); }
void LocationButton::setIsCurrent(bool c)     { m_isCurrent=c;       update(); }

void LocationButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QRect r = rect();

    // draw island; if locked, dim ONLY the island pixels (no square box)
    if (!m_locationPx.isNull()) {
        if (m_locked) {
            QPixmap dimmed = m_locationPx;
            QPainter pp(&dimmed);
            pp.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            pp.fillRect(dimmed.rect(), QColor(0, 0, 0, 110));
            pp.end();
            p.drawPixmap(r, dimmed);
        } else {
            p.drawPixmap(r, m_locationPx);
        }
    }

    // lock icon on hover — KEEP
    if (m_locked && m_hovered && !m_lockPx.isNull()) {
        int lw=r.width()/2, lh=r.height()/2;
        int lx=r.center().x()-lw/2, ly=r.center().y()-lh/2;
        p.setOpacity(m_hoverAlpha);
        p.drawPixmap(QRect(lx,ly,lw,lh), m_lockPx);
        p.setOpacity(1.0);
    }

    // current-location arrow — KEEP
    if (m_isCurrent && !m_arrowPx.isNull()) {
        int aw = 40, ah = 40;
        int ax = r.center().x() - aw/2;
        int ay = r.top() + 90;
        p.drawPixmap(QRect(ax, ay, aw, ah), m_arrowPx);
    }

    // white hover highlight (unlocked) — KEEP
    if (!m_locked && m_hovered) {
        p.setOpacity(m_hoverAlpha * 0.22);
        p.fillRect(r, Qt::white);
        p.setOpacity(1.0);
    }
}

void LocationButton::enterEvent(QEnterEvent *e)
{
    Q_UNUSED(e) m_hovered=true;
    m_hoverAnim->stop();
    m_hoverAnim->setStartValue(m_hoverAlpha);
    m_hoverAnim->setEndValue(1.0);
    m_hoverAnim->start();
}

void LocationButton::leaveEvent(QEvent *e)
{
    Q_UNUSED(e) m_hovered=false;
    m_hoverAnim->stop();
    m_hoverAnim->setStartValue(m_hoverAlpha);
    m_hoverAnim->setEndValue(0.0);
    m_hoverAnim->start();
}

void LocationButton::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    if (m_locked) {
        QMessageBox::information(this, "Locked",
                                 QString("Complete Chapter %1 to unlock %2!")
                                     .arg(m_info.requiredChapter).arg(m_info.name));
        return;
    }
    emit clicked(m_info);
}

// ═════════════════════════════════════════════════════════════════════════════
// QuestPopup
// ═════════════════════════════════════════════════════════════════════════════

QuestPopup::QuestPopup(const LocationInfo &info, QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
    , m_info(info)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    buildUi(info);
}

void QuestPopup::buildUi(const LocationInfo &info)
{
    setFixedSize(430, 560);

    //1. Scroll background
    m_bgLabel = new QLabel(this);
    m_bgLabel->setGeometry(0, 0, 430, 560);
    QPixmap bgPx = loadAsset("assets/lobbyscreen/popUpQuest");
    if (!bgPx.isNull())
        m_bgLabel->setPixmap(bgPx.scaled(450, 680,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));
    else
        m_bgLabel->setStyleSheet(
            "background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            "stop:0 #2a1a0e,stop:1 #1a0e06);"
            "border-radius:16px;border:2px solid rgba(180,120,60,0.6);");

    m_bgLabel->setAlignment(Qt::AlignCenter);
    m_bgLabel->lower();

    //2. Close button
    auto *closeBtn = new QPushButton("✕", this);
    closeBtn->setGeometry(372, 58, 28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton{background:rgba(0,0,0,0.50);color:white;border-radius:14px;"
        "font-weight:bold;font-size:13px;border:none;}"
        "QPushButton:hover{background:rgba(200,50,50,0.9);}");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    closeBtn->raise();

    //3. Location name — below baked-in QUEST graphic
    auto *locNameLbl = new QLabel(info.name, this);
    locNameLbl->setGeometry(40, 165, 350, 28);
    locNameLbl->setAlignment(Qt::AlignCenter);
    locNameLbl->setStyleSheet(
        "color:#ffcc33;font-size:16px;font-weight:bold;"
        "font-family:'Bahnschrift','Trebuchet MS',sans-serif;"
        "letter-spacing:1px;background:transparent;");
    locNameLbl->raise();

    //4. bg_chapter banner  ── DELETED: this was the grey sliver you circled.
    //        (the bg_chapter pixmap's left edge poked past the scroll onto the island)
    //        m_chapterBg is no longer created; leaving the member unused is harmless.

    //5. "Chapter X"
    m_chapterLbl = new QLabel(this);
    m_chapterLbl->setGeometry(130, 203, 170, 34);
    m_chapterLbl->setAlignment(Qt::AlignCenter);
    m_chapterLbl->setStyleSheet(
        "color:#5a2d00;font-size:15px;font-weight:bold;"
        "background:rgba(90,45,0,0.12);"
        "border-radius:10px;padding:3px 0;");
    m_chapterLbl->raise();

    //clickable overlay where the chapter line sits — acts as START
    auto *chapterBtn = new QPushButton(this);
    chapterBtn->setGeometry(115, 205, 200, 36);
    chapterBtn->setStyleSheet(
        "QPushButton{background:transparent;border:none;cursor:pointer;}"
        "QPushButton:hover{background:rgba(255,215,0,0.15);border-radius:6px;}");
    chapterBtn->setCursor(Qt::PointingHandCursor);
    chapterBtn->setToolTip("Start this chapter");
    connect(chapterBtn, &QPushButton::clicked, this, [this](){
        emit playRequested(m_info.id);
        accept();
    });
    chapterBtn->raise();

    //6. Quest text
    m_questText = new QLabel(this);
    m_questText->setGeometry(105, 255, 220, 240);
    m_questText->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_questText->setWordWrap(true);
    m_questText->setStyleSheet(
        "color:#2a1a0a;font-size:13px;font-weight:500;"
        "background:transparent;padding:6px 14px;");
    m_questText->raise();

    //7. Page indicator
    m_pageLbl = new QLabel(this);
    m_pageLbl->setGeometry(175, 505, 80, 20);
    m_pageLbl->setAlignment(Qt::AlignCenter);
    m_pageLbl->setStyleSheet(
        "color:rgba(60,30,10,0.65);font-size:11px;background:transparent;");
    m_pageLbl->raise();

    //8. Left nav
    m_leftBtn = new PixmapButton("assets/lobbyscreen/kiri_popupquest", this);
    m_leftBtn->setGeometry(45, 255, 40, 40);
    m_leftBtn->setToolTip("Previous");
    connect(m_leftBtn, &QPushButton::clicked, this, &QuestPopup::prevPage);
    m_leftBtn->raise();

    //9. Right nav
    m_rightBtn = new PixmapButton("assets/lobbyscreen/kanan_popupquest", this);
    m_rightBtn->setGeometry(345, 255, 40, 40);
    m_rightBtn->setToolTip("Next");
    connect(m_rightBtn, &QPushButton::clicked, this, &QuestPopup::nextPage);
    m_rightBtn->raise();

    updatePage();
}

void QuestPopup::prevPage()
{ if (m_page > 0) { --m_page; updatePage(); } }

void QuestPopup::nextPage()
{ if (m_page < m_info.chapters.size()-1) { ++m_page; updatePage(); } }

void QuestPopup::updatePage()
{
    m_chapterLbl->setText(QString(" CHAPTER %1 ").arg(m_page + 1));

    m_questText->setText(
        (m_page < m_info.chapters.size())
            ? m_info.chapters[m_page]
            : m_info.description);

    m_leftBtn ->setVisible(m_page > 0);
    m_rightBtn->setVisible(m_page < m_info.chapters.size() - 1);

    m_pageLbl->setText(QString("%1 / %2")
                           .arg(m_page + 1)
                           .arg(qMax(1, m_info.chapters.size())));
}

// ═════════════════════════════════════════════════════════════════════════════
// SaveSlotPopup
// ═════════════════════════════════════════════════════════════════════════════

SaveSlotPopup::SaveSlotPopup(SaveManager &saveManager,
                             QWidget *parent)
    : QDialog(parent)
    , m_saveManager(saveManager)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setModal(true);

    setFixedSize(420, 320);

    setStyleSheet(
        "QDialog{"
        "background:#eb94a7;"
        "border:6px solid #fff29e;"
        "border-radius:16px;"
        "}"

        "QLabel{"
        "color:white;"
        "background:transparent;"
        "}"

        "QPushButton{"
        "background:#6ba0db;"
        "color:white;"
        "font-size:15px;"
        "font-weight:bold;"
        "border:2px solid #6ea8ff;"
        "border-radius:12px;"
        "padding:10px;"
        "}"

        "QPushButton:hover{"
        "background:#3d78d8;"
        "}"
        );

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(25, 25, 25, 25);
    layout->setSpacing(15);

    QLabel *title = new QLabel("Choose Save Slot");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "font-size:22px;"
        "font-weight:bold;"
        "color:#dbe9ff;"
        );

    layout->addWidget(title);

    for (int i = 0; i < 3; i++)
    {
        QString text;

        if (m_saveManager.isSlotEmpty(i))
        {
            text = QString("SLOT %1   •   EMPTY")
                       .arg(i + 1);
        }
        else
        {
            text = QString("SLOT %1   •   OCCUPIED")
                       .arg(i + 1);
        }

        m_slotBtns[i] = new QPushButton(text);

        connect(m_slotBtns[i],
                &QPushButton::clicked,
                this,
                [this, i]()
                {
                    emit slotChosen(i);
                    accept();
                });

        layout->addWidget(m_slotBtns[i]);
    }

    layout->addStretch();
}

// LobbyScreen
LobbyScreen::LobbyScreen(PlayerRecord   &playerRecord,
                         vector<Character*> allCharacters,
                         Shop           &shop,
                         Inventory      &inventory,
                         int            &coins,
                         SaveManager    &saveManager,
                         PartyManager   &partyManager,
                         int             saveSlot,
                         QWidget        *parent)
    : QWidget(parent)
    , m_record(playerRecord)
    , m_allCharacters(allCharacters)
    , m_shop(shop)
    , m_inventory(inventory)
    , m_coins(coins)
    , m_saveManager(saveManager)
    , m_partyManager(partyManager)
    , m_saveSlot(saveSlot)
{
    setMinimumSize(900, 600);
    buildLocations();
    buildUi();
    updateLockStates();
    animateEntrance();
}

// ── Locations
void LobbyScreen::buildLocations()
{
    m_locations = {
        {
            "maple_forest", "Maple Forest",
            "Where Your Journey Begin",
            0,
            {
                "After a long time,\n"
                "you comeback to Dawn Town and went to the forest.\n"
                "Deep in the Maple Forest,\n"
                "what wait for you?",

                "• Finish Story Chapter 01\n"
                "• Win the competition\n"
                "Encounter wolf or what",

                "Chapter 3...\n"
                "Chapter description blablabla...."
            },
            "assets/lobbyscreen/mapleForest", 0.215f, 0.680f
        },
        {
            "dungeon", "The Dungeon",
            "Dark ruins, darker secrets.",
            1,
            {
                "Who runs the dungeon?\n\n"
                "A mysterious competition is held\n"
                "three floors underground.\n"
                "Discover the mastermind.",

                "• Enter the Dungeon\n"
                "• Reach Level B3\n"
                "• Defeat the Dungeon King"
            },
            "assets/lobbyscreen/dungeon", 0.434f, 0.58f
        },
        {
            "sun_castle", "Sun Castle",
            "A grand fortress of eternal light.",
            2,
            {
                "The Order of Dawn awaits...\n\n"
                "Sun Castle holds a relic that\n"
                "could change everything.\n"
                "Earn the Order's trust.",

                "• Chapter 01\n"
                "• Find the Sun Relic\n"
                "• Defeat the King."
            },
            "assets/lobbyscreen/sunCastle", 0.675f, 0.53f
        },
        {
            "peak_mountain", "Peak Mountain",
            "A treacherous summit, ancient secrets.",
            3,
            {
                "The summit calls...\n\n"
                "High above the clouds lies\n"
                "a crystal of immense power.\n"
                "Survive the climb.",

                "• Scale the North Face\n"
                "• Find the ??? Met ???\n"
                "• Defeat the Dragon."
            },
            "assets/lobbyscreen/peakMountain", 0.87f, 0.660f
        }
    };
}

// ── UI assembly
void LobbyScreen::buildUi()
{
    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(0,0,0,0);
    vl->setSpacing(0);

    buildTopBar();
    buildMapArea();
    buildBottomBar();

    vl->addWidget(m_topBar,    0);
    vl->addWidget(m_mapArea,   1);
    vl->addWidget(m_bottomBar, 0);
}

// ── Top bar
void LobbyScreen::buildTopBar()
{
    m_topBar = new QWidget(this);
    m_topBar->setFixedHeight(90);
    m_topBar->setAttribute(Qt::WA_TranslucentBackground);
    m_topBar->setStyleSheet("background:transparent;");

    auto *hl = new QHBoxLayout(m_topBar);
    hl->setContentsMargins(12, 4, 16, 4);
    hl->setSpacing(10);

    // ── Player ID button — empty asset, portrait + frame layered manually
    m_playerIdBtn = new PixmapButton("", m_topBar);
    m_playerIdBtn->setFixedSize(280, 78);
    m_playerIdBtn->setToolTip("Change character portrait");
    connect(m_playerIdBtn, &QPushButton::clicked, this, &LobbyScreen::onAvatarClicked);

    // Portrait — added FIRST so it sits behind the frame
    QLabel *portraitLbl = new QLabel(m_playerIdBtn);
    portraitLbl->setObjectName("portraitLbl");
    portraitLbl->setGeometry(16, 8, 62, 62);
    portraitLbl->setScaledContents(true);
    portraitLbl->setAttribute(Qt::WA_TransparentForMouseEvents);
    QPixmap defPortrait = loadAsset("assets/profilepic/mc_pfp");
    if (!defPortrait.isNull()) portraitLbl->setPixmap(defPortrait);

    // Frame — added AFTER portrait so Qt stacks it in front
    QLabel *frameLbl = new QLabel(m_playerIdBtn);
    frameLbl->setGeometry(0, 0, 280, 78);
    frameLbl->setScaledContents(true);
    frameLbl->setAttribute(Qt::WA_TransparentForMouseEvents);
    QPixmap framePx = loadAsset("assets/lobbyscreen/playerID");
    if (!framePx.isNull()) frameLbl->setPixmap(framePx);

    // Player username label — on top of frame
    m_playerUsername = new QLabel(
        QString(m_record.playerUsername), m_playerIdBtn);
    m_playerUsername->setGeometry(81, 27, 90, 28); //change here for user ID layout fah
    m_playerUsername->setStyleSheet(
        "color:#2a4a6b;"
        "font-size:32px;"
        "font-weight:bold;"
        "font-family:'Courier New','Consolas',monospace;"
        "background:transparent;");
    m_playerUsername->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_playerUsername->raise();

    // UID label
    QLabel *uidLbl = new QLabel(QString(m_record.UID), m_playerIdBtn);
    uidLbl->setObjectName("uidLbl");
    uidLbl->setGeometry(183, 33, 120, 18);
    uidLbl->setStyleSheet(
        "color:rgba(60,80,120,0.75);font-size:18px;"
        "background:transparent;letter-spacing:1px;");
    uidLbl->setAttribute(Qt::WA_TransparentForMouseEvents);
    uidLbl->raise();

    hl->addWidget(m_playerIdBtn);

    // Edit name pencil button
    QPushButton *editBtn = new QPushButton("✏", m_topBar);
    editBtn->setFixedSize(26, 26);
    editBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setToolTip("Edit name");
    editBtn->setStyleSheet(
        "QPushButton{background:rgba(255,255,255,0.15);border:none;"
        "border-radius:13px;color:white;font-size:13px;}"
        "QPushButton:hover{background:rgba(255,255,255,0.30);}");
    connect(editBtn, &QPushButton::clicked, this, &LobbyScreen::onEditNameClicked);
    hl->addWidget(editBtn);

    hl->addStretch(1);

    // ── Stat badges
    auto makeBadge = [&](QLabel *&bg, QLabel *&icon, QLabel *&val,
                         const QString &symbol, const QString &text)
    {
        bg = new QLabel(m_topBar);
        bg->setFixedSize(200, 64);
        bg->setStyleSheet(
            "background:rgba(238, 228, 200, 0.94);"
            "border-radius:18px;"
            "border:2px solid rgba(150, 110, 60, 0.6);");

        icon = new QLabel(symbol, bg);              //text/emoji symbol
        icon->setGeometry(12, 14, 36, 36);
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet("background:transparent;font-size:22px;");
        icon->setAttribute(Qt::WA_TransparentForMouseEvents);

        val = new QLabel(text, bg);
        val->setGeometry(54, 16, 138, 32);
        val->setStyleSheet(
            "color:#2a4a6b;font-size:15px;font-weight:bold;background:transparent;");
        val->setAttribute(Qt::WA_TransparentForMouseEvents);
    };

    makeBadge(m_energyBg, m_energyIcon, m_energyVal, "⚡", "Energy: 100");
    hl->addWidget(m_energyBg);
    hl->addSpacing(10);

    makeBadge(m_coinsBg, m_coinsIcon, m_coinsVal, "🪙", QString::number(m_coins));
    hl->addWidget(m_coinsBg);
}

// ── Map area
void LobbyScreen::buildMapArea()
{
    m_mapArea = new QWidget(this);
    m_mapArea->setStyleSheet("background:transparent;");
    m_mapArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_bgLabel = new QLabel(m_mapArea);
    m_bgLabel->setScaledContents(true);
    QPixmap bgPx = loadAsset("assets/lobbyscreen/background_lobby");
    if (!bgPx.isNull())
        m_bgLabel->setPixmap(bgPx);
    else
        m_bgLabel->setStyleSheet(
            "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 #0d1b2a,stop:1 #1a2744);");
    m_bgLabel->lower();
      m_bgLabel->hide();
    m_bgLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    for (int i = 0; i < m_locations.size(); ++i) {
        auto *btn = new LocationButton(m_locations[i], m_mapArea);
        connect(btn, &LocationButton::clicked,
                this, &LobbyScreen::onLocationClicked);
        m_locBtns.append(btn);
    }

    buildSidePanel();
    positionLocations();
}

// ── Side panel
void LobbyScreen::buildSidePanel()
{
    auto makeBtn = [&](PixmapButton *&btn, const QString &asset, const QString &tip){
        btn = new PixmapButton(asset, m_mapArea);
        btn->setFixedSize(80, 80);
        btn->setToolTip(tip);
    };
    makeBtn(m_shopBtn,  "assets/lobbyscreen/shopDraw",  "Shop");
    makeBtn(m_farmBtn,  "assets/lobbyscreen/farmDraw",  "Farm");
    makeBtn(m_questBtn, "assets/lobbyscreen/questDraw", "Quests");

    connect(m_shopBtn,  &QPushButton::clicked, this, &LobbyScreen::onShopClicked);
    connect(m_farmBtn,  &QPushButton::clicked, this, &LobbyScreen::onFarmClicked);
    connect(m_questBtn, &QPushButton::clicked, this, &LobbyScreen::onQuestClicked);

    auto makeSideLbl = [&](const QString &text) -> QLabel* {
        QLabel *lbl = new QLabel(text, m_mapArea);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setFixedWidth(80);
        lbl->setStyleSheet(
            "color:#ffd700;font-size:10px;font-weight:bold;"
            "font-family:'Courier New',monospace;"
            "letter-spacing:2px;background:transparent;");
        return lbl;
    };
    m_shopLbl  = makeSideLbl("SHOP");
    m_farmLbl  = makeSideLbl("FARM");
    m_questLbl = makeSideLbl("QUEST");
}

// Bottom bar
// ── Bottom bar  [ SAVE ][ INV ][ PARTY ][ .....stretch..... ][ PLAY ]
void LobbyScreen::buildBottomBar()
{
    m_bottomBar = new QWidget(this);
    m_bottomBar->setFixedHeight(230);              // taller so the bigger PLAY fits
    m_bottomBar->setAttribute(Qt::WA_TranslucentBackground);
    m_bottomBar->setStyleSheet("background:transparent;");

    auto *hl = new QHBoxLayout(m_bottomBar);
    hl->setContentsMargins(16, 8, 6, 8);
    hl->setSpacing(0);

    auto makeBtn = [&](PixmapButton *&btn, const QString &asset, int w, int h, const QString &tip){
        btn = new PixmapButton(asset, m_bottomBar);
        btn->setFixedSize(w, h);
        btn->setToolTip(tip);
    };

    auto lowered = [&](QWidget *w) -> QVBoxLayout* {
        auto *v = new QVBoxLayout;
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(0);
        v->addStretch(1);
        v->addWidget(w);
        return v;
    };

    // ── trio on the LEFT (no leading stretch)
    makeBtn(m_saveBtn, "assets/lobbyscreen/saveDraw", 175, 80, "Save");
    hl->addLayout(lowered(m_saveBtn));
    connect(m_saveBtn, &QPushButton::clicked, this, &LobbyScreen::onSaveClicked);

    hl->addSpacing(14);

    makeBtn(m_inventoryBtn, "assets/lobbyscreen/invenDraw", 215, 88, "Inventory");
    hl->addLayout(lowered(m_inventoryBtn));
    connect(m_inventoryBtn, &QPushButton::clicked, this, &LobbyScreen::onInventoryClicked);

    hl->addSpacing(14);

    makeBtn(m_partyBtn, "assets/lobbyscreen/partyDraw", 175, 80, "Party Setup");
    hl->addLayout(lowered(m_partyBtn));
    connect(m_partyBtn, &QPushButton::clicked, this, &LobbyScreen::onPartyClicked);

    hl->addStretch(1);   // THE ONLY STRETCH — now AFTER party, pushes PLAY to the right

    // ── bigger PLAY on the right
    makeBtn(m_playBtn, "assets/lobbyscreen/pB", 230, 210, "Play");
    hl->addLayout(lowered(m_playBtn));
    connect(m_playBtn, &QPushButton::clicked, this, [this](){
        for (const LocationInfo &loc : m_locations)
            if (loc.requiredChapter == m_record.currentChapter) {
                onLocationClicked(loc);          //opens the same QuestPopup as clicking the island
                return;
            }
        onLocationClicked(m_locations[0]);       //fallback: first location
    });
}

// ── Position children
void LobbyScreen::positionLocations()
{
    int mw = m_mapArea->width();
    int mh = m_mapArea->height();
    if (mw < 10 || mh < 10) return;

    m_bgLabel->setGeometry(0, 0, mw, mh);

    const int BW = 170, BH = 170;
    const double ANCHOR = 0.72;   // 0.5 = center; higher = sprite sits up, base lands on the click point
    for (int i = 0; i < m_locBtns.size(); ++i) {
        int cx = int(m_locations[i].relX * mw);
        int cy = int(m_locations[i].relY * mh);
        m_locBtns[i]->move(cx - BW/2, cy - int(BH * ANCHOR));   // CHANGED: anchor lower in the sprite
        m_locBtns[i]->raise();
    }

    int sideMidY = mh / 2;
    const int ICON_X = 14, ICON_H = 80, LBL_GAP = 2;

    m_shopBtn ->move(ICON_X, sideMidY - 220); m_shopBtn ->raise();
    m_farmBtn ->move(ICON_X, sideMidY - 110); m_farmBtn ->raise();
    m_questBtn->move(ICON_X, sideMidY + 0);   m_questBtn->raise();

    if (m_shopLbl)  { m_shopLbl ->move(ICON_X, sideMidY - 220 + ICON_H + LBL_GAP); m_shopLbl ->raise(); }
    if (m_farmLbl)  { m_farmLbl ->move(ICON_X, sideMidY - 110 + ICON_H + LBL_GAP); m_farmLbl ->raise(); }
    if (m_questLbl) { m_questLbl->move(ICON_X, sideMidY + 0   + ICON_H + LBL_GAP); m_questLbl->raise(); }
}

// ── Lock states
void LobbyScreen::updateLockStates()
{
    for (int i=0; i<m_locBtns.size(); ++i) {
        m_locBtns[i]->setLocked   (m_locations[i].requiredChapter > m_record.currentChapter);
        m_locBtns[i]->setIsCurrent(m_locations[i].requiredChapter == m_record.currentChapter);
    }
}

// ── Entrance animation
void LobbyScreen::animateEntrance()
{
    auto *fx = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(fx);
    auto *a = new QPropertyAnimation(fx, "opacity", this);
    a->setDuration(500);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::OutCubic);
    connect(a, &QPropertyAnimation::finished, this, [this](){
        setGraphicsEffect(nullptr);
    });
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

// ── paintEvent
void LobbyScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    if (m_bgLabel && !m_bgLabel->pixmap().isNull()) {
        p.drawPixmap(rect(), m_bgLabel->pixmap());
    } else {
        QLinearGradient g(0, 0, 0, height());
        g.setColorAt(0, QColor(0x0D, 0x1B, 0x2A));
        g.setColorAt(1, QColor(0x05, 0x0E, 0x1A));
        p.fillRect(rect(), g);
    }
}

void LobbyScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    QTimer::singleShot(0, this, &LobbyScreen::positionLocations);
}

// ── refreshStats
void LobbyScreen::refreshStats()
{
    m_playerUsername->setText(QString(m_record.playerUsername));
    m_energyVal ->setText("Energy: 100");
    m_coinsVal  ->setText(QString::number(m_coins));
    updateLockStates();
    if (m_questScreen) m_questScreen->setPlayerChapter(m_record.currentChapter);
}

// ═════════════════════════════════════════════════════════════════════════════
// Slots
// ═════════════════════════════════════════════════════════════════════════════

void LobbyScreen::onLocationClicked(const LocationInfo &info)
{
    auto *popup = new QuestPopup(info, this);
    connect(popup, &QuestPopup::playRequested,
            this,  &LobbyScreen::onPlayFromPopup);
    QPoint c = mapToGlobal(rect().center());
    popup->move(c.x()-popup->width()/2, c.y()-popup->height()/2);
    popup->exec();
    popup->deleteLater();
}

void LobbyScreen::onPlayFromPopup(const QString &locationId)
{
    emit battleRequested(locationId);
}

void LobbyScreen::onQuestClicked()
{
    if (!m_questScreen) {
        m_questScreen = new QuestScreen(m_record.currentChapter, this);
        m_questScreen->setWindowFlags(Qt::Dialog);
        m_questScreen->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_questScreen, &QDialog::destroyed,
                this, [this](){ m_questScreen = nullptr; });
    }
    m_questScreen->setPlayerChapter(m_record.currentChapter);
    m_questScreen->show();
    m_questScreen->raise();
    m_questScreen->activateWindow();
}

void LobbyScreen::onShopClicked()
{
    auto *screen = new ShopScreen(m_shop, m_inventory, m_coins, m_partyManager, m_record.currentChapter, this);
    screen->setWindowFlags(Qt::Window);
    screen->setAttribute(Qt::WA_DeleteOnClose);
    screen->resize(parentWidget() ? parentWidget()->size() : QSize(1080, 720));
    screen->showMaximized();
}

void LobbyScreen::onFarmClicked()
{
    QMessageBox::information(this, "Farm",
                             "Welcome to the Farm!\nPlant crops and harvest resources here.");
}

void LobbyScreen::onInventoryClicked()
{
    auto *screen = new InventoryScreen(m_inventory, this);
    screen->setWindowFlags(Qt::Dialog);
    screen->setAttribute(Qt::WA_DeleteOnClose);
    connect(screen, &InventoryScreen::inventoryClosed,
            this, &LobbyScreen::refreshStats);
    screen->showMaximized();

}

void LobbyScreen::onPartyClicked()
{
    auto *screen = new PartySetupScreen(m_allCharacters, m_partyManager, this);
    screen->setAttribute(Qt::WA_DeleteOnClose);
    connect(screen, &PartySetupScreen::partyConfirmed,
            this,   [](vector<Character*>){});
    screen->showMaximized();
}

void LobbyScreen::onSaveClicked()
{
    SaveSlotPopup popup(m_saveManager, this);

    connect(&popup,
            &SaveSlotPopup::slotChosen,
            this,
            [this](int slot)
            {
                if (!m_saveManager.isSlotEmpty(slot))
                {
                    auto reply =
                        QMessageBox::question(
                            this,
                            "Overwrite Save",
                            QString("Overwrite Slot %1?")
                                .arg(slot + 1));

                    if (reply != QMessageBox::Yes)
                        return;
                }

                m_saveManager.saveGame(
                    slot,
                    m_record,
                    m_inventory,
                    m_shop,
                    m_partyManager);

                QMessageBox::information(
                    this,
                    "Saved",
                    QString("Game saved to Slot %1!")
                        .arg(slot + 1));
            });

    QPoint center = mapToGlobal(rect().center());

    popup.move(center.x() - popup.width()/2,
               center.y() - popup.height()/2);

    popup.exec();
}

// ── Avatar picker
void LobbyScreen::onAvatarClicked()
{
    if (m_avatarPicker) m_avatarPicker->close();

    m_avatarPicker = new AvatarPicker(this);
    m_avatarPicker->setCurrentIndex(
        m_record.avatarIndex >= 0 ? m_record.avatarIndex : 0);
    m_avatarPicker->setOwnedStates(
        m_record.characterUnlocked,
        sizeof(m_record.characterUnlocked) / sizeof(bool));
    connect(m_avatarPicker, &AvatarPicker::avatarSelected,
            this,           &LobbyScreen::onAvatarSelected);
    connect(m_avatarPicker, &QDialog::destroyed,
            this, [this](){ m_avatarPicker = nullptr; });
    m_avatarPicker->showBelow(m_playerIdBtn);
}

void LobbyScreen::onAvatarSelected(int index, const QString &assetPath, const QString &name)
{
    Q_UNUSED(name)
    m_record.avatarIndex = index;
    if (auto *lbl = m_playerIdBtn->findChild<QLabel*>("portraitLbl")) {
        QPixmap px = loadAsset(assetPath);
        if (!px.isNull()) lbl->setPixmap(px);
    }
}

void LobbyScreen::onEditNameClicked()
{
    bool ok;
    QString current = QString(m_record.playerUsername);
    QString newName = QInputDialog::getText(
        this, "Edit Name", "Enter your player name:",
        QLineEdit::Normal, current, &ok);

    if (!ok || newName.trimmed().isEmpty()) return;

    strncpy(m_record.playerUsername,
            newName.trimmed().toUtf8().constData(),
            sizeof(m_record.playerUsername) - 1);
    m_record.playerUsername[sizeof(m_record.playerUsername) - 1] = '\0';

    m_playerUsername->setText(newName.trimmed());
}

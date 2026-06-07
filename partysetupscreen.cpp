#include "partysetupscreen.h"

#include <QPainter>
#include <QMessageBox>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
#include <QFrame>
#include <QDebug>

// ─────────────────────────────────────────────────────────────────────────────
// Character path table
// ─────────────────────────────────────────────────────────────────────────────
static QVector<QPair<QString,QString>> s_paths = {
    { "MC",         "mc"     },
    { "Ethan",      "ethan"  },
    { "Hubert",     "hubert" },
    { "Zey",        "zey"    },
    { "Kae",        "kae"    },
    { "Lynn",       "lynn"   },
    { "Cedric",     "ced"    },
    { "Den",        "den"    },
    { "Anak Agung", "agung"  },
    };

static QString portraitPath(const QString &k)
{ return QString("assets/portraits/%1_neutral").arg(k); }

static QString sideartPath(const QString &k)
{ return QString("assets/sideart/%1_pixelSide").arg(k); }

// ═════════════════════════════════════════════════════════════════════════════
// PartySlot  –  top row, shows 2d_character frame + sideart when filled
// ═════════════════════════════════════════════════════════════════════════════

PartySlot::PartySlot(int slotIndex, QWidget *parent)
    : QWidget(parent), m_slotIndex(slotIndex)
{
    setFixedSize(240, 460);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_TranslucentBackground);

    // 2d_character frame — always visible, behind everything
    m_bgLbl = new QLabel(this);
    m_bgLbl->setGeometry(0, 0, 240, 460);
    m_bgLbl->setScaledContents(true);
    QPixmap frame = loadAsset("assets/partysetup/2d_character");
    if (!frame.isNull())
        m_bgLbl->setPixmap(frame);
    else
        m_bgLbl->setStyleSheet(
            "background:rgba(20,15,50,0.80);"
            "border:2px solid rgba(255,215,0,0.55);"
            "border-radius:14px;");
    m_bgLbl->lower();

    // Sideart drawn inside the frame
    m_artLbl = new QLabel(this);
    m_artLbl->setGeometry(20, 40, 200, 340);
    m_artLbl->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    m_artLbl->setVisible(false);

    // Character name at bottom
    m_nameLbl = new QLabel(this);
    m_nameLbl->setGeometry(0, 395, 240, 30);
    m_nameLbl->setAlignment(Qt::AlignCenter);
    m_nameLbl->setObjectName("slotName");
    m_nameLbl->setVisible(false);

    // Empty hint
    m_emptyLbl = new QLabel("＋\nEmpty Slot", this);
    m_emptyLbl->setGeometry(0, 180, 240, 80);
    m_emptyLbl->setAlignment(Qt::AlignCenter);
    m_emptyLbl->setObjectName("slotEmpty");
}

void PartySlot::setCharacter(const CharDisplayInfo &info)
{
    m_isEmpty = false;
    m_charPtr = info.charPtr;

    QPixmap art = loadAsset(info.sideartPath);
    if (!art.isNull())
        m_artLbl->setPixmap(art.scaled(200, 340,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
    else
        m_artLbl->setText("🧙");

    m_nameLbl->setText(info.name);
    m_artLbl ->setVisible(true);
    m_nameLbl->setVisible(true);
    m_emptyLbl->setVisible(false);
    update();
}

void PartySlot::clearCharacter()
{
    m_isEmpty = true;
    m_charPtr = nullptr;
    m_artLbl ->setVisible(false);
    m_nameLbl->setVisible(false);
    m_emptyLbl->setVisible(true);
    update();
}

void PartySlot::mousePressEvent(QMouseEvent *)
{ emit slotClicked(m_slotIndex); }

void PartySlot::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    // Gold highlight ring when occupied
    if (!m_isEmpty) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(255,215,0,160), 3));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect().adjusted(3,3,-3,-3), 12, 12);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// RosterCard  –  bottom row, uses pixel_character frame
// ═════════════════════════════════════════════════════════════════════════════

RosterCard::RosterCard(const CharDisplayInfo &info, QWidget *parent)
    : QWidget(parent), m_info(info)
{
    setFixedSize(120, 150);
    setCursor(m_info.unlocked ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
    setAttribute(Qt::WA_TranslucentBackground);

    // pixel_character frame behind portrait
    QLabel *pixelFrame = new QLabel(this);
    pixelFrame->setGeometry(0, 0, 120, 150);
    pixelFrame->setScaledContents(true);
    QPixmap pxFrame = loadAsset("assets/partysetup/pixel_character");
    if (!pxFrame.isNull())
        pixelFrame->setPixmap(pxFrame);
    pixelFrame->lower();

    // Portrait centred inside the frame
    m_portraitLbl = new QLabel(this);
    m_portraitLbl->setGeometry(10, 8, 100, 105);
    m_portraitLbl->setAlignment(Qt::AlignCenter);

    QPixmap portrait = loadAsset(info.portraitPath);
    if (!portrait.isNull())
        m_portraitLbl->setPixmap(portrait.scaled(100, 105,
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation));
    else
        m_portraitLbl->setText("👤");

    // Dim portrait if locked
    if (!info.unlocked) {
        auto *fx = new QGraphicsOpacityEffect(m_portraitLbl);
        fx->setOpacity(0.30);
        m_portraitLbl->setGraphicsEffect(fx);
    }

    // Name label
    m_nameLbl = new QLabel(info.name, this);
    m_nameLbl->setGeometry(0, 116, 120, 28);
    m_nameLbl->setAlignment(Qt::AlignCenter);
    m_nameLbl->setObjectName("rosterName");
}

void RosterCard::setHighlighted(bool on)
{ m_highlighted = on; update(); }

void RosterCard::mousePressEvent(QMouseEvent *)
{
    if (!m_info.unlocked) return;
    emit cardClicked(m_info);
}

void RosterCard::enterEvent(QEnterEvent *)
{ if (m_info.unlocked) { m_hovered = true; update(); } }

void RosterCard::leaveEvent(QEvent *)
{ m_hovered = false; update(); }

void RosterCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Selection / hover ring
    if (m_highlighted) {
        p.setPen(QPen(QColor(255,215,0,220), 3));
        p.setBrush(QColor(255,215,0,30));
        p.drawRoundedRect(rect().adjusted(2,2,-2,-2), 8, 8);
    } else if (m_hovered) {
        p.setPen(QPen(QColor(255,255,255,100), 2));
        p.setBrush(QColor(255,255,255,15));
        p.drawRoundedRect(rect().adjusted(2,2,-2,-2), 8, 8);
    }

    // Lock icon overlay
    if (!m_info.unlocked) {
        p.setPen(QColor(255,215,0,200));
        QFont f = p.font(); f.setPointSize(20); p.setFont(f);
        p.drawText(QRect(0, 20, 120, 80), Qt::AlignCenter, "🔒");
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// PartySetupScreen
// ═════════════════════════════════════════════════════════════════════════════

PartySetupScreen::PartySetupScreen(vector<Character*> allCharacters,
                                   PartyManager      &partyManager,
                                   QWidget           *parent)
    : QDialog(parent)
    , m_allCharacters(allCharacters)
    , m_partyManager(partyManager)
{
    setWindowTitle("Party Setup");
    setObjectName("partyScreen");
    setMinimumSize(1100, 720);

    buildCharList();
    buildUi();
    applyStyle();
    refreshSlots();
}

// ── Build character display list ──────────────────────────────────────────────
void PartySetupScreen::buildCharList()
{
    for (auto &pair : s_paths) {
        CharDisplayInfo info;
        info.name         = pair.first;
        info.portraitPath = portraitPath(pair.second);
        info.sideartPath  = sideartPath(pair.second);
        info.charPtr      = nullptr;
        info.unlocked     = false;

        for (Character *c : m_allCharacters) {
            if (QString::fromStdString(c->getName()) == info.name) {
                info.charPtr  = c;
                info.unlocked = m_partyManager.isUnlocked(c);
                break;
            }
        }
        m_charList.append(info);
    }
}

// ── Build UI ──────────────────────────────────────────────────────────────────
void PartySetupScreen::buildUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    root->setSpacing(12);

    // ── Close button top-right
    QHBoxLayout *topRow = new QHBoxLayout;
    topRow->addStretch();
    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setObjectName("partyCloseBtn");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    topRow->addWidget(closeBtn);
    root->addLayout(topRow);

    // ── Middle: slots + stats panel
    QHBoxLayout *middle = new QHBoxLayout;
    middle->setSpacing(20);

    buildPartySlots();
    QHBoxLayout *slotsRow = new QHBoxLayout;
    slotsRow->setSpacing(16);
    slotsRow->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    for (PartySlot *s : m_slots)
        slotsRow->addWidget(s);

    buildStatsPanel();

    middle->addLayout(slotsRow, 1);
    middle->addWidget(m_statsPanel, 0, Qt::AlignTop);
    root->addLayout(middle, 1);

    // ── Roster header
    QLabel *rosterHdr = new QLabel("ROSTER", this);
    rosterHdr->setObjectName("rosterHeader");
    root->addWidget(rosterHdr);

    // ── Roster scroll
    buildRoster(root);

    // ── Bottom buttons
    buildBottomBar(root);
}

void PartySetupScreen::buildPartySlots()
{
    for (int i = 0; i < 3; ++i) {
        PartySlot *slot = new PartySlot(i, this);
        connect(slot, &PartySlot::slotClicked,
                this, &PartySetupScreen::onSlotClicked);
        m_slots.append(slot);
    }
}

void PartySetupScreen::buildStatsPanel()
{
    m_statsPanel = new QWidget(this);
    m_statsPanel->setObjectName("statsPanel");
    m_statsPanel->setFixedSize(280, 420);

    // Clean dark background — no asset overlay so text is readable
    QLabel *bg = new QLabel(m_statsPanel);
    bg->setGeometry(0, 0, 280, 420);
    bg->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "stop:0 #1c1640,stop:1 #0e0b20);"
        "border:2px solid rgba(255,215,0,0.50);"
        "border-radius:14px;");
    bg->lower();

    // Portrait — top centre
    m_statsPortrait = new QLabel(m_statsPanel);
    m_statsPortrait->setGeometry(90, 14, 100, 100);
    m_statsPortrait->setAlignment(Qt::AlignCenter);
    m_statsPortrait->setScaledContents(false);
    m_statsPortrait->setObjectName("statsPortrait");

    // Character name
    m_statsName = new QLabel("Select a character", m_statsPanel);
    m_statsName->setGeometry(10, 120, 260, 28);
    m_statsName->setAlignment(Qt::AlignCenter);
    m_statsName->setObjectName("statsName");

    // Divider line
    QFrame *div = new QFrame(m_statsPanel);
    div->setGeometry(20, 154, 240, 1);
    div->setStyleSheet("background:rgba(255,215,0,0.35);");

    // Stat rows — clean, evenly spaced
    auto makeStatRow = [&](QLabel *&lbl, int y, const QString &icon, const QString &label){
        QLabel *iconLbl = new QLabel(icon, m_statsPanel);
        iconLbl->setGeometry(18, y, 24, 28);
        iconLbl->setAlignment(Qt::AlignCenter);
        iconLbl->setStyleSheet("background:transparent;font-size:14px;");

        QLabel *keyLbl = new QLabel(label, m_statsPanel);
        keyLbl->setGeometry(46, y, 60, 28);
        keyLbl->setObjectName("statKey");

        lbl = new QLabel("—", m_statsPanel);
        lbl->setGeometry(110, y, 150, 28);
        lbl->setObjectName("statVal");
    };

    makeStatRow(m_statsHp,  168, "❤", "HP");
    makeStatRow(m_statsAtk, 210, "⚔", "ATK");
    makeStatRow(m_statsDef, 252, "🛡", "DEF");
    makeStatRow(m_statsSpd, 294, "💨", "SPD");
}

void PartySetupScreen::buildRoster(QVBoxLayout *root)
{
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setObjectName("rosterScroll");
    scroll->setFixedHeight(178);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *rosterWidget = new QWidget;
    QHBoxLayout *hl = new QHBoxLayout(rosterWidget);
    hl->setContentsMargins(6, 6, 6, 6);
    hl->setSpacing(10);

    for (auto &info : m_charList) {
        RosterCard *card = new RosterCard(info, rosterWidget);
        connect(card, &RosterCard::cardClicked,
                this, &PartySetupScreen::onRosterCardClicked);
        m_rosterCards.append(card);
        hl->addWidget(card);
    }
    hl->addStretch();

    scroll->setWidget(rosterWidget);
    root->addWidget(scroll);
}

void PartySetupScreen::buildBottomBar(QVBoxLayout *root)
{
    QHBoxLayout *hl = new QHBoxLayout;
    hl->setSpacing(16);
    hl->addStretch();

    m_quickSetupBtn = new PixmapButton("assets/partysetup/quickSetup_party", this);
    m_quickSetupBtn->setFixedSize(180, 56);
    m_quickSetupBtn->setToolTip("Quick Setup — auto fill party");
    connect(m_quickSetupBtn, &QPushButton::clicked,
            this, &PartySetupScreen::onQuickSetup);

    m_replaceBtn = new PixmapButton("assets/partysetup/replace_party", this);
    m_replaceBtn->setFixedSize(160, 56);
    m_replaceBtn->setToolTip("Clear selected slot");
    connect(m_replaceBtn, &QPushButton::clicked,
            this, &PartySetupScreen::onReplace);

    m_deployBtn = new PixmapButton("assets/partysetup/deploy_party", this);
    m_deployBtn->setFixedSize(180, 56);
    m_deployBtn->setToolTip("Confirm and deploy party");
    connect(m_deployBtn, &QPushButton::clicked,
            this, &PartySetupScreen::onDeploy);

    hl->addWidget(m_quickSetupBtn);
    hl->addWidget(m_replaceBtn);
    hl->addWidget(m_deployBtn);

    root->addLayout(hl);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void PartySetupScreen::onSlotClicked(int slotIndex)
{
    m_selectedSlot = slotIndex;

    if (!m_slots[slotIndex]->isEmpty()) {
        Character *c = m_slots[slotIndex]->character();
        for (auto &info : m_charList) {
            if (info.charPtr == c) {
                updateStatsPanel(info);
                break;
            }
        }
    }
}

void PartySetupScreen::onRosterCardClicked(const CharDisplayInfo &info)
{
    // Highlight selected card
    for (int i = 0; i < m_rosterCards.size(); ++i) {
        bool match = (m_rosterCards[i]->info().name == info.name);
        m_rosterCards[i]->setHighlighted(match);
        if (match) m_selectedRoster = i;
    }

    updateStatsPanel(info);

    // Assign to selected slot
    if (m_selectedSlot >= 0 && m_selectedSlot < m_slots.size()) {
        // Check if already in another slot
        for (int i = 0; i < m_slots.size(); ++i) {
            if (!m_slots[i]->isEmpty() &&
                m_slots[i]->character() == info.charPtr &&
                i != m_selectedSlot) {
                QMessageBox::information(this, "Already in Party",
                                         info.name + " is already in slot "
                                             + QString::number(i + 1) + "!");
                return;
            }
        }
        m_slots[m_selectedSlot]->setCharacter(info);

        // Auto-advance to next empty slot
        for (int i = 0; i < m_slots.size(); ++i) {
            if (m_slots[i]->isEmpty()) {
                m_selectedSlot = i;
                return;
            }
        }
        m_selectedSlot = -1;
    }
}

void PartySetupScreen::onQuickSetup()
{
    int filled = 0;
    for (auto &info : m_charList) {
        if (!info.unlocked || filled >= 3) continue;
        m_slots[filled]->setCharacter(info);
        filled++;
    }
    if (filled == 0)
        QMessageBox::information(this, "Quick Setup",
                                 "No unlocked characters available!");
}

void PartySetupScreen::onReplace()
{
    if (m_selectedSlot >= 0 && m_selectedSlot < m_slots.size())
        m_slots[m_selectedSlot]->clearCharacter();
    else
        for (PartySlot *s : m_slots) s->clearCharacter();

    for (RosterCard *c : m_rosterCards)
        c->setHighlighted(false);
    m_selectedRoster = -1;
}

void PartySetupScreen::onDeploy()
{
    m_partyManager.clearParty();
    QStringList names;

    for (PartySlot *slot : m_slots) {
        if (!slot->isEmpty() && slot->character()) {
            m_partyManager.addCharacter(slot->character());
            names << QString::fromStdString(slot->character()->getName());
        }
    }

    if (names.isEmpty()) {
        QMessageBox::warning(this, "Deploy",
                             "Add at least one character to your party!");
        return;
    }

    QMessageBox::information(this, "Party Deployed!",
                             "Party: " + names.join(", "));

    emit partyConfirmed(m_partyManager.getParty());
    accept();
}

void PartySetupScreen::updateStatsPanel(const CharDisplayInfo &info)
{
    QPixmap portrait = loadAsset(info.portraitPath);
    if (!portrait.isNull())
        m_statsPortrait->setPixmap(portrait.scaled(100, 100,
                                                   Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation));
    else
        m_statsPortrait->setText("👤");

    m_statsName->setText(info.name);

    if (info.charPtr) {
        m_statsHp ->setText(QString::number(info.charPtr->getMaxHP()));
        m_statsAtk->setText(QString::number(info.charPtr->getAttack()));
        m_statsDef->setText(QString::number(info.charPtr->getDefense()));
        m_statsSpd->setText(QString::number(info.charPtr->getSpeed()));
    } else {
        m_statsHp ->setText("—");
        m_statsAtk->setText("—");
        m_statsDef->setText("—");
        m_statsSpd->setText("—");
    }
}

void PartySetupScreen::refreshSlots()
{
    vector<Character*> party = m_partyManager.getParty();
    for (int i = 0; i < (int)party.size() && i < 3; ++i) {
        for (auto &info : m_charList) {
            if (info.charPtr == party[i]) {
                m_slots[i]->setCharacter(info);
                break;
            }
        }
    }
}

void PartySetupScreen::applyStyle()
{
    setStyleSheet(R"(
        #partyScreen {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #0d0a1a, stop:1 #1a1030);
        }
        #partyCloseBtn {
            background: rgba(255,255,255,0.10);
            border: none; border-radius: 18px;
            color: white; font-size: 16px; font-weight: bold;
        }
        #partyCloseBtn:hover { background: rgba(200,50,50,0.85); }

        #rosterHeader {
            color: #ffd700;
            font-size: 13px;
            font-weight: bold;
            font-family: 'Courier New', monospace;
            letter-spacing: 3px;
            background: transparent;
        }
        #rosterScroll {
            background: rgba(0,0,0,0.25);
            border: 1px solid rgba(255,215,0,0.20);
            border-radius: 10px;
        }
        #slotName {
            color: #ffd700;
            font-size: 13px;
            font-weight: bold;
            font-family: 'Courier New', monospace;
            background: transparent;
        }
        #slotEmpty {
            color: rgba(255,255,255,0.28);
            font-size: 15px;
            background: transparent;
        }
        #statsPanel { background: transparent; }
        #statsPortrait { background: transparent; }
        #statsName {
            color: #ffd700;
            font-size: 16px;
            font-weight: bold;
            font-family: 'Courier New', monospace;
            background: transparent;
        }
        #statKey {
            color: rgba(220,210,180,0.70);
            font-size: 12px;
            font-family: 'Courier New', monospace;
            background: transparent;
        }
        #statVal {
            color: #e2d8c0;
            font-size: 13px;
            font-weight: bold;
            font-family: 'Courier New', monospace;
            background: transparent;
        }
        #rosterName {
            color: rgba(220,210,180,0.80);
            font-size: 9px;
            font-family: 'Courier New', monospace;
            background: transparent;
        }
        QScrollBar:horizontal {
            background: rgba(255,255,255,8);
            height: 5px; border-radius: 2px;
        }
        QScrollBar::handle:horizontal {
            background: rgba(255,215,0,60); border-radius: 2px;
        }
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal { width: 0; }
        QMessageBox { background: #0f172a; }
        QMessageBox QLabel { color: #e2e8f0; }
        QMessageBox QPushButton {
            background: rgba(255,255,255,12);
            border: 1px solid rgba(255,255,255,25);
            border-radius: 6px; color: #e2e8f0;
            padding: 4px 16px;
        }
    )");
}
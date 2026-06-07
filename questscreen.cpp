#include "questscreen.h"
#include "lobbyscreen.h"   // loadAsset()

// Full includes here in the .cpp — NOT in the header, which only forward-declares QLabel.
#include <QLabel>
#include <QFrame>
#include <QDebug>

// ═════════════════════════════════════════════════════════════════════════════
// QuestRow
// ═════════════════════════════════════════════════════════════════════════════

QuestRow::QuestRow(const QuestEntry &entry, QWidget *parent)
    : QWidget(parent)
    , m_entry(entry)
{
    setMinimumHeight(60);

    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setContentsMargins(10, 8, 10, 8);
    hl->setSpacing(12);

    m_badge = new QLabel(this);
    m_badge->setFixedSize(22, 22);
    m_badge->setAlignment(Qt::AlignCenter);

    // Text block
    QVBoxLayout *tv = new QVBoxLayout;
    tv->setSpacing(2);
    m_title  = new QLabel(m_entry.title,  this);
    m_detail = new QLabel(m_entry.detail, this);
    m_detail->setWordWrap(true);
    tv->addWidget(m_title);
    tv->addWidget(m_detail);

    // Reward label
    m_reward = new QLabel(
        QString("🪙 %1   |   %2 XP")
            .arg(m_entry.rewardCoins)
            .arg(m_entry.rewardExp),
        this);
    m_reward->setFixedWidth(130);
    m_reward->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Claim button — only visible when Done
    m_claim = new QPushButton("CLAIM", this);
    m_claim->setFixedSize(68, 28);
    m_claim->setCursor(Qt::PointingHandCursor);
    m_claim->setVisible(false);
    connect(m_claim, &QPushButton::clicked, this, [this](){
        emit claimClicked(m_entry.id);
    });

    hl->addWidget(m_badge);
    hl->addLayout(tv, 1);
    hl->addWidget(m_reward);
    hl->addWidget(m_claim);

    applyStyle();
}

void QuestRow::setStatus(QuestStatus s)
{
    m_entry.status = s;
    applyStyle();
}

void QuestRow::applyStyle()
{
    QString rowBg, rowBorder, badgeCSS, titleColor, detailColor, rewardColor;

    switch (m_entry.status) {
    case QuestStatus::Done:
        rowBg = "rgba(210,245,215,0.85)"; rowBorder = "rgba(60,160,80,0.55)";
        badgeCSS = "background:rgba(60,160,80,0.9);border-radius:11px;color:white;font-weight:bold;font-size:13px;";
        titleColor = "#1f5e2e"; detailColor = "#5a7a60"; rewardColor = "#3a6b45";
        m_badge->setText("✓");
        m_claim->setVisible(true);
        m_claim->setStyleSheet(
            "QPushButton{background:rgba(60,160,80,0.75);border:1px solid rgba(60,160,80,0.9);"
            "border-radius:6px;color:white;font-size:10px;font-weight:bold;letter-spacing:1px;}"
            "QPushButton:hover{background:rgba(60,160,80,0.95);}");
        break;

    case QuestStatus::Active:
        rowBg = "rgba(255,245,210,0.9)"; rowBorder = "rgba(210,150,40,0.6)";
        badgeCSS = "background:rgba(230,170,40,0.9);border-radius:11px;color:white;font-weight:bold;font-size:14px;";
        titleColor = "#5a2d00"; detailColor = "#7a5a30"; rewardColor = "#8a5a10";
        m_badge->setText("•");
        m_claim->setVisible(false);
        break;

    case QuestStatus::Locked:
    default:
        rowBg = "rgba(220,220,220,0.5)"; rowBorder = "rgba(150,150,150,0.45)";
        badgeCSS = "background:rgba(150,150,150,0.5);border-radius:11px;color:#777;font-size:11px;";
        titleColor = "#8a8a8a"; detailColor = "#9a9a9a"; rewardColor = "#9a9a9a";
        m_badge->setText("🔒");
        m_claim->setVisible(false);
        break;
    }

    setStyleSheet(QString("QuestRow{background:%1;border:1px solid %2;border-radius:10px;}").arg(rowBg, rowBorder));
    m_badge ->setStyleSheet(badgeCSS);
    m_title ->setStyleSheet(QString("color:%1;font-size:13px;font-weight:600;background:transparent;").arg(titleColor));
    m_detail->setStyleSheet(QString("color:%1;font-size:11px;background:transparent;").arg(detailColor));
    m_reward->setStyleSheet(QString("color:%1;font-size:11px;background:transparent;").arg(rewardColor));
}

// QuestScreen
QuestScreen::QuestScreen(int playerChapter, QWidget *parent)
    : QDialog(parent)
    , m_playerChapter(playerChapter)
{
    setWindowTitle("QUEST");
    setMinimumSize(480, 560);
    setObjectName("questScreen");
    buildQuests();
    buildUi();
    applyStyle();
}

void QuestScreen::setPlayerChapter(int ch)
{
    m_playerChapter = ch;
    buildQuests();
    refresh();
}

// ── Quest list ────────────────────────────────────────────────────────────────
// Status logic:
//   chapter already finished  → Done
//   currently on this chapter → Active
//   chapter not reached yet   → Locked
void QuestScreen::buildQuests()
{
    auto st = [&](int requiredChapter) -> QuestStatus {
        if (m_playerChapter > requiredChapter) return QuestStatus::Done;
        if (m_playerChapter == requiredChapter) return QuestStatus::Active;
        return QuestStatus::Locked;
    };

    m_quests = {
                // ── Maple Forest  (chapter 1) ─────────────────────────────────────────
                { "mf_q1", "maple_forest",
                 "Finish Story Chapter 01",
                 "Explore and Battle",
                 st(0), 20, 50 },

                { "mf_q2", "maple_forest",
                 "First Journey to begin.",
                 "FParticipate in hunting competition.",
                 st(0), 30, 80 },

                { "mf_q3", "maple_forest",
                 "Finish Story Chapter 02",
                 "Continue your journey to the next region.",
                 st(0), 50, 120 },

                // ── Sun Castle  (chapter 3) ───────────────────────────────────────────
                { "sc_q1", "sun_castle",
                 "Chapter 01",
                 "The head guard waits at the main gate.",
                 st(1), 25, 60 },

                { "sc_q2", "sun_castle",
                 "Find the Sun Relic",
                 "Hidden somewhere.",
                 st(1), 60, 150 },

                { "sc_q3", "sun_castle",
                 "Defeat the Shadow Knight",
                 "Final guardian.",
                 st(1), 100, 300 },

                // ── Dungeon  (chapter 2) ──────────────────────────────────────────────
                { "dg_q1", "dungeon",
                 "Enter the Dungeon",
                 "Discover the mastermind behind the dungeon's competition.",
                 st(2), 50, 120 },

                { "dg_q2", "dungeon",
                 "Reach Level B3",
                 "Navigate three floors of the maze.",
                 st(2), 90, 250 },

                { "dg_q3", "dungeon",
                 "Defeat the Dungeon King",
                 "The final boss. Bring your best party.",
                 st(2), 200, 600 },

                // ── Peak Mountain  (chapter 4) ────────────────────────────────────────
                { "pm_q1", "peak_mountain",
                 "Scale the North Face",
                 "Base camp to the northern ridge.",
                 st(3), 40, 100 },

                { "pm_q2", "peak_mountain",
                 "Find the ??? Met ???",
                 "Continue your journey.",
                 st(3), 80, 200 },

                { "pm_q3", "peak_mountain",
                 "Defeat the Mountain Guardian",
                 "The sky guardian awakens at the summit.",
                 st(3), 150, 400 },
                };
}

// ── UI ────────────────────────────────────────────────────────────────────────
void QuestScreen::buildUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Header: bg_chapter asset as tiled/stretched background, text on top
    QWidget *header = new QWidget(this);
    header->setFixedHeight(72);
    header->setObjectName("questHeader");
    {
        QLabel *bgLbl = new QLabel(header);
            bgLbl->setStyleSheet(
                "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                "stop:0 #1e2d40,stop:1 #0f1923);");
        bgLbl->lower();

        QLabel *titleLbl = new QLabel("📜 QUEST", header);
        titleLbl->setGeometry(20, 0, 300, 72);
        titleLbl->setStyleSheet(
            "color:#e6c77a;"
            "font-size:18px;"
            "font-weight:700;"
            "letter-spacing:3px;"
            );

        m_progressLbl = new QLabel(header);
        m_progressLbl->setGeometry(330, 0, 320, 72);
        m_progressLbl->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_progressLbl->setStyleSheet(
            "color:rgba(255,255,255,0.50);font-size:12px;background:transparent;");
    }
    root->addWidget(header);

    // ── Scrollable quest list
    QWidget *body = new QWidget(this);
    body->setObjectName("questBody");
    QVBoxLayout *bodyL = new QVBoxLayout(body);
    bodyL->setContentsMargins(0, 0, 0, 0);
    bodyL->setSpacing(0);

    m_scroll     = new QScrollArea(body);
    m_listWidget = new QWidget;
    m_listLayout = new QVBoxLayout(m_listWidget);
    m_listLayout->setSpacing(6);
    m_listLayout->setContentsMargins(14, 10, 14, 14);

    m_scroll->setWidget(m_listWidget);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setObjectName("questScroll");
    bodyL->addWidget(m_scroll);
    root->addWidget(body, 1);

    // ── Footer with Close button
    QWidget *footer = new QWidget(this);
    footer->setFixedHeight(52);
    footer->setObjectName("questFooter");
    QHBoxLayout *fl = new QHBoxLayout(footer);
    fl->setContentsMargins(14, 8, 14, 8);
    QPushButton *closeBtn = new QPushButton("Close", footer);
    closeBtn->setObjectName("questCloseBtn");
    closeBtn->setFixedHeight(34);
    connect(closeBtn, &QPushButton::clicked, this, [this](){
        emit questClosed();
        accept();
    });
    fl->addStretch();
    fl->addWidget(closeBtn);
    root->addWidget(footer);

    populateList();
}

void QuestScreen::populateList()
{
    // Remove old rows
    while (QLayoutItem *item = m_listLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    // Count completed for header progress text
    int done = 0;
    for (const auto &q : m_quests)
        if (q.status == QuestStatus::Done) done++;
    if (m_progressLbl)
        m_progressLbl->setText(
            QString("Completed  %1 / %2").arg(done).arg(m_quests.size()));

    // Groups — story order
    struct Group { QString locId; QString header; };
    const QVector<Group> groups = {
                                   { "maple_forest",  "MAPLE FOREST"  },
                                   { "dungeon",    "DUNGEON"    },
                                   { "sun_castle",       "SUN CASTLE"   },
                                   { "peak_mountain", "PEAK MOUNTAIN" },
                                   };

    for (const Group &g : groups) {
        // Location header label
        QLabel *hdr = new QLabel(g.header, m_listWidget);
        hdr->setContentsMargins(0, 10, 0, 4);
        hdr->setStyleSheet(
            "color:#faf1d7;"
            "font-size:12px;"
            "font-weight:700;"
            "letter-spacing:4px;"
            "padding-top:8px;"
            "padding-bottom:4px;"
            );
        m_listLayout->addWidget(hdr);

        for (const auto &quest : m_quests) {
            if (quest.locationId != g.locId) continue;
            QuestRow *row = new QuestRow(quest, m_listWidget);
            connect(row, &QuestRow::claimClicked,
                    this, &QuestScreen::onClaimClicked);
            m_listLayout->addWidget(row);
        }
    }

    m_listLayout->addStretch();
}

void QuestScreen::refresh()
{
    buildQuests();
    populateList();
}

void QuestScreen::onClaimClicked(const QString &id)
{
    // Mark as claimed, give rewards (wire to real save data when ready)
    for (auto &q : m_quests)
        if (q.id == id) { q.status = QuestStatus::Done; break; }
    populateList();
}

void QuestScreen::applyStyle()
{
    setStyleSheet(R"(
        #questScreen,
        #questBody,
        QDialog {
            background:#1b1b1f;
        }
        #questHeader {
            background:#b3567c;
            border-bottom:1px solid #3d4a75;
        }
        #questFooter {
            background:#181818;
            border-top:1px solid #2f2f2f;
        }
        #questScroll { background:transparent; }
        #questScroll QScrollBar:vertical {
            background:rgba(150,110,60,0.12); width:6px; border-radius:3px; }
        #questScroll QScrollBar::handle:vertical {
            background:rgba(120,80,40,0.45); border-radius:3px; min-height:18px; }
        #questScroll QScrollBar::add-line:vertical,
        #questScroll QScrollBar::sub-line:vertical { height:0; }
        #questCloseBtn {
            background:#2c2c2c;
            border:1px solid #444;
            color:#d6d6d6;
            border-radius:6px;
            padding:0 24px;
        }
        #questCloseBtn:hover { background:rgba(180,60,20,0.32); }
        QLabel {
            color:#2a61a8
            font-family:'Trebuchet MS','Segoe UI',sans-serif;   /* <-- THE FONT */
        }
    )");
}
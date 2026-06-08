#include "mainmenuscreen.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFile>
#include <QTime>
#include <QTimer>
#include <QEnterEvent>
#include <QCoreApplication>
#include <QApplication>
#include <QScreen>
#include <QDir>
#include <QDebug>
#include <QResizeEvent>
#include <QShortcut>

// =============================================================================
//  ImageButton
// =============================================================================

ImageButton::ImageButton(const QString &normalImg,
                         const QString &hoverImg,
                         QWidget *parent)
    : QPushButton(parent)
{
    m_normal = QPixmap(normalImg);
    m_hover  = QPixmap(hoverImg);

    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_TranslucentBackground);

    if (!m_normal.isNull()) {
        m_normal = m_normal.scaled(240, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!m_hover.isNull())
            m_hover = m_hover.scaled(240, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setFixedSize(240, 80);
    } else {
        setFixedSize(220, 65);
    }

    setFlat(true);
    setStyleSheet("background: none; border: none;");
}

void ImageButton::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    QPushButton::enterEvent(event);
}

void ImageButton::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    QPushButton::leaveEvent(event);
}

void ImageButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!m_normal.isNull()) {
        const QPixmap &px = (m_hovered && !m_hover.isNull()) ? m_hover : m_normal;
        p.drawPixmap(rect(), px);
    } else {
        QColor bg     = m_hovered ? QColor(60, 40, 0, 230)   : QColor(10, 10, 30, 220);
        QColor border = m_hovered ? QColor(230, 126, 0)       : QColor(240, 192, 32);
        QColor tc     = m_hovered ? QColor(230, 126, 0)       : QColor(240, 192, 32);
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawRoundedRect(rect(), 8, 8);
        p.setPen(QPen(border, 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect().adjusted(1,1,-1,-1), 8, 8);
        p.setPen(tc);
        p.setFont(QFont("Courier New", 16, QFont::Bold));
        p.drawText(rect(), Qt::AlignCenter, this->text());
    }

    if (!isEnabled())
        p.fillRect(rect(), QColor(0, 0, 0, 150));
}

// =============================================================================
//  LoadSlotPopup
// =============================================================================

LoadSlotPopup::LoadSlotPopup(SaveManager  &saveManager,
                             Inventory    &inventory,
                             Shop         &shop,
                             PartyManager &partyManager,
                             QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
    , m_saveManager(saveManager)
    , m_inventory(inventory)
    , m_shop(shop)
    , m_partyManager(partyManager)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setFixedSize(480, 340);

    setStyleSheet(
        "QDialog {"
        "background:#eb94a7;"
        "border:6px solid #fff29e;"
        "border-radius:16px;"
        "}"
        "QLabel { color:white; background:transparent; }"
        "QPushButton {"
        "background:#6ba0db; color:white;"
        "font-size:15px; font-weight:bold;"
        "border:2px solid #6ea8ff; border-radius:12px; padding:10px;"
        "}"
        "QPushButton:hover { background:#3d78d8; }"
        "QPushButton:disabled { background:#444; color:#888; border-color:#666; }"
        );

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(25, 25, 25, 25);
    layout->setSpacing(12);

    QLabel *title = new QLabel("📂 Load Game");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:22px; font-weight:bold; color:#dbe9ff;");
    layout->addWidget(title);

    for (int i = 0; i < 3; i++) {
        QString text;
        bool isEmpty = m_saveManager.isSlotEmpty(i);

        if (isEmpty) {
            text = QString("SLOT %1   •   EMPTY").arg(i + 1);
        } else {
            // Preview save info
            Inventory tempInv;
            Shop tempShop;
            PartyManager tempPM;
            PlayerRecord preview = m_saveManager.loadGame(i, tempInv, tempShop, tempPM);
            text = QString("SLOT %1   •   %2   |   Ch.%3   |   🪙%4")
                       .arg(i + 1)
                       .arg(QString(preview.playerUsername).isEmpty()
                                ? "Unknown" : QString(preview.playerUsername))
                       .arg(preview.currentChapter)
                       .arg(preview.coins);
        }

        QPushButton *btn = new QPushButton(text);
        btn->setEnabled(!isEmpty);

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            PlayerRecord record = m_saveManager.loadGame(
                i, m_inventory, m_shop, m_partyManager);
            if (record.occupied) {
                emit slotChosen(i, record);
                accept();
            }
        });

        layout->addWidget(btn);
    }

    // Cancel
    QPushButton *btnCancel = new QPushButton("CANCEL");
    btnCancel->setStyleSheet(
        "QPushButton { background:#a04060; border-color:#ff8888; }"
        "QPushButton:hover { background:#cc4444; }");
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    layout->addWidget(btnCancel);
}

// =============================================================================
//  MainMenuScreen
// =============================================================================

MainMenuScreen::MainMenuScreen(SaveManager  &saveManager,
                               Inventory    &inventory,
                               Shop         &shop,
                               PartyManager &partyManager,
                               QWidget *parent)
    : QWidget(parent)
    , m_saveManager(saveManager)
    , m_inventory(inventory)
    , m_shop(shop)
    , m_partyManager(partyManager)
{
    setStyleSheet("background-color: black;");

    const QString exeDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        exeDir + "/assets",
        exeDir + "/../assets",
        exeDir + "/../../assets",
        exeDir + "/../../../assets"
    };
    for (const QString &c : candidates) {
        if (QDir(c + "/mainmenu").exists()) {
            m_assetRoot = QDir::cleanPath(c);
            break;
        }
    }

    m_dayPixmap   = QPixmap(m_assetRoot + "/mainmenu/Day.PNG");
    m_nightPixmap = QPixmap(m_assetRoot + "/mainmenu/Night.PNG");

    bgLabel = new QLabel(this);
    bgLabel->setGeometry(0, 0, 800, 600);

    m_bgTimer = new QTimer(this);
    m_bgTimer->setInterval(60 * 1000);
    connect(m_bgTimer, &QTimer::timeout, this, &MainMenuScreen::updateBackground);
    m_bgTimer->start();

    auto p = [&](const QString &f) {
        return m_assetRoot + "/mainmenu/" + f;
    };

    btnStart = new ImageButton(p("Start_Y.png"), p("Start_O.png"), this);
    btnStart->setText("START");

    btnExit  = new ImageButton(p("Exit_Y.png"),  p("Exit_O.png"),  this);
    btnExit->setText("EXIT");

    btnNew   = new ImageButton(p("New_Y.png"),   p("New_O.png"),   this);
    btnNew->setText("NEW GAME");

    btnLoad  = new ImageButton(p("Load_Y.png"),  p("Load_O.png"),  this);
    btnLoad->setText("LOAD GAME");

    bgBtn1 = new QLabel(this);
    bgBtn2 = new QLabel(this);
    bgBtn1->setAttribute(Qt::WA_TranslucentBackground);
    bgBtn2->setAttribute(Qt::WA_TranslucentBackground);
    bgBtn1->lower();
    bgBtn2->lower();

    setupConnections();
    showPhase1();

    QShortcut *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(esc, &QShortcut::activated, [=]() {
        if (window()->isFullScreen())
            window()->showNormal();
        else
            window()->showFullScreen();
    });
}

void MainMenuScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    m_W = width();
    m_H = height();

    bgLabel->setGeometry(0, 0, m_W, m_H);
    updateBackground();

    int btnW = (btnStart->width()  > 0) ? btnStart->width()  : 240;
    int btnH = (btnStart->height() > 0) ? btnStart->height() : 80;

    int btnX = m_W - btnW - 140;
    int btnY = (m_H / 2) - btnH + 50;
    int gap  = btnH + 95;

    btnStart->move(btnX, btnY);
    btnExit->move (btnX, btnY + gap);
    btnNew->move  (btnX, btnY);
    btnLoad->move (btnX, btnY + gap);

    int platePadX = 70;
    int platePadY = 25;

    bgBtn1->setGeometry(btnX - platePadX, btnY - platePadY,
                        btnW + platePadX * 2, btnH + platePadY * 2);
    bgBtn2->setGeometry(btnX - platePadX, btnY + gap - platePadY,
                        btnW + platePadX * 2, btnH + platePadY * 2);

    updateBackground();

    bgLabel->lower();
    bgBtn1->raise(); bgBtn2->raise();
    btnStart->raise(); btnExit->raise();
    btnNew->raise(); btnLoad->raise();
}

void MainMenuScreen::setupConnections()
{
    connect(btnStart, &QPushButton::clicked, this, [this]() { showPhase2(); });
    connect(btnExit,  &QPushButton::clicked, this, [this]() { emit exitClicked(); });
    connect(btnNew,   &QPushButton::clicked, this, [this]() { emit newGameClicked(); });
    connect(btnLoad,  &QPushButton::clicked, this, &MainMenuScreen::onLoadClicked);
}

void MainMenuScreen::onLoadClicked()
{
    LoadSlotPopup *popup = new LoadSlotPopup(
        m_saveManager, m_inventory, m_shop, m_partyManager, this);

    connect(popup, &LoadSlotPopup::slotChosen,
            this,  &MainMenuScreen::loadSlotChosen);

    QPoint c = mapToGlobal(rect().center());
    popup->move(c.x() - popup->width()/2, c.y() - popup->height()/2);
    popup->exec();
    popup->deleteLater();
}

void MainMenuScreen::showPhase1()
{
    btnStart->show(); btnExit->show();
    btnNew->hide();   btnLoad->hide();
    bgBtn1->show();   bgBtn2->show();
    bgLabel->lower();
    bgBtn1->raise();  bgBtn2->raise();
    btnStart->raise(); btnExit->raise();
}

void MainMenuScreen::showPhase2()
{
    btnStart->hide(); btnExit->hide();
    btnNew->show();   btnLoad->show();
    bgBtn1->show();   bgBtn2->show();
    bgLabel->lower();
    bgBtn1->raise();  bgBtn2->raise();
    btnNew->raise();  btnLoad->raise();
}

void MainMenuScreen::updateBackground()
{
    if (m_W == 0 || m_H == 0) return;

    const QPixmap &src = isNightTime() ? m_nightPixmap : m_dayPixmap;

    if (src.isNull()) {
        bgLabel->setStyleSheet(isNightTime()
                               ? "background-color: #0d0d1a;"
                               : "background-color: #87CEEB;");
    } else {
        QPixmap scaled = src.scaled(m_W, m_H,
                                    Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        int cx = (scaled.width()  - m_W) / 2;
        int cy = (scaled.height() - m_H) / 2;
        bgLabel->setPixmap(scaled.copy(cx, cy, m_W, m_H));
    }

    QString btnBgPath = m_assetRoot + "/mainmenu/" +
                        (isNightTime() ? "darkButton.png" : "lightButton.png");
    QPixmap btnBg(btnBgPath);

    if (!btnBg.isNull() && bgBtn1 && bgBtn2) {
        QSize plateSize(bgBtn1->width(), bgBtn1->height());
        if (plateSize.width() <= 0 || plateSize.height() <= 0) return;

        QPixmap scaledPlate = btnBg.scaled(plateSize,
                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        bgBtn1->setPixmap(scaledPlate);
        bgBtn2->setPixmap(scaledPlate);
        bgBtn1->setScaledContents(true);
        bgBtn2->setScaledContents(true);
    }
}

bool MainMenuScreen::isNightTime() const
{
    const int hour = QTime::currentTime().hour();
    return (hour >= 18 || hour < 6);
}

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

    qDebug() << "[ImageButton] Normal:" << normalImg << "loaded:" << !m_normal.isNull();

    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);

    if (!m_normal.isNull()) {
        setFixedSize(m_normal.size());
    } else {
        setFixedSize(220, 65);
    }

    setFlat(true);
    setStyleSheet("background: transparent; border: none;");
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
        // Fallback — always visible
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
//  MainMenuScreen
// =============================================================================

MainMenuScreen::MainMenuScreen(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: black;");

    // Resolve asset root
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
    qDebug() << "[MainMenu] Asset root:" << m_assetRoot;

    // NOTE: Day.PNG and Night.PNG use uppercase PNG (your files)
    //       Start/Exit/New/Load use lowercase png (your files)
    m_dayPixmap   = QPixmap(m_assetRoot + "/mainmenu/Day.PNG");
    m_nightPixmap = QPixmap(m_assetRoot + "/mainmenu/Night.PNG");
    qDebug() << "[MainMenu] Day:"   << !m_dayPixmap.isNull() << m_dayPixmap.size();
    qDebug() << "[MainMenu] Night:" << !m_nightPixmap.isNull() << m_nightPixmap.size();

    // Background — BELOW everything
    bgLabel = new QLabel(this);
    bgLabel->setGeometry(0, 0, 800, 600); // resized in resizeEvent

    // Timer for day/night switching
    m_bgTimer = new QTimer(this);
    m_bgTimer->setInterval(60 * 1000);
    connect(m_bgTimer, &QTimer::timeout, this, &MainMenuScreen::updateBackground);
    m_bgTimer->start();

    // Buttons — lowercase .png
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

    setupConnections();
    showPhase1();
}

void MainMenuScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    m_W = width();
    m_H = height();
    qDebug() << "[MainMenu] resizeEvent:" << m_W << "x" << m_H;

    // Fill background
    bgLabel->setGeometry(0, 0, m_W, m_H);
    updateBackground();

    // Button size (use actual or fallback 220x65)
    int btnW = (btnStart->width()  > 0) ? btnStart->width()  : 220;
    int btnH = (btnStart->height() > 0) ? btnStart->height() : 65;
    int btnX = m_W - btnW - 80;
    int btnY = (m_H / 2) - btnH;
    int gap  = btnH + 20;

    btnStart->move(btnX, btnY);
    btnExit->move (btnX, btnY + gap);
    btnNew->move  (btnX, btnY);
    btnLoad->move (btnX, btnY + gap);

    // CRITICAL: raise buttons ABOVE bgLabel
    bgLabel->lower();   // push bg to very bottom
    btnStart->raise();
    btnExit->raise();
    btnNew->raise();
    btnLoad->raise();

    qDebug() << "[MainMenu] btn pos:" << btnX << btnY
             << "size:" << btnW << "x" << btnH;
}

void MainMenuScreen::setupConnections()
{
    connect(btnStart, &QPushButton::clicked, this, &MainMenuScreen::showPhase2);
    connect(btnExit,  &QPushButton::clicked, this, &MainMenuScreen::exitClicked);
    connect(btnNew,   &QPushButton::clicked, this, &MainMenuScreen::newGameClicked);
    connect(btnLoad,  &QPushButton::clicked, this, &MainMenuScreen::loadGameClicked);
}

void MainMenuScreen::showPhase1()
{
    btnStart->show();
    btnExit->show();
    btnNew->hide();
    btnLoad->hide();
}

void MainMenuScreen::showPhase2()
{
    btnStart->hide();
    btnExit->hide();
    btnNew->show();
    btnLoad->show();

    const bool saveExists = hasSaveFile();
    btnLoad->setEnabled(saveExists);
    btnLoad->setToolTip(saveExists ? "" : "No save file found. Start a new game first.");
}

bool MainMenuScreen::isNightTime() const
{
    const int hour = QTime::currentTime().hour();
    return (hour >= 18 || hour < 6);
}

void MainMenuScreen::updateBackground()
{
    if (m_W == 0 || m_H == 0) return;

    const QPixmap &src = isNightTime() ? m_nightPixmap : m_dayPixmap;
    if (src.isNull()) {
        bgLabel->setStyleSheet(isNightTime()
                               ? "background-color: #0d0d1a;"
                               : "background-color: #87CEEB;");
        return;
    }

    // Scale to fill full screen, crop centre — no black bars, not stretched
    QPixmap scaled = src.scaled(m_W, m_H,
                                Qt::KeepAspectRatioByExpanding,
                                Qt::SmoothTransformation);
    int cx = (scaled.width()  - m_W) / 2;
    int cy = (scaled.height() - m_H) / 2;
    bgLabel->setPixmap(scaled.copy(cx, cy, m_W, m_H));
}

bool MainMenuScreen::hasSaveFile() const
{
    return QFile::exists("players.dat");
}
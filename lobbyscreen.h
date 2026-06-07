#ifndef LOBBYSCREEN_H
#define LOBBYSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QVector>
#include <QPixmap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QScrollArea>
#include <QScreen>
#include <QApplication>

#include <vector>
using namespace std;

#include "character.h"
#include "inventory.h"
#include "partymanager.h"
#include "savemanager.h"
#include "shop.h"
#include "shopscreen.h"
#include "avatarpicker.h"
#include "questscreen.h"

// ─────────────────────────────────────────────────────────────────────────────
// loadAsset – case-insensitive asset loader
// Tries path as-is, then appends .PNG/.png/.JPEG/.jpeg etc.
// Also checks Qt resource prefix :/
// ─────────────────────────────────────────────────────────────────────────────
inline QPixmap loadAsset(const QString &basePath)
{
    // Try exact path first (already has extension)
    { QPixmap p(basePath);                          if (!p.isNull()) return p; }
    { QPixmap p(QStringLiteral(":/") + basePath);  if (!p.isNull()) return p; }

    const QStringList exts = {
        ".PNG",".png",".JPEG",".jpeg",".JPG",".jpg",".BMP",".bmp",".WEBP",".webp"
    };
    for (const QString &e : exts) {
        { QPixmap p(basePath + e);                         if (!p.isNull()) return p; }
        { QPixmap p(QStringLiteral(":/") + basePath + e); if (!p.isNull()) return p; }
    }
    QPixmap fb(1, 1); fb.fill(Qt::transparent); return fb;
}

// ─────────────────────────────────────────────────────────────────────────────
// LocationInfo
// ─────────────────────────────────────────────────────────────────────────────
struct LocationInfo {
    QString          id;
    QString          name;
    QString          description;
    int              requiredChapter;   // player.chapter must be >= this to unlock
    QVector<QString> chapters;          // one entry per popup page
    QString          assetPath;         // base path, no extension
    float            relX, relY;        // 0-1 fraction of mapArea size
};

// ─────────────────────────────────────────────────────────────────────────────
// PixmapButton
// ─────────────────────────────────────────────────────────────────────────────
class PixmapButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PixmapButton(const QString &assetPath, QWidget *parent = nullptr);
    void setAsset(const QString &assetPath);
    void setHoverOverlay(bool e) { m_hoverOverlay = e; }

protected:
    void paintEvent (QPaintEvent *) override;
    void enterEvent (QEnterEvent *) override;
    void leaveEvent (QEvent *)      override;

private:
    QPixmap m_pixmap;
    bool    m_hovered      = false;
    bool    m_hoverOverlay = true;
};

// ─────────────────────────────────────────────────────────────────────────────
// LocationButton
// ─────────────────────────────────────────────────────────────────────────────
class LocationButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal hoverAlpha READ hoverAlpha WRITE setHoverAlpha)
public:
    explicit LocationButton(const LocationInfo &info, QWidget *parent = nullptr);
    void setLocked(bool locked);
    void setIsCurrent(bool current);
    bool               isLocked() const { return m_locked; }
    const LocationInfo &info()    const { return m_info;   }

signals:
    void clicked(const LocationInfo &info);

protected:
    void paintEvent     (QPaintEvent  *) override;
    void enterEvent     (QEnterEvent  *) override;
    void leaveEvent     (QEvent       *) override;
    void mousePressEvent(QMouseEvent  *) override;

private:
    qreal hoverAlpha()       const { return m_hoverAlpha; }
    void  setHoverAlpha(qreal v)   { m_hoverAlpha = v; update(); }

    LocationInfo        m_info;
    QPixmap             m_locationPx, m_lockPx, m_arrowPx;
    bool                m_locked = false, m_isCurrent = false, m_hovered = false;
    QPropertyAnimation *m_hoverAnim  = nullptr;
    qreal               m_hoverAlpha = 0.0;
};

// ─────────────────────────────────────────────────────────────────────────────
// QuestPopup
// ─────────────────────────────────────────────────────────────────────────────
class QuestPopup : public QDialog
{
    Q_OBJECT
public:
    explicit QuestPopup(const LocationInfo &info, QWidget *parent = nullptr);
signals:
    void playRequested(const QString &locationId);
private slots:
    void prevPage();
    void nextPage();
private:
    void buildUi(const LocationInfo &info);
    void updatePage();

    LocationInfo  m_info;
    int           m_page = 0;
    QLabel       *m_bgLabel    = nullptr;
    QLabel       *m_chapterBg  = nullptr;   // bg_chapter background
    QLabel       *m_chapterLbl = nullptr;   // "Chapter 1" text on top
    QLabel       *m_questText  = nullptr;
    QLabel       *m_pageLbl    = nullptr;
    PixmapButton *m_leftBtn    = nullptr;
    PixmapButton *m_rightBtn   = nullptr;
    PixmapButton *m_playBtn    = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────
// LobbyScreen
// ─────────────────────────────────────────────────────────────────────────────
class LobbyScreen : public QWidget
{
    Q_OBJECT
public:
    explicit LobbyScreen(PlayerRecord   &playerRecord,
                         vector<Character*> allCharacters,
                         Shop           &shop,
                         Inventory      &inventory,
                         int            &coins,
                         SaveManager    &saveManager,
                         PartyManager   &partyManager,
                         int             saveSlot = 0,
                         QWidget        *parent   = nullptr);

    void refreshStats();

signals:
    void battleRequested(const QString &locationId);

protected:
    void paintEvent (QPaintEvent  *) override;
    void resizeEvent(QResizeEvent *) override;

private slots:
    void onLocationClicked(const LocationInfo &info);
    void onPlayFromPopup  (const QString &locationId);
    void onShopClicked();
    void onFarmClicked();
    void onQuestClicked();       //opens QuestScreen
    void onInventoryClicked();
    void onPartyClicked();
    void onSaveClicked();
    void onAvatarClicked();      // opens AvatarPicker
    void onAvatarSelected(int index, const QString &assetPath, const QString &name);
    void onEditNameClicked();    // pencil button → rename player

private:
    void buildLocations();
    void buildUi();
    void buildTopBar();
    void buildMapArea();
    void buildSidePanel();
    void buildBottomBar();
    void positionLocations();
    void updateLockStates();
    void animateEntrance();

    // ── State
    PlayerRecord       &m_record;
    vector<Character*>  m_allCharacters;
    Shop               &m_shop;
    Inventory          &m_inventory;
    int                &m_coins;
    SaveManager        &m_saveManager;
    PartyManager       &m_partyManager;
    int                 m_saveSlot;

    QVector<LocationInfo>    m_locations;
    QVector<LocationButton*> m_locBtns;

    // ── Containers
    QWidget *m_topBar    = nullptr;
    QWidget *m_mapArea   = nullptr;
    QWidget *m_bottomBar = nullptr;

    // ── Top bar
    PixmapButton *m_playerIdBtn = nullptr;
    QLabel       *m_playerUsername = nullptr;
    // Energy badge
    QLabel       *m_energyBg    = nullptr;
    QLabel       *m_energyIcon  = nullptr;
    QLabel       *m_energyVal   = nullptr;
    // Coins badge
    QLabel       *m_coinsBg     = nullptr;
    QLabel       *m_coinsIcon   = nullptr;
    QLabel       *m_coinsVal    = nullptr;

    // ── Side panel
    PixmapButton *m_shopBtn  = nullptr;
    PixmapButton *m_farmBtn  = nullptr;
    PixmapButton *m_questBtn = nullptr;
    QLabel *m_shopLbl  = nullptr;
    QLabel *m_farmLbl  = nullptr;
    QLabel *m_questLbl = nullptr;

    // ── Bottom bar
    PixmapButton *m_saveBtn      = nullptr;
    PixmapButton *m_inventoryBtn = nullptr;
    PixmapButton *m_partyBtn     = nullptr;
    PixmapButton *m_playBtn      = nullptr;

    // ── Background
    QLabel *m_bgLabel = nullptr;

    // ── Owned sub-dialogs (created once, reused)
    AvatarPicker *m_avatarPicker = nullptr;
    QuestScreen  *m_questScreen  = nullptr;
};

#endif // LOBBYSCREEN_H
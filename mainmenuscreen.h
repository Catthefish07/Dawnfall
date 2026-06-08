#ifndef MAINMENUSCREEN_H
#define MAINMENUSCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QDialog>
#include <QVBoxLayout>
#include "savemanager.h"
#include "inventory.h"
#include "shop.h"
#include "partymanager.h"

// ── Custom image-based button ─────────────────────────────────────────────────
class ImageButton : public QPushButton {
    Q_OBJECT
public:
    explicit ImageButton(const QString &normalImg,
                         const QString &hoverImg,
                         QWidget *parent = nullptr);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent      *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_normal;
    QPixmap m_hover;
    bool    m_hovered = false;
};

// ── Load Slot Popup ───────────────────────────────────────────────────────────
class LoadSlotPopup : public QDialog {
    Q_OBJECT
public:
    explicit LoadSlotPopup(SaveManager &saveManager,
                           Inventory   &inventory,
                           Shop        &shop,
                           PartyManager &partyManager,
                           QWidget *parent = nullptr);
signals:
    void slotChosen(int slot, PlayerRecord record);

private:
    SaveManager  &m_saveManager;
    Inventory    &m_inventory;
    Shop         &m_shop;
    PartyManager &m_partyManager;
};

// ── Main menu screen ──────────────────────────────────────────────────────────
class MainMenuScreen : public QWidget {
    Q_OBJECT
public:
    explicit MainMenuScreen(SaveManager  &saveManager,
                            Inventory    &inventory,
                            Shop         &shop,
                            PartyManager &partyManager,
                            QWidget *parent = nullptr);

signals:
    void newGameClicked();
    void exitClicked();
    void loadSlotChosen(int slot, PlayerRecord record);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateBackground();
    void onLoadClicked();

private:
    SaveManager  &m_saveManager;
    Inventory    &m_inventory;
    Shop         &m_shop;
    PartyManager &m_partyManager;

    QLabel      *bgLabel     = nullptr;
    QPixmap      m_dayPixmap;
    QPixmap      m_nightPixmap;
    QTimer      *m_bgTimer   = nullptr;
    int          m_W         = 0;
    int          m_H         = 0;
    QString      m_assetRoot;
    QLabel      *bgBtn1      = nullptr;
    QLabel      *bgBtn2      = nullptr;

    ImageButton *btnStart    = nullptr;
    ImageButton *btnExit     = nullptr;
    ImageButton *btnNew      = nullptr;
    ImageButton *btnLoad     = nullptr;

    void setupConnections();
    void showPhase1();
    void showPhase2();
    bool isNightTime() const;
};

#endif

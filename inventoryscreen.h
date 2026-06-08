#ifndef INVENTORYSCREEN_H
#define INVENTORYSCREEN_H

// ─────────────────────────────────────────────────────────────────────────────
// InventoryScreen – shows the player's current item quantities.
// Player can sell items back for 50% of buy price.
//
// Connected from LobbyScreen::onInventoryClicked():
//   auto *screen = new InventoryScreen(m_inventory, this);
//   screen->setWindowFlags(Qt::Dialog);
//   screen->setAttribute(Qt::WA_DeleteOnClose);
//   screen->show();
//
// If you want the coin count to update in lobby after selling,
// connect the signal:
//   connect(screen, &InventoryScreen::inventoryClosed,
//           this, &LobbyScreen::refreshStats);
// ─────────────────────────────────────────────────────────────────────────────

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QVector>
#include <QString>
#include <QSpinBox>

#include "inventory.h"
#include "lobbyscreen.h"   // for loadAsset() and PixmapButton

// ── One item slot in the inventory grid ──────────────────────────────────────
class InventorySlot : public QWidget
{
    Q_OBJECT
public:
    explicit InventorySlot(const QString &itemName,
                           int            quantity,
                           int            sellPrice,
                           QWidget       *parent = nullptr);

    void setQuantity(int qty);
    QString itemName() const { return m_itemName; }

signals:
    void sellRequested(const QString &itemName, int qty);

private:
    void buildUi();

    QString     m_itemName;
    int         m_quantity;
    int         m_sellPrice;   // 50% of buy price

    QLabel      *m_iconLbl   = nullptr;
    QLabel      *m_nameLbl   = nullptr;
    QLabel      *m_qtyLbl    = nullptr;
    QLabel      *m_priceLbl  = nullptr;
    QSpinBox    *m_sellSpin  = nullptr;
    QPushButton *m_sellBtn   = nullptr;
};

// ── Main inventory dialog ─────────────────────────────────────────────────────
class InventoryScreen : public QDialog
{
    Q_OBJECT
public:
    // Matches the call in lobbyscreen.cpp:
    //   new InventoryScreen(m_inventory, this)
    explicit InventoryScreen(Inventory &inventory, QWidget *parent = nullptr);

    void refresh();

signals:
    void inventoryClosed();

private slots:
    void onSellItem(const QString &itemName, int qty);

private:
    void buildUi();
    void populateGrid();
    void applyStyle();

    Inventory              &m_inventory;
    QWidget                *m_gridWidget  = nullptr;
    QGridLayout            *m_grid        = nullptr;
    QLabel                 *m_coinsLbl    = nullptr;
    QVector<InventorySlot*> m_slots;

    // Sell prices = 50% of buy price
    struct ItemInfo {
        QString name;
        QString iconAsset;
        int     buyPrice;
    };
    const QVector<ItemInfo> m_itemDefs = {
                                          { "Health Potion", "assets/shopscreen/healthpotion", 1500 },
                                          { "Mega Potion",   "assets/shopscreen/megapotion",   3000 },
                                          { "Revive Stone",  "assets/shopscreen/revivestone",  5000 },
                                          };
};

#endif // INVENTORYSCREEN_H
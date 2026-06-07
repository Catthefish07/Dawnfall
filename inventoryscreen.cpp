#include "inventoryscreen.h"
#include <QMessageBox>
#include <QFrame>
#include <QScrollArea>
#include <QDebug>

// ═════════════════════════════════════════════════════════════════════════════
// InventorySlot
// ═════════════════════════════════════════════════════════════════════════════

InventorySlot::InventorySlot(const QString &itemName,
                             int            quantity,
                             int            sellPrice,
                             QWidget       *parent)
    : QWidget(parent)
    , m_itemName(itemName)
    , m_quantity(quantity)
    , m_sellPrice(sellPrice)
{
    buildUi();
}

void InventorySlot::buildUi()
{
    setFixedSize(240, 220);
    setObjectName("inventorySlot");

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(12, 12, 12, 12);
    vl->setSpacing(6);
    vl->setAlignment(Qt::AlignHCenter);

    // ── Item icon
    m_iconLbl = new QLabel(this);
    m_iconLbl->setFixedSize(90, 90);
    m_iconLbl->setAlignment(Qt::AlignCenter);
    m_iconLbl->setScaledContents(true);

    QString iconPath =
        m_itemName == "Health Potion" ? "assets/shopscreen/healthpotion" :
            m_itemName == "Mega Potion"   ? "assets/shopscreen/megapotion"   :
            "assets/shopscreen/revivestone";
    QPixmap px = loadAsset(iconPath);
    if (!px.isNull())
        m_iconLbl->setPixmap(px);
    else
        m_iconLbl->setText(m_itemName == "Health Potion" ? "🧪" :
                               m_itemName == "Mega Potion"   ? "💊" : "💎");

    // ── Name
    m_nameLbl = new QLabel(m_itemName, this);
    m_nameLbl->setAlignment(Qt::AlignCenter);
    m_nameLbl->setObjectName("slotName");

    // ── Quantity badge
    m_qtyLbl = new QLabel(QString("x%1").arg(m_quantity), this);
    m_qtyLbl->setAlignment(Qt::AlignCenter);
    m_qtyLbl->setObjectName(m_quantity > 0 ? "slotQty" : "slotQtyEmpty");

    // ── Sell price label
    m_priceLbl = new QLabel(
        QString("🪙 %1 / item").arg(m_sellPrice / 2), this);
    m_priceLbl->setAlignment(Qt::AlignCenter);
    m_priceLbl->setObjectName("slotSellPrice");

    // ── Sell spinner + button (only if has items)
    QHBoxLayout *sellRow = new QHBoxLayout;
    sellRow->setSpacing(6);

    m_sellSpin = new QSpinBox(this);
    m_sellSpin->setRange(1, qMax(1, m_quantity));
    m_sellSpin->setValue(1);
    m_sellSpin->setFixedWidth(65);
    m_sellSpin->setObjectName("sellSpinner");
    m_sellSpin->setEnabled(m_quantity > 0);

    m_sellBtn = new QPushButton("SELL", this);
    m_sellBtn->setObjectName("sellBtn");
    m_sellBtn->setFixedHeight(28);
    m_sellBtn->setCursor(Qt::PointingHandCursor);
    m_sellBtn->setEnabled(m_quantity > 0);

    connect(m_sellBtn, &QPushButton::clicked, this, [this](){
        emit sellRequested(m_itemName, m_sellSpin->value());
    });

    sellRow->addWidget(m_sellSpin);
    sellRow->addWidget(m_sellBtn, 1);

    vl->addWidget(m_iconLbl,  0, Qt::AlignHCenter);
    vl->addWidget(m_nameLbl);
    vl->addWidget(m_qtyLbl);
    vl->addWidget(m_priceLbl);
    vl->addLayout(sellRow);
}

void InventorySlot::setQuantity(int qty)
{
    m_quantity = qty;
    m_qtyLbl->setText(QString("x%1").arg(qty));
    m_qtyLbl->setObjectName(qty > 0 ? "slotQty" : "slotQtyEmpty");

    m_sellSpin->setRange(1, qMax(1, qty));
    m_sellSpin->setValue(1);
    m_sellSpin->setEnabled(qty > 0);
    m_sellBtn->setEnabled(qty > 0);
}

// InventoryScreen

InventoryScreen::InventoryScreen(Inventory &inventory, QWidget *parent)
    : QDialog(parent)
    , m_inventory(inventory)
{
    setWindowTitle("Inventory");
    setMinimumSize(860, 520);
    setObjectName("inventoryScreen");
    buildUi();
    applyStyle();
}

void InventoryScreen::buildUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Top bar
    QWidget *topBar = new QWidget(this);
    topBar->setObjectName("invTopBar");
    topBar->setFixedHeight(64);

    QHBoxLayout *th = new QHBoxLayout(topBar);
    th->setContentsMargins(20, 10, 20, 10);
    th->setSpacing(12);

    QLabel *titleLbl = new QLabel("🎒  Inventory", topBar);
    titleLbl->setObjectName("invTitle");
    th->addWidget(titleLbl);
    th->addStretch(1);

    m_coinsLbl = new QLabel(
        QString("🪙  %1").arg(m_inventory.getCoins()), topBar);
    m_coinsLbl->setObjectName("invCoins");
    th->addWidget(m_coinsLbl);

    th->addSpacing(16);

    QPushButton *closeBtn = new QPushButton("✕", topBar);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setObjectName("invCloseBtn");
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, [this](){
        emit inventoryClosed();
        accept();
    });
    th->addWidget(closeBtn);

    root->addWidget(topBar);

    // ── Hint text
    QLabel *hintLbl = new QLabel(
        "Sell items for 50% of their purchase price.", this);
    hintLbl->setObjectName("invHint");
    hintLbl->setAlignment(Qt::AlignCenter);
    root->addWidget(hintLbl);

    // ── Item grid (scrollable)
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setObjectName("invScroll");
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);

    m_gridWidget = new QWidget;
    m_grid       = new QGridLayout(m_gridWidget);
    m_grid->setSpacing(20);
    m_grid->setContentsMargins(30, 20, 30, 20);
    m_grid->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    populateGrid();

    scroll->setWidget(m_gridWidget);
    root->addWidget(scroll, 1);
}

void InventoryScreen::populateGrid()
{
    // Clear existing slots
    while (QLayoutItem *item = m_grid->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    m_slots.clear();

    // 3 items displayed in a row
    for (int i = 0; i < m_itemDefs.size(); ++i) {
        const ItemInfo &def = m_itemDefs[i];
        int qty = m_inventory.getQuantity(def.name.toStdString());

        InventorySlot *slot = new InventorySlot(
            def.name, qty, def.buyPrice, m_gridWidget);

        connect(slot, &InventorySlot::sellRequested,
                this, &InventoryScreen::onSellItem);

        m_grid->addWidget(slot, 0, i);
        m_slots.append(slot);
    }
}

void InventoryScreen::refresh()
{
    // Update quantities and coins display
    m_coinsLbl->setText(QString("🪙  %1").arg(m_inventory.getCoins()));
    for (InventorySlot *slot : m_slots) {
        int qty = m_inventory.getQuantity(slot->itemName().toStdString());
        slot->setQuantity(qty);
    }
}

void InventoryScreen::onSellItem(const QString &itemName, int qty)
{
    // Find sell price (50% of buy)
    int sellPrice = 0;
    for (const ItemInfo &def : m_itemDefs) {
        if (def.name == itemName) {
            sellPrice = (def.buyPrice / 2) * qty;
            break;
        }
    }

    // Check if player has enough
    int owned = m_inventory.getQuantity(itemName.toStdString());
    if (qty > owned) {
        QMessageBox::warning(this, "Cannot Sell",
                             QString("You only have %1x %2!").arg(owned).arg(itemName));
        return;
    }

    // Confirm
    auto reply = QMessageBox::question(this, "Sell Items",
                                       QString("Sell %1x %2 for 🪙 %3?")
                                           .arg(qty).arg(itemName).arg(sellPrice),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    // Remove items and give coins
    m_inventory.removeItems(itemName.toStdString(), qty);
    m_inventory.addCoins(sellPrice);

    QMessageBox::information(this, "Sold!",
                             QString("Sold %1x %2 for 🪙 %3!")
                                 .arg(qty).arg(itemName).arg(sellPrice));

    refresh();
}

void InventoryScreen::applyStyle()
{
    setStyleSheet(R"(
        #inventoryScreen {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #dff4ff, stop:1 #bfe8ff);
            border: 2px solid rgba(180,140,60,0.4);
        }
        #invTopBar {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #0a1035, stop:0.5 #1a2455, stop:1 #0a1035);
            border-bottom: 3px solid rgba(255,215,0,0.5);
        }
        #invTitle {
            color: #ffd700;
            font-size: 18px;
            font-weight: bold;
            background: transparent;
        }
        #invCoins {
            color: #ffd700;
            font-size: 14px;
            font-weight: bold;
            background: rgba(0,0,0,0.25);
            border: 1px solid rgba(255,215,0,0.3);
            border-radius: 6px;
            padding: 4px 14px;
        }
        #invCloseBtn {
            background: rgba(255,255,255,0.10);
            border: none;
            border-radius: 15px;
            color: white;
            font-size: 14px;
            font-weight: bold;
        }
        #invCloseBtn:hover { background: rgba(200,50,50,0.85); }
        #invHint {
            color: rgba(80,50,20,0.7);
            font-size: 11px;
            font-style: italic;
            background: rgba(180,140,60,0.12);
            border: 1px solid rgba(180,140,60,0.25);
            border-radius: 6px;
            padding: 6px 20px;
            margin: 4px 20px;
        }
        #invScroll { background: transparent; border: none; }
        #invScroll QScrollBar:vertical {
            background: rgba(255,255,255,10);
            width: 5px; border-radius: 2px;
        }
        #invScroll QScrollBar::handle:vertical {
            background: rgba(100,60,20,0.4);
            border-radius: 2px;
        }

        /* Slot cards */
        #inventorySlot {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #f5fcff, stop:1 #fdf0c8);
            border: 2px solid rgba(180,140,60,0.6);
            border-radius: 14px;
        }
        #inventorySlot:hover {
            border: 2px solid rgba(255,180,0,0.8);
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #fffff0, stop:1 #bfe8ff);
        }
        #slotName {
            color: #3a2000;
            font-size: 13px;
            font-weight: bold;
            background: transparent;
        }
        #slotQty {
            color: #228b22;
            font-size: 15px;
            font-weight: bold;
            background: transparent;
        }
        #slotQtyEmpty {
            color: #999;
            font-size: 15px;
            background: transparent;
        }
        #slotSellPrice {
            color: #b8860b;
            font-size: 11px;
            background: transparent;
        }
        #sellBtn {
            background: rgba(180,60,20,0.75);
            border: 1px solid rgba(220,80,30,0.7);
            border-radius: 7px;
            color: #ffe0d0;
            font-size: 11px;
            font-weight: bold;
            letter-spacing: 1px;
        }
        #sellBtn:hover { background: rgba(220,80,30,0.85); }
        #sellBtn:disabled {
            background: rgba(100,100,100,0.3);
            color: #888;
            border-color: #555;
        }
        #sellSpinner {
            background: rgba(255,255,255,0.7);
            border: 1px solid rgba(180,140,60,0.5);
            border-radius: 5px;
            color: #3a2000;
            font-size: 12px;
        }
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
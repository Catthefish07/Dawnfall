#include "shopscreen.h"
#include "lobbyscreen.h"
#include <QMessageBox>
#include <QScrollArea>
#include <QStyle>
#include <QFrame>
#include <QDebug>

// ═════════════════════════════════════════════════════════════════════════════
// ShopItemCard
// ═════════════════════════════════════════════════════════════════════════════

ShopItemCard::ShopItemCard(const Item &item, QWidget *parent)
    : QWidget(parent), m_item(item)
{
    buildUi();
}

void ShopItemCard::buildUi()
{
    // Each card fits inside one of the 3 slot frames on shopBuy_item asset
    setFixedSize(200, 180);

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(8, 4, 8, 4);
    vl->setSpacing(0);
    vl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // Item icon — use shopBuy_item asset as card bg, icon on top
    m_iconLbl = new QLabel(this);
    m_iconLbl->setFixedSize(80, 80);
    m_iconLbl->setAlignment(Qt::AlignCenter);
    m_iconLbl->setScaledContents(true);
    {
        QString iconPath =
            m_item.name == "Health Potion" ? "assets/shopscreen/healthpotion" :
                m_item.name == "Mega Potion"   ? "assets/shopscreen/megapotion"   :
                "assets/shopscreen/revivestone";
        QPixmap px = loadAsset(iconPath);
        if (!px.isNull()) m_iconLbl->setPixmap(px);
        else m_iconLbl->setText(m_item.name == "Health Potion" ? "🧪" :
                                   m_item.name == "Mega Potion"   ? "💊" : "💎");
    }

    m_priceLbl = new QLabel(QString("🪙 %1").arg(m_item.price), this);
    m_priceLbl->setAlignment(Qt::AlignCenter);
    m_priceLbl->setObjectName("cardPrice");

    // Qty spinner
    m_qtySpin = new QSpinBox(this);
    m_qtySpin->setRange(1, Shop::maxPerBuy);
    m_qtySpin->setValue(1);
    m_qtySpin->setObjectName("qtySpinner");
    m_qtySpin->setFixedWidth(70);

    m_buyBtn = new QPushButton("BUY", this);
    m_buyBtn->setObjectName("buyBtn");
    m_buyBtn->setFixedHeight(30);
    m_buyBtn->setCursor(Qt::PointingHandCursor);
    connect(m_buyBtn, &QPushButton::clicked, this, [this](){
        emit buyRequested(QString::fromStdString(m_item.name), m_qtySpin->value());
    });

    vl->addWidget(m_iconLbl, 0, Qt::AlignHCenter);
    vl->addSpacing(20);
    vl->addWidget(m_priceLbl);
    vl->addWidget(m_qtySpin, 0, Qt::AlignHCenter);
    vl->addWidget(m_buyBtn);
}

void ShopItemCard::refreshCoins(int coins)
{
    m_coins = coins;
    // Disable buy if can't afford minimum
    bool canAfford = (coins >= m_item.price);
    m_buyBtn->setEnabled(canAfford);
    m_buyBtn->setToolTip(canAfford ? "" : "Not enough coins!");
}

// ═════════════════════════════════════════════════════════════════════════════
// ShopCharCard
// ═════════════════════════════════════════════════════════════════════════════

ShopCharCard::ShopCharCard(const CharacterShop &ch,
                           int currentChapter,
                           QWidget *parent)
    : QWidget(parent), m_char(ch), m_currentChapter(currentChapter)
{
    buildUi();
}

void ShopCharCard::buildUi()
{
    setFixedSize(210, 245);

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(10, 10, 10, 10);
    vl->setSpacing(6);
    vl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // Portrait
    m_portraitLbl = new QLabel(this);
    m_portraitLbl->setFixedSize(150, 180);
    m_portraitLbl->setScaledContents(false);
    m_portraitLbl->setAlignment(Qt::AlignCenter);
    QString charKey = QString::fromStdString(m_char.name).toLower().replace(" ", "");
    QMap<QString,QString> pixMap = {
                                     {"lynn",     "assets/shopscreen/lynn_pixelFull"},
                                     {"cedric",   "assets/shopscreen/ced_pixelFull"},
                                     {"ben",      "assets/shopscreen/den_pixelFull"},
                                     {"kae",      "assets/shopscreen/kae_pixelFull"},
                                     {"zey",      "assets/shopscreen/zey_pixelFull"},
                                     {"anakagung","assets/shopscreen/agung_pixelFull"},
                                     };
    QPixmap px = loadAsset(pixMap.value(charKey,
                                        QString("assets/profilepic/%1_pfp").arg(charKey)));
    if (!px.isNull()) m_portraitLbl->setPixmap(
            px.scaled(130, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else m_portraitLbl->setText("👤");

    m_nameLbl = new QLabel(QString::fromStdString(m_char.name), this);
    m_nameLbl->setAlignment(Qt::AlignCenter);
    m_nameLbl->setObjectName("cardName");

    m_priceLbl = new QLabel(
        m_char.isFree ? "FREE" : QString("🪙 %1").arg(m_char.price), this);
    m_priceLbl->setAlignment(Qt::AlignCenter);
    m_priceLbl->setObjectName(m_char.isFree ? "cardFree" : "cardPrice");

    // Status label (locked / available / owned)
    m_statusLbl = new QLabel(this);
    m_statusLbl->setAlignment(Qt::AlignCenter);
    m_statusLbl->setObjectName("cardStatus");

    m_buyBtn = new QPushButton(this);
    m_buyBtn->setObjectName("buyBtn");
    m_buyBtn->setFixedHeight(30);
    m_buyBtn->setCursor(Qt::PointingHandCursor);

    bool available = (m_currentChapter >= m_char.worldRequired);
    bool owned     = m_char.isOwned || m_char.isFree;

    if (owned) {
        m_statusLbl->setText("✓ Owned");
        m_buyBtn->setText("OWNED");
        m_buyBtn->setEnabled(false);
        m_buyBtn->setObjectName("ownedBtn");
    } else if (!available) {
        m_statusLbl->setText(QString("🔒 Ch.%1 required").arg(m_char.worldRequired));
        m_buyBtn->setText("LOCKED");
        m_buyBtn->setEnabled(false);
        m_buyBtn->setObjectName("lockedBtn");
    } else {
        m_statusLbl->setText("Available");
        m_buyBtn->setText("BUY");
        m_buyBtn->setEnabled(true);
        connect(m_buyBtn, &QPushButton::clicked, this, [this](){
            emit buyRequested(QString::fromStdString(m_char.name));
        });
    }

    vl->addWidget(m_portraitLbl, 0, Qt::AlignHCenter);
    vl->addWidget(m_nameLbl);
    vl->addWidget(m_priceLbl);
    vl->addWidget(m_statusLbl);
    vl->addWidget(m_buyBtn);
}

void ShopCharCard::setOwned(bool owned)
{
    m_char.isOwned = owned;
    if (owned) {
        m_buyBtn->setText("OWNED");
        m_buyBtn->setEnabled(false);
        m_buyBtn->setObjectName("ownedBtn");
        m_statusLbl->setText("✓ Owned");
    }
    m_buyBtn->style()->unpolish(m_buyBtn);
    m_buyBtn->style()->polish(m_buyBtn);
}

// ═════════════════════════════════════════════════════════════════════════════
// ShopScreen
// ═════════════════════════════════════════════════════════════════════════════

ShopScreen::ShopScreen(Shop      &shop,
                       Inventory &inventory,
                       int       &coins,
                       int        currentChapter,
                       QWidget   *parent)
    : QDialog(parent)
    , m_shop(shop)
    , m_inventory(inventory)
    , m_coins(coins)
    , m_currentChapter(currentChapter)
{
    setWindowTitle("Shop");
    setMinimumSize(1200, 720);
    setObjectName("shopScreen");
    buildUi();
    applyStyle();
}

void ShopScreen::buildUi()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Stack first so buildTopBar can connect to it
    m_stack = new QStackedWidget(this);
    m_stack->setObjectName("shopStack");
    m_stack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    buildItemPage();
    buildCharPage();

    // Topbar AFTER stack is ready, inserts itself at top of layout
    buildTopBar();
}
void ShopScreen::buildItemPage()
{
    // Page background: shopBuy_item asset
    QWidget *page = new QWidget;
    page->setObjectName("itemPage");

    QVBoxLayout *vl = new QVBoxLayout(page);
    vl->setContentsMargins(0, 0, 0, 30);
    vl->setSpacing(0);

    // Asset background for item page
    QLabel *pageBg = new QLabel(page);
    QPixmap itemBg = loadAsset("assets/shopscreen/shopBuy_item");
    // Asset ratio: 1254×736 → at 860 wide → height = 860*(736/1254) ≈ 505
    int bgW = 1200, bgH = int(1280.0 * 736.0 / 1254.0);
    pageBg->setFixedSize(bgW, bgH);
    if (!itemBg.isNull())
        pageBg->setPixmap(itemBg.scaled(bgW, bgH,
                                        Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation));
    else
        pageBg->setStyleSheet("background:rgba(255,255,255,0.05);border-radius:12px;");

    // Cards overlaid on the 3 top slots of the asset
    // The 3 slots start at roughly x=155,y=80 spaced ~270px apart, height ~270px
    QVector<QString> itemNames = {"Health Potion", "Mega Potion", "Revive Stone"};
    QVector<int>     itemPrices = {1500, 3000, 5000};
    int slotX[3] = {250, 520, 790};
    int slotY    = 175;

    for (int i = 0; i < 3; ++i) {
        // Find item in stock
        Item found;
        found.name  = itemNames[i].toStdString();
        found.price = itemPrices[i];
        for (const Item &it : m_shop.itemStock)
            if (it.name == found.name) { found = it; break; }

        ShopItemCard *card = new ShopItemCard(found, pageBg);
        card->move(slotX[i], slotY);
        card->raise();
        card->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        card->refreshCoins(m_coins);
        connect(card, &ShopItemCard::buyRequested,
                this, &ShopScreen::onItemBuy);
        m_itemCards.append(card);
    }

    vl->addSpacing(5);
    vl->addWidget(pageBg, 0, Qt::AlignHCenter | Qt::AlignTop);
    m_stack->addWidget(page);   // index 0 = items
}

void ShopScreen::buildCharPage()
{
    QWidget *page = new QWidget;
    page->setObjectName("charPage");

    QVBoxLayout *vl = new QVBoxLayout(page);
    vl->setContentsMargins(0, 0, 0, 30);
    vl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    vl->setSpacing(0);

    // Asset background for character page
    QLabel *pageBg = new QLabel(page);
    QPixmap charBg = loadAsset("assets/shopscreen/shopBuy_char");
    // Same ratio assumption as item bg
    int bgW = 1200, bgH = int(1200.0 * 650.0 / 1254.0);
    pageBg->setFixedSize(bgW, bgH);
    if (!m_itemCards.isEmpty())
        pageBg->stackUnder(m_itemCards[0]);
    if (!m_charCards.isEmpty())
        pageBg->stackUnder(m_charCards[0]);
    if (!charBg.isNull())
        pageBg->setPixmap(charBg.scaled(bgW, bgH,
                                        Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation));
    else
        pageBg->setStyleSheet("background:rgba(255,255,255,0.05);border-radius:12px;");

    // 6 characters for sale (skip Joy, Ethan, Hubert — they're free/auto)
    // From shop.cpp: Lynn, Ben, Cedric, Kae, Zey, Anak Agung
    // Layout: 3 per row, 2 rows
    int slotX[3] = {220, 445, 670};
    int slotY[2] = {100, 340};
    int cardIdx   = 0;

    for (const CharacterShop &ch : m_shop.getCharacterStock()) {
        if (ch.isFree) continue;   // skip free characters
        if (cardIdx >= 6) break;

        int row = cardIdx / 3;
        int col = cardIdx % 3;

        ShopCharCard *card = new ShopCharCard(ch, m_currentChapter, pageBg);
        card->move(slotX[col], slotY[row]);
        connect(card, &ShopCharCard::buyRequested,
                this, &ShopScreen::onCharBuy);
        m_charCards.append(card);
        cardIdx++;
    }

    vl->addWidget(pageBg, 0, Qt::AlignHCenter);
    m_stack->addWidget(page);   // index 1 = characters
}

void ShopScreen::buildTopBar()
{
    // Top bar: [itemShop btn] [characterShop btn]  ── stretch ──  [coins] [close]
    QWidget *bar = new QWidget(this);
    bar->setObjectName("shopTopBar");
    bar->setFixedHeight(55);

    QHBoxLayout *hl = new QHBoxLayout(bar);
    hl->setContentsMargins(20, 2, 40, 2);
    hl->setSpacing(12);

    // Tab buttons
    m_itemTabBtn = new PixmapButton("assets/shopscreen/itemShop", bar);
    m_itemTabBtn->setFixedSize(160, 57);
    m_itemTabBtn->setToolTip("Items");
    connect(m_itemTabBtn, &QPushButton::clicked, this, &ShopScreen::switchToItems);

    m_charTabBtn = new PixmapButton("assets/shopscreen/characterShop", bar);
    m_charTabBtn->setFixedSize(160, 50);
    m_charTabBtn->setToolTip("Characters");
    connect(m_charTabBtn, &QPushButton::clicked, this, &ShopScreen::switchToChars);

    hl->addWidget(m_itemTabBtn);
    hl->addWidget(m_charTabBtn);
    hl->addStretch(1);

    // Coins display
    m_coinsLbl = new QLabel(QString("🪙  %1").arg(m_coins), bar);
    m_coinsLbl->setObjectName("coinsDisplay");
    hl->addWidget(m_coinsLbl);

    hl->addSpacing(16);

    // Close button
    QPushButton *closeBtn = new QPushButton("✕", bar);
    closeBtn->setFixedSize(32, 32);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setObjectName("shopCloseBtn");
    connect(closeBtn, &QPushButton::clicked, this, [this](){
        emit shopClosed();
        accept();
    });
    hl->addWidget(closeBtn);

    QVBoxLayout *root = qobject_cast<QVBoxLayout*>(layout());
    if (root) {
        root->insertWidget(0, bar);
        root->addWidget(m_stack, 1);
    }
}

void ShopScreen::switchToItems()
{
    m_stack->setCurrentIndex(0);
}

void ShopScreen::switchToChars()
{
    m_stack->setCurrentIndex(1);
}

void ShopScreen::refreshCoinsDisplay()
{
    m_coinsLbl->setText(QString("🪙  %1").arg(m_coins));
    for (ShopItemCard *c : m_itemCards)
        c->refreshCoins(m_coins);
}

void ShopScreen::onItemBuy(const QString &itemName, int qty)
{
    bool ok = m_shop.buyItem(itemName.toStdString(), qty, m_inventory);
    if (ok) {
        m_coins = m_inventory.getCoins();
        refreshCoinsDisplay();
        QMessageBox::information(this, "Purchased!",
                                 QString("Bought %1x %2!").arg(qty).arg(itemName));
    } else {
        QMessageBox::warning(this, "Cannot Buy",
                             "Not enough coins or inventory full!");
    }
}

void ShopScreen::onCharBuy(const QString &charName)
{
    bool ok = m_shop.buyCharacter(charName.toStdString(),
                                  m_inventory,
                                  m_currentChapter);
    if (ok) {
        m_coins = m_inventory.getCoins();
        refreshCoinsDisplay();

        // Update the card to show owned
        for (ShopCharCard *c : m_charCards) {
            // find matching card by checking name label
            if (c->findChild<QLabel*>("cardName") &&
                c->findChild<QLabel*>("cardName")->text() == charName)
                c->setOwned(true);
        }

        QMessageBox::information(this, "Unlocked!",
                                 QString("%1 has joined your roster!").arg(charName));
    } else {
        QString reason = "Cannot purchase.";
        if (m_coins < 500)
            reason = "Not enough coins!";
        else if (charName == "Anak Agung" && m_currentChapter < 4)
            reason = "Finish Peak Mountain first to unlock Anak Agung!";
        QMessageBox::warning(this, "Cannot Buy", reason);
    }
}

void ShopScreen::applyStyle()
{
    setStyleSheet(R"(
        #shopScreen { background: #f5f0e0; }
        #shopTopBar {
            background: rgba(0,0,0,0.35);
            border-bottom: 1px solid rgba(255,255,255,0.08);
        }
        #coinsDisplay {
            color: #ffd700;
            font-size: 13px;
            font-weight: bold;
            background: rgba(0,0,0,0.25);
            border: 1px solid rgba(255,215,0,0.3);
            border-radius: 6px;
            padding: 6px 20px;
            min-width: 100px;
        }
        #shopCloseBtn {
            background: rgba(255,255,255,0.10);
            border: none; border-radius: 16px;
            color: white; font-size: 14px; font-weight: bold;
        }
        #shopCloseBtn:hover { background: rgba(200,50,50,0.85); }
        #shopStack { background: transparent; }
        #itemPage, #charPage { background: transparent; }

        /* Cards */
        ShopItemCard, ShopCharCard {
            background: rgba(255,248,220,0.55);
            border: 1px solid rgba(180,140,60,0.4);
            border-radius: 10px;
        }
        #cardName {
            color: #3a2000;
            font-size: 13px;
            font-weight: bold;
            background: transparent;
        }
        #cardPrice {
            color: #b8860b;
            font-size: 12px;
            font-weight: bold;
            background: transparent;
        }
        #cardFree {
            color: #228b22;
            font-size: 12px;
            font-weight: bold;
            background: transparent;
        }
        #cardStatus {
            color: #5a5a5a;
            font-size: 11px;
            background: transparent;
        }
        #buyBtn {
            background: rgba(34,139,34,0.75);
            border: 1px solid rgba(34,197,94,0.7);
            border-radius: 7px;
            color: #ccffcc;
            font-size: 11px;
            font-weight: bold;
            letter-spacing: 1px;
        }
        #buyBtn:hover { background: rgba(34,197,94,0.85); }
        #buyBtn:disabled { background: rgba(100,100,100,0.3); color: #888; border-color: #555; }
        #ownedBtn {
            background: rgba(70,130,180,0.4);
            border: 1px solid rgba(100,160,210,0.6);
            border-radius: 7px;
            color: #aad4f5;
            font-size: 11px;
            font-weight: bold;
        }
        #lockedBtn {
            background: rgba(80,80,80,0.35);
            border: 1px solid rgba(100,100,100,0.4);
            border-radius: 7px;
            color: #888;
            font-size: 11px;
        }
        #qtySpinner {
            background: rgba(255,255,255,0.6);
            border: 1px solid rgba(180,140,60,0.5);
            border-radius: 5px;
            color: #3a2000;
            font-size: 12px;
        }
        #descLabel {
            color: #4a3000;
            font-size: 12px;
            font-style: italic;
            background: transparent;
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
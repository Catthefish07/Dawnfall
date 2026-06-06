#ifndef SHOPSCREEN_H
#define SHOPSCREEN_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpinBox>
#include <QVector>
#include <QString>
#include "shop.h"
#include "inventory.h"

class PixmapButton;

//for items
class ShopItemCard : public QWidget
{
    Q_OBJECT
public:
    explicit ShopItemCard(const Item &item, QWidget *parent = nullptr);
    void refreshCoins(int coins);

signals:
    void buyRequested(const QString &itemName, int qty);

private:
    void buildUi();
    Item m_item;
    QLabel *m_iconLbl = nullptr;
    QLabel *m_nameLbl = nullptr;
    QLabel *m_priceLbl = nullptr;
    QSpinBox *m_qtySpin = nullptr;
    QPushButton *m_buyBtn = nullptr;
    int m_coins = 0;
};

//for character
class ShopCharCard : public QWidget
{
    Q_OBJECT
public:
    explicit ShopCharCard(const CharacterShop &ch,
                          int currentChapter,
                          QWidget *parent = nullptr);
    void setOwned(bool owned);

signals:
    void buyRequested(const QString &charName);

private:
    void buildUi();
    CharacterShop m_char;
    int m_currentChapter;
    QLabel *m_portraitLbl = nullptr;
    QLabel *m_nameLbl = nullptr;
    QLabel *m_priceLbl = nullptr;
    QLabel *m_statusLbl = nullptr;
    QPushButton *m_buyBtn = nullptr;
};

//main
class ShopScreen : public QDialog
{
    Q_OBJECT
public:
    explicit ShopScreen(Shop &shop,
                        Inventory &inventory,
                        int &coins,
                        int currentChapter = 0,
                        QWidget *parent = nullptr);

signals:
    void shopClosed();

private slots:
    void onItemBuy(const QString &itemName, int qty);
    void onCharBuy(const QString &charName);
    void switchToItems();
    void switchToChars();

private:
    void buildUi();
    void buildTopBar();
    void buildItemPage();
    void buildCharPage();
    void refreshCoinsDisplay();
    void applyStyle();

    Shop &m_shop;
    Inventory &m_inventory;
    int &m_coins;
    int m_currentChapter;
\
    QLabel  *m_coinsLbl = nullptr;
    QStackedWidget *m_stack = nullptr;
    PixmapButton *m_itemTabBtn = nullptr;   // itemShop.png
    PixmapButton *m_charTabBtn = nullptr;   // characterShop.png

    //to refresh coins display
    QVector<ShopItemCard*> m_itemCards;
    QVector<ShopCharCard*> m_charCards;
};

#endif
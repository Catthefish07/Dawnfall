#include "mainmenuscreen.h"
#include "battlescreen.h"
#include "enemy.h"
#include "minion.h"
#include "character.h"
#include "archer.h"
#include "tank.h"
#include "warrior.h"
#include "inventory.h"

#include <QApplication>
#include <QStackedWidget>
#include <QScreen>
#include <QDebug>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Get screen size
    QScreen *screen = QApplication::primaryScreen();
    QRect geo = screen->geometry();
    int W = geo.width();
    int H = geo.height();

    QStackedWidget *screens = new QStackedWidget();
    screens->setWindowTitle("DawnFall");
    screens->showFullScreen();

    MainMenuScreen *menu = new MainMenuScreen();
    screens->addWidget(menu);
    screens->setCurrentIndex(0);

    QObject::connect(menu, &MainMenuScreen::newGameClicked, [=]() {
        vector<Character*> party;
        party.push_back(new Archer("Ethan",  100, 22, 10, 18));
        party.push_back(new Warrior("MC",    120, 18, 12, 15));
        party.push_back(new Tank("Hubert",   180, 14, 25, 10));

        vector<Enemy*> enemies;
        enemies.push_back(new Minion("Wolf",  60, 25, 4, 9,  1));
        enemies.push_back(new Minion("Snake", 75, 30, 6, 11, 1));
        enemies.push_back(new Minion("Slime", 40, 20, 2, 8,  1));

        Inventory *inventory = new Inventory();

        inventory->setQuantity("Health Potion", 3);
        inventory->setQuantity("Mega Potion", 1);
        inventory->setQuantity("Revive Stone", 1);

        BattleScreen *battle = new BattleScreen(party,enemies,inventory,
                                                STORY_BATTLE,W,H);

        screens->addWidget(battle);
        screens->setCurrentIndex(screens->count() - 1);

        QObject::connect(battle, &BattleScreen::battleFinished, [=](bool victory) {
            Q_UNUSED(victory);

            screens->setCurrentIndex(0);
        });
    });

    QObject::connect(menu, &MainMenuScreen::exitClicked, []() {
        QApplication::quit();
    });

    return a.exec();
}
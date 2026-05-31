#include "mainmenuscreen.h"
#include "battlescreen.h"
#include "enemy.h"
#include "hero.h"
#include <QApplication>
#include <QStackedWidget>
#include <QScreen>
#include <QDebug>

class TestHero : public Hero {
public:
    TestHero(string name) : Hero(name, 120, 22, 10, 15, "", "", true) {}
    int dealDamage() const override { return (int)(atk * 1.2 * level) + 5; }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QScreen *screen = QApplication::primaryScreen();
    QRect geo = screen->geometry();
    int W = geo.width();
    int H = geo.height();

    QStackedWidget *screens = new QStackedWidget();
    screens->setWindowTitle("DawnFall");
    screens->setStyleSheet("background-color: black;"); // fix white bg
    screens->showFullScreen();

    MainMenuScreen *menu = new MainMenuScreen(nullptr);
    screens->addWidget(menu);
    screens->setCurrentIndex(0);

    QObject::connect(menu, &MainMenuScreen::newGameClicked, [=]() {
        vector<Hero*> party;
        party.push_back(new TestHero("Ethan"));
        party.push_back(new TestHero("MC"));
        party.push_back(new TestHero("Hubert"));

        vector<Enemy*> enemies;
        enemies.push_back(new CommonEnemy("Wolf",  60, 12, 4, 9,  18, 1, ""));
        enemies.push_back(new CommonEnemy("Snake", 75, 15, 6, 11, 25, 1, ""));

        BattleScreen *battle = new BattleScreen(party, enemies, STORY_BATTLE, W, H);
        screens->addWidget(battle);
        screens->setCurrentIndex(screens->count() - 1);
    });

    QObject::connect(menu, &MainMenuScreen::exitClicked, []() {
        QApplication::quit();
    });

    return a.exec();
}
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <cstring>
#include "lobbyscreen.h"
#include "partymanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // ── Working directory fix
    {
        QDir dir = QDir::current();
        for (int i = 0; i < 6; ++i) {
            if (dir.exists("assets")) {
                QDir::setCurrent(dir.absolutePath());
                qDebug() << "[main] Working dir:" << dir.absolutePath();
                break;
            }
            if (!dir.cdUp()) break;
        }
    }

    QApplication::setApplicationName("MyGame");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("MyStudio");

    app.setStyleSheet(R"(
        QWidget {
            font-family: 'Bahnschrift', 'Trebuchet MS', sans-serif;
            font-size: 13px;
            color: #e2e8f0;
        }
        QDialog     { background: #0f172a; }
        QMessageBox { background: #0f172a; }
        QMessageBox QLabel { color: #e2e8f0; }
        QMessageBox QPushButton {
            background: rgba(255,255,255,12);
            border: 1px solid rgba(255,255,255,25);
            border-radius: 6px; color: #e2e8f0;
            padding: 5px 18px; min-width: 70px;
        }
        QMessageBox QPushButton:hover { background: rgba(255,255,255,22); }
        QScrollBar:vertical {
            background: rgba(255,255,255,10);
            width: 6px; border-radius: 3px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(255,255,255,40);
            border-radius: 3px; min-height: 20px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical { height: 0; }
    )");

    // ── Game state
    SaveManager  saveManager;
    int          saveSlot = 0;

    Inventory    inventory;
    Shop         shop;
    PartyManager partyManager;

    // loadGame() takes slot + inventory + shop + partyManager
    PlayerRecord playerRecord = saveManager.loadGame(saveSlot, inventory, shop, partyManager);

    // occupied=false means the slot was empty / fresh game
    if (!playerRecord.occupied) {
        strncpy(playerRecord.playerUsername, "Hero",
                sizeof(playerRecord.playerUsername) - 1);
        playerRecord.playerUsername[sizeof(playerRecord.playerUsername) - 1] = '\0';

        playerRecord.currentChapter = 0;
        playerRecord.coins          = 200;
        playerRecord.avatarIndex    = 0;
        playerRecord.occupied       = true;

        memset(playerRecord.characterLevels,   0, sizeof(playerRecord.characterLevels));
        memset(playerRecord.characterUnlocked, 0, sizeof(playerRecord.characterUnlocked));

        // Generate UID — fits in char[8]: "DF-XXXX" = 7 chars + null
        int uidNum = (int)(QDateTime::currentMSecsSinceEpoch() % 10000);
        snprintf(playerRecord.UID, sizeof(playerRecord.UID), "DF-%04d", uidNum);
    }

    int &coins = playerRecord.coins;

    // ── Character roster
    vector<Character*> allCharacters;
    // Mage *mage = new Mage("Aelric");
    // allCharacters.push_back(mage);

    // ── Build lobby
    LobbyScreen lobby(playerRecord,
                      allCharacters,
                      shop,
                      inventory,
                      coins,
                      saveManager,
                      partyManager,
                      saveSlot);

    QObject::connect(&lobby, &LobbyScreen::battleRequested,
                     [&](const QString &locationId) {
                         qDebug() << "[main] Battle requested for:" << locationId;
                         // TODO: show BattleScreen, on finish:
                         //   playerRecord.currentChapter++;
                         //   saveManager.saveGame(saveSlot, playerRecord, inventory, shop, partyManager);
                         //   lobby.refreshStats();
                         //   lobby.show();
                     });

    lobby.setWindowTitle("My Game — Lobby");
    lobby.resize(1080, 680);
    lobby.show();

    int result = app.exec();

    for (Character *c : allCharacters) delete c;
    return result;
}
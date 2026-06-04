#ifndef BATTLESCREEN_H
#define BATTLESCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QListWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "character.h"
#include "enemy.h"
#include "battlesystem.h"
#include "inventory.h"
#include <vector>
using namespace std;

class BattleScreen : public QWidget {
    Q_OBJECT
public:
    explicit BattleScreen(vector<Character*> partyIn,
                          vector<vector<string>> wavesIn,
                          Inventory* inventoryIn,
                          BattleMode mode, int W, int H, QWidget *parent = nullptr);
signals:
    void battleFinished(bool victory);

private:
    // ── Logic ──
    BattleSystem*       battleSystem;
    BattleMode          mode;
    vector<Character*>  party;
    vector<Enemy*>      enemies;
    Inventory*          inventory;
    int                 currentEnemyIndex;
    int                 W, H;

    // ── Turn Order ──
    struct TurnEntry {
        string name;
        int speed;
        bool isParty;
        int index;
    };
    vector<TurnEntry>   combinedOrder;
    int                 combinedIndex;
    void                buildTurnOrder();
    void                advanceTurn();

    // ── UI Widgets ──
    QLabel*         bgLabel;
    QLabel*         chapterLabel;
    QLabel*         turnLabel;
    QLabel*         dialoguePortrait;
    QLabel*         dialogueText;
    QListWidget*    battleLog;

    // Party
    QLabel*         characterSprites[3];
    QProgressBar*   characterHpBars[3];
    QLabel*         characterHpText[3];
    QLabel*         characterPfp[3];

    // Enemy
    QLabel*         enemySprite;
    QLabel*         enemyNameLabel;
    QProgressBar*   enemyHpBar;
    QLabel*         enemyHpText;
    QLabel*         enemyNameLabels[3];
    QProgressBar*   enemyHpBars3[3];
    QLabel*         enemySprites3[3];

    // Buttons
    QPushButton*    btnFight;
    QPushButton*    btnSkill;
    QPushButton*    btnItem;
    QPushButton*    btnFlee;

    // Turn order labels
    QLabel*         turnOrderLabels[4];

    // ── Setup ──
    void setupUI();
    void setupConnections();
    void updatePartyUI();
    void updateEnemyUI();
    void updateTurnOrder();

    void updatePortrait(const QString& portraitPath);
    QString getPortraitPathByName(const string& name) const;
    void updatePortraitByName(const string& name);

    void addBattleLog(const QString& entry);
    void loadNextEnemy();
    void showVictoryScreen();
    void showDefeatScreen();

    // ── Animations ──
    void playAttackAnimation(int characterIndex);
    void playMageAnimation(int characterIndex);
    void playEnemyHitFlash();
    void playShakeAnimation(QLabel* sprite);
    void playDefeatAnimation();
    void playHpBarAnimation(QProgressBar* bar, int oldVal, int newVal);
    void playEnemyTurn();

    // ── Input Lock ──
    bool isAnimating;
    void setButtonsEnabled(bool enabled);
    void lockAllInput();
    void unlockAllInput();
    bool eventFilter(QObject *obj, QEvent *event) override;

    // ── Enemy Targeting ──
    int  selectedEnemyIndex = 0;
    int  getFirstAliveEnemyIndex() const;
    void selectEnemyTarget(int enemyIndex);
    void updateEnemyTargetHighlight();

    // ── Inventory UI ──
    QLabel*         inventoryPanel  = nullptr;
    QPushButton*    itemButtons[3];
    QPushButton*    btnCancelInventory = nullptr;
    bool            isInventoryOpen = false;
    int             selectedItemIndex = -1;

    void openInventoryPanel();
    void closeInventoryPanel();
    void showItemChoices();
    void showPartyTargetChoices(int itemIndex);

    // ── Ally Selection ──
    enum class SelectionMode { NONE, SKILL_ALLY, ITEM_ALLY };
    SelectionMode   selectionMode = SelectionMode::NONE;
    int             pendingSkillCharIndex = -1;
    void            updateAllyHighlight();
    void            enlargeAllySprite(int index);
    void            resetAllySpriteSize(int index);
    void            useAbilityOnAlly(int targetPartyIndex);

    // ── Skill Cooldown ──
    QLabel* skillCooldownLabels[3];
    void    updateSkillCooldowns();

    // ── Coins ──
    int totalCoinsEarned = 0;

    // ── Wave System ──
    vector<vector<string>> waves;
    int currentWave = 0;
    QLabel* waveLabel = nullptr;
    void loadNextWave();

    // ── Checking Stats ──
    QLabel* statsPanel = nullptr;
    void showStatsPanel(int hp, int maxHp, int atk, int def, int spd, QString name, QPoint pos);
    void hideStatsPanel();

};

#endif // BATTLESCREEN_H
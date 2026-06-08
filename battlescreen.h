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
                          BattleMode mode,
                          int W,
                          int H,
                          QString backgroundPath = ":/assets/background/forest_battle.png",
                          QWidget *parent = nullptr);

signals:
    void battleFinished(bool victory);

private:
    // ── Constants ──
    static constexpr int MAX_PARTY_SIZE = 3;
    static constexpr int MAX_ENEMY_SIZE = 3;
    static constexpr int MAX_TURN_LABELS = 4;

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

    void buildTurnOrder();
    void advanceTurn();

    // ── UI Widgets ──
    QLabel*         bgLabel;
    QLabel*         chapterLabel;
    QLabel*         turnLabel;
    QLabel*         dialoguePortrait;
    QLabel*         dialogueText;
    QListWidget*    battleLog;

    // ── Party UI ──
    QLabel*         characterSprites[MAX_PARTY_SIZE];
    QProgressBar*   characterHpBars[MAX_PARTY_SIZE];
    QLabel*         characterHpText[MAX_PARTY_SIZE];
    QLabel*         characterPfp[MAX_PARTY_SIZE];
    QLabel*         characterNameLabels[MAX_PARTY_SIZE];

    // Dynamic party display
    void updatePartyUI();
    void updatePartySprites();
    void updatePartyHUD();
    void positionPartySprites();
    void positionPartyHUD();
    void hideAllPartySlots();

    // Helper party
    bool isValidPartyIndex(int index) const;
    int  getActivePartySize() const;

    // ── Enemy UI ──
    QLabel*         enemySprite;
    QLabel*         enemyNameLabel;
    QProgressBar*   enemyHpBar;
    QLabel*         enemyHpText;
    QLabel*         enemyNameLabels[MAX_ENEMY_SIZE];
    QProgressBar*   enemyHpBars3[MAX_ENEMY_SIZE];
    QLabel*         enemySprites3[MAX_ENEMY_SIZE];

    void updateEnemyUI();

    // ── Buttons ──
    QPushButton*    btnFight;
    QPushButton*    btnSkill;
    QPushButton*    btnItem;
    QPushButton*    btnFlee;

    // ── Turn order labels ──
    QLabel*         turnOrderLabels[MAX_TURN_LABELS];

    void updateTurnOrder();

    // ── Setup ──
    void setupUI();
    void setupConnections();
    QString m_backgroundPath;

    void updatePortrait(const QString& portraitPath);
    QString getPortraitPathByName(const string& name) const;
    void updatePortraitByName(const string& name);

    void addBattleLog(const QString& entry);
    void loadNextEnemy();

    // ── Animations ──
    void playAttackAnimation(int characterIndex);
    void playMageAnimation(int characterIndex);
    void playShakeAnimation(QLabel* sprite);
    void playDefeatAnimation();
    void playHpBarAnimation(QProgressBar* bar, int oldVal, int newVal);
    void playEnemyTurn();

    QString getIdleSpritePathByName(const string& name) const;
    QStringList getBasicAttackFramesByName(const string& name) const;
    QStringList getSkillFramesByName(const string& name) const;
    QString getEnemyIdleSpritePath(const string& name) const;
    QStringList getEnemyAttackFrames(const string& name) const;

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
    QLabel* enemyTargetIndicators[MAX_ENEMY_SIZE];

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
    enum class SelectionMode {
        NONE,
        SKILL_ALLY,
        ITEM_ALLY
    };

    SelectionMode   selectionMode = SelectionMode::NONE;
    int             pendingSkillCharIndex = -1;

    void updateAllyHighlight();
    void enlargeAllySprite(int index);
    void resetAllySpriteSize(int index);
    void useAbilityOnAlly(int targetPartyIndex);

    // ── Skill Cooldown ──
    QLabel* skillCooldownLabels[MAX_PARTY_SIZE];
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
    void showStatsPanel(int hp,
                        int maxHp,
                        int atk,
                        int def,
                        int spd,
                        QString name,
                        QPoint pos);
    void hideStatsPanel();

    // ── Autoplay ──
    bool isAutoplay = false;
    QPushButton* btnAutoplay = nullptr;
    void executeAutoTurn();

    // ── Battle Guide ──
    void showBattleGuidePopup();
    bool battleGuideShown = false;

    // ── Battle Result ──
    void showVictoryScreen();
    void showDefeatScreen();
    void showBattleEndBanner(bool victory);

    bool battleEnding = false;



};

#endif // BATTLESCREEN_H
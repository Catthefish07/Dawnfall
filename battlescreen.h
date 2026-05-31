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
#include "hero.h"
#include "enemy.h"
#include "battlesystem.h"
#include <vector>
using namespace std;

// BattleScreen — core battle UI (Deltarune-inspired)
// Member 4 — Frontend 2: Battle UI, Animations & Dialogue
class BattleScreen : public QWidget {
    Q_OBJECT
public:
    explicit BattleScreen(vector<Hero*> partyIn, vector<Enemy*> enemiesIn,
                          BattleMode mode, int W, int H, QWidget *parent = nullptr);
signals:
    void battleFinished(bool victory);

private:
    // Logic
    BattleSystem    *battleSystem;
    BattleMode       mode;
    vector<Hero*>    party;
    vector<Enemy*>   enemies;
    int              currentEnemyIndex;

    // Background
    QLabel          *bgLabel;
    QLabel          *chapterLabel;
    QLabel          *turnLabel;

    // Party (left side) — 4 slots
    QLabel          *heroSprites[4];
    QLabel          *heroNames[4];
    QProgressBar    *heroHpBars[4];
    QLabel          *heroHpText[4];
    QLabel          *heroPfp[3];

    // Enemy (right side)
    QLabel          *enemySprite;
    QLabel          *enemyNameLabel;
    QProgressBar    *enemyHpBar;
    QLabel          *enemyHpText;
    QLabel          *enemyNameLabels[3];
    QProgressBar    *enemyHpBars3[3];
    QLabel          *enemySprites3[3];
    QLabel          *enemyQueueDots[3];

    // Dialogue + log
    QLabel          *dialoguePortrait;
    QLabel          *dialogueText;
    QListWidget     *battleLog;

    // Action buttons
    QPushButton     *btnFight;
    QPushButton     *btnItem;
    QPushButton     *btnDefend;
    QPushButton     *btnSkill;

    // Turn order labels
    QLabel          *turnOrderLabels[8];

    // Sprite animation timer
    QTimer          *spriteTimer;

    void setupUI();
    void setupConnections();
    void updatePartyUI();
    void updateEnemyUI();
    void updateTurnOrder();
    void addBattleLog(const QString& entry);
    void loadNextEnemy();
    void showVictoryScreen();
    void showDefeatScreen();

    // ── Animations (Member 4 implements these) ──
    void playAttackAnimation(int heroIndex);
    void playMageAnimation(int heroIndex);
    void playEnemyHitFlash();
    void playShakeAnimation(QLabel* sprite);
    void playDefeatAnimation();
    void playHpBarAnimation(QProgressBar* bar, int oldVal, int newVal);
    void playEnemyTurn();

    // Delay during attack
    bool isAnimating;
    void setButtonsEnabled(bool enabled);

private:
    int W, H; // screen dimensions
};

#endif // BATTLESCREEN_H

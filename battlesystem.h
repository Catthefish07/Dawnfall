#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H

#include "character.h"
#include "enemy.h"
#include "inventory.h"
#include "skill.h"
#include <vector>
#include <string>

enum BattleMode {
    STORY_BATTLE,
    FARMING_BATTLE
};

enum BattleResult {
    ONGOING,
    VICTORY,
    DEFEAT,
    FLED
};

class BattleSystem {
private:
    std::vector<Character*> party;
    std::vector<Enemy*> enemies;
    Inventory& inventory;
    BattleMode mode;
    BattleResult result;
    int currentTurn;
    std::string battleLog;
    bool autoplayEnabled;

    void autoTurn(Character* character);
    void manualTurn(Character* character);
    Enemy* getWeakestEnemy();
    int findReadySkill(Character* character, SkillType type);

    Enemy* chooseEnemyTarget();
    Character* chooseAllyTarget();
    bool chooseAndUseSkill(Character* character);
    void showEnemyStats();

public:
    BattleSystem(const std::vector<Character*>& party, const std::vector<Enemy*>& enemies, Inventory& inventory, BattleMode mode);

    BattleResult startBattle();
    BattleResult processTurn();

    void playerTurn(Character* character);
    void enemyTurn(Enemy* enemy);

    std::string characterAttack(Character* character, Enemy* target);
    std::string characterUseSkill(Character* user, int skillIndex, Enemy* enemyTarget, Character* allyTarget);
    std::string characterUseItem(Character* character, int itemIndex);
    bool attemptFlee();

    void setAutoplay(bool on);
    bool isAutoplayEnabled() const;

    bool allCharactersDead() const;
    bool allEnemiesDead() const;

    int calculateCoinReward() const;
    void distributeRewards();

    Character* getLowestHpCharacter();

    std::string getBattleLog() const;
    int getTurnNumber() const;
    BattleResult getResult() const;
};

#endif // BATTLESYSTEM_H
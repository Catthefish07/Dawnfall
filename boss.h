#ifndef BOSS_H
#define BOSS_H

#include "enemy.h"

class Boss : public Enemy
{
private:
    static const int AOE_COOLDOWN = 2;
    static const int HEAL_COOLDOWN = 3;

    int currentAoeCooldown;
    int currentHealCooldown;

public:
    Boss(string enemyName, int enemyHP, int enemyAttack, int enemyDefense, int enemySpeed, int enemyCoinDrop);

    EnemyAction chooseAction(const vector<Character*>& party) override;

    Character* chooseTarget(const vector<Character*>& party) override;

    void reduceCooldowns() override;
};

#endif

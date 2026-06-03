#ifndef MINION_H
#define MINION_H

#include "enemy.h"

class Minion : public Enemy {
public:
    Minion(string enemyName, int enemyHP, int enemyAttack, int enemyDefense, int enemySpeed, int enemyCoinDrop);

    EnemyAction chooseAction(const vector<Character*>& party) override;

    Character* chooseTarget(const vector<Character*>& party) override;
};

#endif // MINION_H

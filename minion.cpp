#include "minion.h"
#include "character.h"

Minion::Minion(string enemyName, int enemyHP, int enemyAttack, int enemyDefense, int enemySpeed, int enemyCoinDrop) : Enemy(enemyName, enemyHP, enemyAttack, enemyDefense, enemySpeed, enemyCoinDrop) {}

EnemyAction Minion::chooseAction(const vector<Character*>&) {
    return BASIC_ATTACK;
}

Character* Minion::chooseTarget(const vector<Character*>& party) {
    Character* target = nullptr;

    for (Character* member : party) {
        if (!member->isAlive()) continue;

        if (!target || member->getHP() < target->getHP()) {
            target = member;
        }
    }

    return target;
}
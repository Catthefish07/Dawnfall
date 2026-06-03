#include "minion.h"
#include "character.h"

Minion::Minion(string enemyName, int enemyHP, int enemyAttack, int enemyDefense, int enemySpeed, int enemyCoinDrop) : Enemy(enemyName, enemyHP, enemyAttack, enemyDefense, enemySpeed, enemyCoinDrop) {}

EnemyAction Minion::chooseAction(const vector<Character*>&) {
    return BASIC_ATTACK;
}

Character* Minion::chooseTarget(const vector<Character*>& party) {
    std::vector<Character*> alive;
    for (Character* c : party) {
        if (c->isAlive()){
            alive.push_back(c);
        }
    }
    if (alive.empty()){
        return nullptr;
    }

    if (rand() % 100 < 60) {
        Character* target = alive[0];
        for (Character* c : alive) {
            if (c->getHP() < target->getHP()){
                target = c;
            }
        }
        return target;
    }
    else {
        return alive[rand() % alive.size()];
    }
}

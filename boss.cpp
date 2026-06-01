#include "boss.h"
#include "character.h"

Boss::Boss(string enemyName, int enemyHP, int enemyAttack, int enemyDefense, int enemySpeed, int enemyCoinDrop) : Enemy(enemyName, enemyHP, enemyAttack, enemyDefense, enemySpeed, enemyCoinDrop), currentAoeCooldown(0), currentHealCooldown(0) {}

EnemyAction Boss::chooseAction(const vector<Character*>&) {
    if (HP <= maxHP * 0.30 && currentHealCooldown == 0) {
        currentHealCooldown = HEAL_COOLDOWN;
        return ENEMY_HEAL;
    }

    if (currentAoeCooldown == 0) {
        currentAoeCooldown = AOE_COOLDOWN;
        return AOE_ATTACK;
    }

    return BASIC_ATTACK;
}

Character* Boss::chooseTarget(const vector<Character*>& party) {
    Character* target = nullptr;

    for (Character* member : party) {
        if (!member->isAlive()) continue;

        if (!target || member->getAttack() > target->getAttack()) {
            target = member;
        }
    }

    return target;
}

void Boss::reduceCooldowns() {
    if (currentAoeCooldown > 0){
        currentAoeCooldown--;
    }

    if (currentHealCooldown > 0) {
        currentHealCooldown--;
    }
}
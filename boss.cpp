#include "boss.h"
#include "character.h"

Boss::Boss(std::string enemyName, int enemyHP, int enemyAttack, int enemyDefense, int enemySpeed, int enemyCoinDrop) : Enemy(enemyName, enemyHP, enemyAttack, enemyDefense, enemySpeed, enemyCoinDrop), currentAoeCooldown(0), currentHealCooldown(0) {}

EnemyAction Boss::chooseAction(const vector<Character*>&) {
    // if the boss' HP is below 30% it will self heal
    if (HP <= maxHP * 0.30 && currentHealCooldown == 0) {
        currentHealCooldown = HEAL_COOLDOWN;
        return ENEMY_HEAL;
    }

    // if the cooldown is off then use this smash AOE
    if (currentAoeCooldown == 0) {
        currentAoeCooldown = AOE_COOLDOWN;
        return AOE_ATTACK;
    }

    return BASIC_ATTACK;
}

// How the boss works here is that it would go for the highest ATK hero
Character* Boss::chooseTarget(const vector<Character*>& party) {
    std::vector<Character*> alive;
    for (Character* c : party) {
        if (c->isAlive()){
            alive.push_back(c);
        }
    }
    if (alive.empty())
        return nullptr;

    if (rand() % 100 < 70) {
        Character* target = alive[0];
        for (Character* c : alive) {
            if (c->getAttack() > target->getAttack()){
                target = c;
            }
        }
        return target;
    }
    else {
        return alive[rand() % alive.size()];
    }
}

// Thus function will be called at the start of the boss' turn to count its cooldowns down
void Boss::reduceCooldowns() {
    if (currentAoeCooldown > 0){
        currentAoeCooldown--;
    }

    if (currentHealCooldown > 0) {
        currentHealCooldown--;
    }
}

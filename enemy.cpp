#include "Enemy.h"

Enemy::Enemy(string enemyName, int enemyHP, int enemyAttack, int enemyDefense, int enemySpeed, int enemycoinDrop) : name(enemyName), HP(enemyHP), maxHP(enemyHP), attack(enemyAttack), defense(enemyDefense), speed(enemySpeed), coinDrop(enemycoinDrop) {}

void Enemy::takeDamage(int amount) {
    int actualDamage = amount - defense;
    if (actualDamage < 1) {
        actualDamage = 1;
    }

    HP -= actualDamage;

    if (HP < 0) {
        HP = 0;
    }
}

void Enemy::heal(int amount) {
    HP += amount;

    if (HP > maxHP)
        HP = maxHP;
}

bool Enemy::isDead() const {
    return HP <= 0;
}

bool Enemy::isAlive() const {
    return !isDead();
}

string Enemy::getName() const {
    return name;
}

int Enemy::getHP() const {
    return HP;
}

int Enemy::getMaxHP() const {
    return maxHP;
}

int Enemy::getAttack() const {
    return attack;
}

int Enemy::getDefense() const {
    return defense;
}

int Enemy::getSpeed() const {
    return speed;
}

int Enemy::getCoinDrop() const {
    return coinDrop;
}

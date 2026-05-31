#include "character.h"

Character::Character(std::string name, int maxHP, int attack, int defense, int speed) : name(name), HP(maxHP), maxHP(maxHP), attack(attack), defense(defense), speed(speed), level(1) {}

Character::~Character() {}

void Character::takeDamage(int amount) {
    int actualDamage = amount - defense;
    if (actualDamage < 1) {
        actualDamage = 1;
    }
    HP -= actualDamage;
    if (HP < 0) {
        HP = 0;
    }
}

void Character::heal(int amount) {
    HP += amount;
    if (HP > maxHP) {
        HP = maxHP;
    }
}

void Character::addDefense(int amount) {
    defense += amount;
}

bool Character::isAlive() const {
    return HP > 0;
}

void Character::levelUp() {
    level++;
    maxHP = maxHP * 1.15;
    attack = attack * 1.10;
    defense = defense * 1.08;
    HP = maxHP;
}

std::string Character::getName() const {
    return name;
}

int Character::getHP() const {
    return HP;
}

int Character::getMaxHP() const {
    return maxHP;
}

int Character::getAttack() const {
    return attack;
}

int Character::getDefense() const {
    return defense;
}

int Character::getSpeed() const {
    return speed;
}

int Character::getLevel() const {
    return level;
}

std::vector<Skill>& Character::getSkills() {
    return skills;
}

void Character::tickSkills() {
    for (Skill& s : skills) {
        s.tick();
    }
}
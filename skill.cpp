#include "skill.h"

Skill::Skill(std::string name, int damage, int cooldown)
    : name(name), baseDamage(damage), maxCooldown(cooldown), currentCooldown(0) {}

bool Skill::isReady() const {
    return currentCooldown == 0;
}

void Skill::use() {
    currentCooldown = maxCooldown;
}

void Skill::tick() {
    if (currentCooldown > 0) {
        currentCooldown--;
    }
}
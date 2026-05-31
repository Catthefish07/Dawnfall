#include "skill.h"

Skill::Skill(std::string name, SkillType type, int power, int cooldown) : name(name), type(type), power(power), maxCooldown(cooldown), currentCooldown(0) {}

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
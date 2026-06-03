#include "skill.h"

Skill::Skill(std::string name, SkillType type, int power, int cooldown) : name(name), type(type), power(power), maxCooldown(cooldown), currentCooldown(0) {}

std::string Skill::getName() const{
    return name;
}

SkillType Skill::getType() const
{
    return type;
}

int Skill::getPower() const
{
    return power;
}

int Skill::getMaxCooldown() const
{
    return maxCooldown;
}

int Skill::getCurrentCooldown() const
{
    return currentCooldown;
}

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

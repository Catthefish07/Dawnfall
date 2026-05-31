#ifndef SKILL_H
#define SKILL_H

#include <string>

class Skill {
public:
    std::string name;
    int baseDamage;
    int maxCooldown;
    int currentCooldown;

    Skill(std::string name, int baseDamage, int maxCooldown);
    bool isReady() const;
    void use();
    void tick();
};

#endif
#ifndef SKILL_H
#define SKILL_H

#include <string>

enum SkillType { DAMAGE, DEFEND, HEAL };

class Skill {
public:
    std::string name;
    SkillType type;
    int power;
    int maxCooldown;
    int currentCooldown;
    Skill(std::string name, SkillType type, int power, int maxCooldown);
    bool isReady() const;
    void use();
    void tick();
};

#endif
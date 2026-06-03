#ifndef SKILL_H
#define SKILL_H

#include <string>

enum SkillType { DAMAGE, DEFEND, HEAL };

class Skill {
private:
    std::string name;
    SkillType type;
    int power;
    int maxCooldown;
    int currentCooldown;

public:
    std::string getName() const;
    SkillType getType() const;
    int getPower() const;
    int getBasePower() const;
    int getMaxCooldown() const;
    int getCurrentCooldown() const;

    Skill(std::string name, SkillType type, int power, int maxCooldown);
    bool isReady() const;
    void use();
    void tick();
};

#endif

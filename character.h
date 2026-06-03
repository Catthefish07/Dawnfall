#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include "skill.h"

enum AttackType { MELEE, RANGE };

class Character {
protected:
    std::string name;
    std::vector<Skill> skills;
    int level;
    int HP;
    int maxHP;
    int attack;
    int defense;
    int speed;
    int defenseBuff = 0;
    int defenseBuffDuration = 0;

public:
    Character(std::string name, int maxHP, int attack, int defense, int speed);
    virtual ~Character() = default;

    void takeDamage(int amount);
    virtual int dealDamage() const = 0;
    virtual AttackType getAttackType() const = 0;
    void levelUp();

    void heal(int amount);
    bool isAlive() const;

    std::string getName() const;
    int getHP() const;
    int getMaxHP() const;
    int getAttack() const;
    int getDefense() const;
    int getSpeed() const;
    int getLevel() const;

    std::vector<Skill>& getSkills();
    virtual void tickSkills();
    void addTempDefense(int amount, int turns);

};

#endif

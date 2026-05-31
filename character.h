#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include "skill.h"

class Character
{
protected:
    std::string name;
    std::vector<Skill> skills;
    int level;
    int HP;
    int maxHP;
    int attack;
    int defense;
    int speed;


public:
    Character(std::string name, int maxHP, int attack, int defense, int speed);
    virtual ~Character();

    virtual void takeDamage(int amount);
    virtual int dealDamage() const = 0;
    virtual void levelUp();

    void heal(int amount);
    bool isAlive() const;

    std::string getName() const;
    int getHP() const;
    int getMaxHP() const;
    int getAttack() const;
    int getDefense() const;
    int getSpeed() const;
    int getLevel() const;

};

#endif

#include "warrior.h"
#include "skill.h"
#include <string>
#include <cstdlib>

Warrior::Warrior(std::string name, int maxHP, int attack, int defense, int speed) : MeleeAttack(name, maxHP, attack, defense, speed) {
    skills.push_back(Skill("Power Strike", DAMAGE, 25, 3));
}

int Warrior::dealDamage() const {
    return (int)(attack * 1.2);
}

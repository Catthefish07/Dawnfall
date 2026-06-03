#ifndef ENEMY_H
#define ENEMY_H

#include <string>
#include <vector>

using namespace std;

class Character;

enum EnemyAction {
    BASIC_ATTACK,
    AOE_ATTACK,
    ENEMY_HEAL
};

class Enemy {
protected:
    string name;
    int HP;
    int maxHP;
    int attack;
    int defense;
    int speed;
    int coinDrop;

public:
    Enemy(string enemyName, int enemyHP, int enemyAttack, int enemyDefense, int enemySpeed, int enemycoinDrop);

    virtual ~Enemy() {}

    virtual EnemyAction chooseAction(const vector<Character*>& party) = 0;

    virtual Character* chooseTarget(const vector<Character*>& party) = 0;

    void takeDamage(int amount);
    void heal(int amount);

    virtual void reduceCooldowns() {}

    bool isDead() const;
    bool isAlive() const;

    string getName() const;
    int getHP() const;
    int getMaxHP() const;
    int getAttack() const;
    int getDefense() const;
    int getSpeed() const;
    int getCoinDrop() const;
};

#endif

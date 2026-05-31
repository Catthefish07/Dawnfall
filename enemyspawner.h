#ifndef ENEMYSPAWNER_H
#define ENEMYSPAWNER_H

#include <string>
#include "enemy.h"

using namespace std;

class EnemySpawner {
public:
    static Enemy* createEnemy(const string& enemyName);
};

#endif // ENEMYSPAWNER_H
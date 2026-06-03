#ifndef ENEMYSPAWNER_H
#define ENEMYSPAWNER_H

#include <string>
#include "enemy.h"

using namespace std;

class EnemySpawner {
public:
    static Enemy* createEnemy(const string& enemyName);

    static vector<Enemy*> spawnLevel(int level);
};

#endif // ENEMYSPAWNER_H
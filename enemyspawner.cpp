#include "enemyspawner.h"
#include "minion.h"
#include "boss.h"

Enemy* EnemySpawner::createEnemy(const string& enemyName) {
    // As of right now, this only contains the enemy in the Forest Zone

    if (enemyName == "Slime") {
        return new Minion("Slime", 40, 8, 2, 5, 10);
    }
    if (enemyName == "Snake") {
        return new Minion("Snake", 60, 13, 5, 18, 18);
    }
    if (enemyName == "Wolf") {
        return new Minion("Wolf", 78, 16, 7, 14, 25);
    }
    if (enemyName == "Gorilla") {
        return new Boss("Gorilla", 250, 23, 10, 12, 100);
    }
    return nullptr;
}

// Returns the enemies that are present in the Forest level.
// Level 1: Slime  Level 2: Wolf + Snake  Level 3: Gorilla (boss)
vector<Enemy*> EnemySpawner::spawnLevel(int level) {
    vector<Enemy*> enemies;

    switch (level) {
    case 1:
        enemies.push_back(createEnemy("Slime"));
        break;

    case 2:
        enemies.push_back(createEnemy("Wolf"));
        enemies.push_back(createEnemy("Snake"));
        break;

    case 3:
        enemies.push_back(createEnemy("Gorilla"));
        enemies.push_back(createEnemy("Slime"));
        enemies.push_back(createEnemy("Wolf"));
        enemies.push_back(createEnemy("Snake"));
        break;
    }

    return enemies;
}
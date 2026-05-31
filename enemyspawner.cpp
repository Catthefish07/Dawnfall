#include "enemyspawner.h"
#include "minion.h"
#include "boss.h"

Enemy* EnemySpawner::createEnemy(const string& enemyName) {
    // As of right now, this only contains the enemy in the Forest Zone

    if (enemyName == "Slime") {
        return new Minion("Slime", 40, 8, 2, 5, 10);
    }

    if (enemyName == "Snake") {
        return new Minion("Snake", 50, 12, 5, 15, 20);
    }

    if (enemyName == "Wolf") {
        return new Minion("Wolf", 80, 18, 8, 20, 35);
    }

    if (enemyName == "Gorilla") {
        return new Boss("Gorilla", 250, 25, 12, 7, 100);
    }

    return nullptr;
}
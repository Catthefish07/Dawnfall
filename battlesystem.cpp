#include "battlesystem.h"
#include <sstream>
#include <iostream>

using namespace std;

namespace {
    // This is a small helper struct that would be useful for deciding the character's and enemy's turn (based on speed)
    struct Turn{
        Character * character;
        Enemy* enemy;
        int speed;
    };
}

static const double LOW_HP_THRESHOLD = 0.30;

BattleSystem::BattleSystem(const vector<Character*>& party, const vector<Enemy*>& enemies, Inventory& inventory, BattleMode mode) : party(party), enemies(enemies), inventory(inventory), mode(mode), result(ONGOING), currentTurn(1), autoplayEnabled(mode == FARMING_BATTLE) {}

BattleResult BattleSystem::startBattle() {
    while (result == ONGOING) {
        processTurn();
    }

    return result;
}

BattleResult BattleSystem::processTurn() {
    // Build this round's turn order, fastest first.
    vector<Turn> order;

    for (Character* c : party) {
        if (c->isAlive()) {
            order.push_back({ c, nullptr, c->getSpeed() });
        }
    }
    for (Enemy* e : enemies) {
        if (e->isAlive()) {
            order.push_back({ nullptr, e, e->getSpeed() });
        }
    }

    // Sort by speed descending. Ties keep their original order
    // (so party members, added first, act before enemies on a speed tie).
    for (size_t i = 1; i < order.size(); i++) {
        Turn current = order[i];
        size_t j = i;
        while (j > 0 && order[j - 1].speed < current.speed) {
            order[j] = order[j - 1];
            j--;
        }
        order[j] = current;
    }

    for (const Turn& turn : order) {
        if (turn.character) {
            Character* character = turn.character;
            if (!character->isAlive()) continue;   // may have died earlier this round

            playerTurn(character);

            if (result == FLED) {
                return result;
            }
            if (allEnemiesDead()) {
                result = VICTORY;
                distributeRewards();
                return result;
            }
        } else {
            Enemy* enemy = turn.enemy;
            if (!enemy->isAlive()) continue;       // may have died earlier this round

            enemyTurn(enemy);

            if (allCharactersDead()) {
                result = DEFEAT;
                return result;
            }
        }
    }

    currentTurn++;
    return result;
}

void BattleSystem::playerTurn(Character* character) {
    if (!character || !character->isAlive()) {
        return;
    }

    character->tickSkills();

    if (autoplayEnabled) {
        autoTurn(character);
    } else {
        manualTurn(character);
    }
}

void BattleSystem::autoTurn(Character* character) {
    Character* hurt = getLowestHpCharacter();
    int healIdx = findReadySkill(character, HEAL);

    if (hurt && healIdx != -1 &&
        hurt->getHP() < hurt->getMaxHP() * LOW_HP_THRESHOLD) {
        battleLog += characterUseSkill(character, healIdx, nullptr, hurt) + "\n";
        return;
    }

    Enemy* target = getWeakestEnemy();
    if (!target) return;

    int dmgIdx = findReadySkill(character, DAMAGE);
    if (dmgIdx != -1) {
        battleLog += characterUseSkill(character, dmgIdx, target, nullptr) + "\n";
    } else {
        battleLog += characterAttack(character, target) + "\n";
    }
}

int BattleSystem::findReadySkill(Character* character, SkillType type) {
    vector<Skill>& skills = character->getSkills();

    for (size_t i = 0; i < skills.size(); i++) {
        if (skills[i].type == type && skills[i].isReady()) {
            return (int)i;
        }
    }

    return -1;
}

// Input ManualTurn()
void BattleSystem::manualTurn(Character* character) {
    bool actionTaken = false;

    while (!actionTaken) {
        cout << "\n" << character->getName() << "'s turn:\n";
        cout << "1) Attack\n2) Skill\n3) Item\n4) Flee\n> ";

        int choice;
        cin >> choice;

        switch (choice) {

        case 1: {
            Enemy* target = chooseEnemyTarget();
            if (target) {
                string log = characterAttack(character, target);
                cout << log << "\n";
                battleLog += log + "\n";
                actionTaken = true;
            }
            break;
        }

        case 2:
            actionTaken = chooseAndUseSkill(character);
            break;

        case 3:
            cout << "Items aren't available yet.\n";
            break;

        case 4: {
            if (attemptFlee()) {
                cout << character->getName() << " fled the battle!\n";
                actionTaken = true;
            } else {
                cout << "You can't flee this battle!\n";
            }
            break;
        }

        default:
            cout << "Invalid choice.\n";
        }
    }
}

// Here is the autoplay function
void BattleSystem::setAutoplay(bool on) {
    if (mode == STORY_BATTLE) {
        autoplayEnabled = false;
        return;
    }

    autoplayEnabled = on;
}

bool BattleSystem::isAutoplayEnabled() const {
    return autoplayEnabled;
}


Enemy* BattleSystem::chooseEnemyTarget() {
    vector<Enemy*> alive;

    cout << "Choose a target:\n";
    for (Enemy* e : enemies) {
        if (e->isAlive()) {
            alive.push_back(e);
            cout << alive.size() << ") " << e->getName() << " (HP: " << e->getHP() << ")\n";
        }
    }

    if (alive.empty()) return nullptr;

    int pick;
    cin >> pick;

    if (pick < 1 || pick > (int)alive.size()) {
        cout << "Invalid target.\n";
        return nullptr;
    }

    return alive[pick - 1];
}

Character* BattleSystem::chooseAllyTarget() {
    vector<Character*> alive;

    cout << "Choose an ally:\n";
    for (Character* c : party) {
        if (c->isAlive()) {
            alive.push_back(c);
            cout << alive.size() << ") " << c->getName() << " (HP: " << c->getHP() << ")\n";
        }
    }

    if (alive.empty()) return nullptr;

    int pick;
    cin >> pick;

    if (pick < 1 || pick > (int)alive.size()) {
        cout << "Invalid ally.\n";
        return nullptr;
    }

    return alive[pick - 1];
}

bool BattleSystem::chooseAndUseSkill(Character* character) {
    vector<Skill>& skills = character->getSkills();

    if (skills.empty()) {
        cout << "No skills available.\n";
        return false;
    }

    cout << "Choose a skill:\n";
    for (size_t i = 0; i < skills.size(); i++) {
        cout << (i + 1) << ") " << skills[i].name;
        if (skills[i].isReady()) {
            cout << " (ready)\n";
        }
        else {
            cout << " (cooldown: " << skills[i].currentCooldown << ")\n";
        }
    }

    int pick;
    cin >> pick;

    if (pick < 1 || pick > (int)skills.size()) {
        cout << "Invalid skill.\n";
        return false;
    }

    Skill& skill = skills[pick - 1];

    if (!skill.isReady()) {
        cout << skill.name << " is still on cooldown!\n";
        return false;
    }

    Enemy* enemyTarget = nullptr;
    Character* allyTarget = nullptr;

    if (skill.type == DAMAGE) {
        enemyTarget = chooseEnemyTarget();
        if (!enemyTarget) return false;
    }
    else {
        allyTarget = chooseAllyTarget();
        if (!allyTarget) return false;
    }

    string log = characterUseSkill(character, pick - 1, enemyTarget, allyTarget);
    cout << log << "\n";
    battleLog += log + "\n";
    return true;
}

void BattleSystem::showEnemyStats() {
    cout << "Enemy stats:\n";
    for (Enemy* e : enemies) {
        if (e->isAlive()) {
            cout << "- " << e->getName() << " | HP: " << e->getHP() << "/" << e->getMaxHP() << " | ATK: " << e->getAttack() << " | DEF: " << e->getDefense() << "\n";
        }
    }
}

void BattleSystem::enemyTurn(Enemy* enemy) {
    if (!enemy || !enemy->isAlive()) {
        return;
    }

    enemy->reduceCooldowns();

    EnemyAction action = enemy->chooseAction(party);
    Character* target = enemy->chooseTarget(party);

    switch (action) {

    case BASIC_ATTACK:
    {
        if (target) {
            int damage = enemy->getAttack();
            target->takeDamage(damage);
            battleLog += enemy->getName() + " attacked " + target->getName() + "\n";
        }
        break;
    }

    case AOE_ATTACK:
    {
        for (Character* character : party) {
            if (character->isAlive()) {
                character->takeDamage(enemy->getAttack());
            }
        }
        battleLog += enemy->getName() + " used AOE Attack!\n";
        break;
    }

    case ENEMY_HEAL:
    {
        enemy->heal(50);
        battleLog += enemy->getName() + " healed itself!\n";
        break;
    }
    }
}

string BattleSystem::characterAttack(Character* character, Enemy* target) {
    if (!character || !target) {
        return "";
    }

    int damage = character->dealDamage();
    target->takeDamage(damage);

    ostringstream log;
    log << character->getName() << " attacked " << target->getName() << " for " << damage << " damage!";
    return log.str();
}

string BattleSystem::characterUseSkill(Character* user, int skillIndex, Enemy* enemyTarget, Character* allyTarget) {
    if (!user) {
        return "";
    }

    vector<Skill>& skills = user->getSkills();
    if (skillIndex < 0 || skillIndex >= (int)skills.size()) {
        return "Invalid skill.";
    }

    Skill& skill = skills[skillIndex];
    if (!skill.isReady()) {
        return skill.name + " is still on cooldown!";
    }

    string log;

    switch (skill.type) {

    case DAMAGE: {
        if (!enemyTarget) return "No enemy target.";
        int damage = user->dealDamage() + skill.power;
        enemyTarget->takeDamage(damage);
        log = user->getName() + " used " + skill.name + " on " + enemyTarget->getName() + " for " + to_string(damage) + " damage!";
        break;
    }

    case DEFEND: {
        if (!allyTarget) return "No ally target.";
        allyTarget->addDefense(skill.power);
        log = user->getName() + " used " + skill.name + " on " + allyTarget->getName() + ", raising defense by " + to_string(skill.power) + "!";
        break;
    }

    case HEAL: {
        if (!allyTarget) return "No ally target.";
        allyTarget->heal(skill.power);
        log = user->getName() + " used " + skill.name + " on " + allyTarget->getName() + ", healing " + to_string(skill.power) + " HP!";
        break;
    }
    }

    skill.use();
    return log;
}

string BattleSystem::characterUseItem(Character* character, int itemIndex) {
    // Stub: needs the Inventory item interface (list / effect / consume) wired in.
    if (!character) {
        return "";
    }
    return character->getName() + " used an item.";
}

bool BattleSystem::attemptFlee() {
    if (mode == STORY_BATTLE) {
        return false;
    }

    result = FLED;
    return true;
}

bool BattleSystem::allCharactersDead() const {
    for (Character* character : party) {
        if (character->isAlive()) {
            return false;
        }
    }
    return true;
}

bool BattleSystem::allEnemiesDead() const {
    for (Enemy* enemy : enemies) {
        if (enemy->isAlive()) {
            return false;
        }
    }
    return true;
}

Character* BattleSystem::getLowestHpCharacter() {
    Character* lowest = nullptr;

    for (Character* character : party) {
        if (character->isAlive()) {
            if (!lowest || character->getHP() < lowest->getHP()) {
                lowest = character;
            }
        }
    }

    return lowest;
}

Enemy* BattleSystem::getWeakestEnemy() {
    Enemy* weakest = nullptr;

    for (Enemy* enemy : enemies) {
        if (enemy->isAlive()) {
            if (!weakest || enemy->getHP() < weakest->getHP()) {
                weakest = enemy;
            }
        }
    }

    return weakest;
}

int BattleSystem::calculateCoinReward() const {
    int total = 0;

    for (Enemy* enemy : enemies) {
        total += enemy->getCoinDrop();
    }

    return total;
}

void BattleSystem::distributeRewards() {
    int coins = calculateCoinReward();
    inventory.addGold(coins);
    battleLog += "Victory! Earned " + to_string(coins) + " coins.\n";
}

string BattleSystem::getBattleLog() const {
    return battleLog;
}

int BattleSystem::getTurnNumber() const {
    return currentTurn;
}

BattleResult BattleSystem::getResult() const {
    return result;
}
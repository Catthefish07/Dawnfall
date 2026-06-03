#include "battlesystem.h"
#include <sstream>

using namespace std;

namespace {
    // This is a small helper struct that would be useful for deciding the character's and enemy's turn (based on speed)
    struct Turn{
        Character * character;
        Enemy* enemy;
        int speed;
    };
}

// if an ally's HP drops below 30% the autoplay tries to heal them
static const double LOW_HP_THRESHOLD = 0.30;

// Here is the main setup of Dawnfall - saves the party, enemy, inventory, and mode
// Here it also sets the autoplay to be ONLY enabled in farming mode not story mode
BattleSystem::BattleSystem(const vector<Character*>& party, const vector<Enemy*>& enemies, Inventory& inventory, BattleMode mode) : party(party), enemies(enemies), inventory(inventory), mode(mode), result(ONGOING), currentTurn(1), autoplayEnabled(mode == FARMING_BATTLE) {}

// This is the main loop which keeps on going until somebody wins, loses, or the character flees
BattleResult BattleSystem::startBattle() {
    const int MAX_TURNS = 50;

    while (result == ONGOING && currentTurn <= MAX_TURNS){
        processTurn();
    }

    while (result == ONGOING) {
        result = DEFEAT;
    }

    return result;
}

// Manages everyone's turn by speed: If you have a higher speed, you can have your turn first
BattleResult BattleSystem::processTurn() {
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
            // If the current character being pointed is dead, then skip their turn
            Character* character = turn.character;
            if (!character->isAlive()) {
                continue;
            }

            playerTurn(character);

            if (result == FLED) {
                return result;
            }

            if (allEnemiesDead()) {
                result = VICTORY;
                distributeRewards();
                return result;
            }
        }
        else {
            // If the current enemy being pointed is dead, then skip their turn
            Enemy* enemy = turn.enemy;
            if (!enemy->isAlive()) {
                continue;
            }

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

// Handles one party member's turn. if autoplay is on the computer plays for them, otherwise the player gets to pick their own action.
void BattleSystem::playerTurn(Character* character) {
    if (!character || !character->isAlive()) {
        return;
    }

    character->tickSkills();

    if (autoplayEnabled) {
        autoTurn(character);
    }
}

// This function is taking care of the enemy's turn (It picks its own action and target)
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

// This handles the AutoPlay system
// The logic behind the autoplay is just like this:
// 1) if a character is hurt (below 30% HP) and we have a heal ready, heal them
// 2) If not hit the weakest enemy with a damage skill if it's ready
// 3) if no skill is ready, just do a normal attack
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

// This looks through a character's skills and returns the index of the first one that matches the type AND is off cooldown. returns -1 if it can't find one.
int BattleSystem::findReadySkill(Character* character, SkillType type) {
    vector<Skill>& skills = character->getSkills();

    for (size_t i = 0; i < skills.size(); i++) {
        if (skills[i].getType() == type && skills[i].isReady()) {
            return (int)i;
        }
    }

    return -1;
}

// Here is the autoplay function, it turns on and off depending on the mode (If storymode then its off)
void BattleSystem::setAutoplay(bool on) {
    if (mode == STORY_BATTLE) {
        autoplayEnabled = false;
        return;
    }

    autoplayEnabled = on;
}

// tells if the autoplay is currently ON
bool BattleSystem::isAutoplayEnabled() const {
    return autoplayEnabled;
}

// void BattleSystem::showEnemyStats() {
//     cout << "Enemy stats:\n";
//     for (Enemy* e : enemies) {
//         if (e->isAlive()) {
//             cout << "- " << e->getName() << " | HP: " << e->getHP() << "/" << e->getMaxHP() << " | ATK: " << e->getAttack() << " | DEF: " << e->getDefense() << "\n";
//         }
//     }
// }

// Character does a basic attack to enemy and return a sentence describing it for the battle log
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

// First the function check is the skill is or isnt ready to use, then for whichever skill type you choose, the system also return a log string and puts the skill on cooldown
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
        return skill.getName() + " is still on cooldown!";
    }

    string log;

    switch (skill.getType()) {

    case DAMAGE: {
        if (!enemyTarget) return "No enemy target.";
        int damage = user->dealDamage() + skill.getPower();
        enemyTarget->takeDamage(damage);
        log = user->getName() + " used " + skill.getName() + " on " + enemyTarget->getName() + ". Deals " + to_string(damage) + " damage!";
        break;
    }

    case DEFEND: {
        if (!allyTarget){
            return "No ally target.";
        }
        int reduction = skill.getPower() + (user->getLevel() - 1) * 2;
        if (reduction > 50){
            reduction = 50;
        }

        allyTarget->addTempDefense(reduction, 3);
        log = user->getName() + " used " + skill.getName() + " on " + allyTarget->getName() + ". Damage reduced by " + to_string(reduction) + "% for 3 turns!";
        break;
    }

    case HEAL: {
        if (!allyTarget) return "No ally target.";
        int healAmount = (int)(allyTarget->getMaxHP() * skill.getPower() / 100.0);
        allyTarget->heal(healAmount);
        log = user->getName() + " used " + skill.getName() + " on " + allyTarget->getName() + ". " + to_string(healAmount) + " HP restored!";
        break;
    }
    }

    skill.use();
    return log;
}

string BattleSystem::characterUseItem(Character* character, int itemIndex) {
    if (!character) {
        return "";
    }

    vector<Item> items = inventory.getItems();
    if (itemIndex < 0 || itemIndex >= (int)items.size()) {
        return "Invalid item.";
    }

    Item item = items[itemIndex];
    if (!item.isOwned || item.quantity <= 0) {
        return item.name + " is not available.";
    }

    int hpBefore = character->getHP();
    inventory.useItem(item.name, character);            // friend's code, untouched
    int healed = character->getHP() - hpBefore;

    if (healed > 0) {
        return character->getName() + " used " + item.name + ". " + to_string(healed) + " HP restored!";
    }

    return character->getName() + " used " + item.name + ".";
}

// This function decided when user tries to run away from the fight. In a story battle, it sets to FALSE
// If fleeing is allowed, then it sets the battle result to FLED and returns TRUE
bool BattleSystem::attemptFlee() {
    if (mode == STORY_BATTLE) {
        return false;
    }

    result = FLED;
    return true;
}

// This returns true if every party member is dead (this means the user lost)
bool BattleSystem::allCharactersDead() const {
    for (Character* character : party) {
        if (character->isAlive()) {
            return false;
        }
    }
    return true;
}

// This returns true if every enemy is dead (this means the user won)
bool BattleSystem::allEnemiesDead() const {
    for (Enemy* enemy : enemies) {
        if (enemy->isAlive()) {
            return false;
        }
    }
    return true;
}

// This finds the alive party member with the lowest HP. autoplay uses this to decide who to heal.
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

// This finds the alive enemy with the lowest HP. autoplay attacks this one to finish enemies off faster.
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

// This adds up all the coins dropped by the enemies in this battle
int BattleSystem::calculateCoinReward() const {
    int total = 0;

    for (Enemy* enemy : enemies) {
        total += enemy->getCoinDrop();
    }

    return total;
}

// This gets called after a victory: gives the coins to the inventory and writes it in the log
void BattleSystem::distributeRewards() {
    int coins = calculateCoinReward();
    inventory.addCoins(coins);
    battleLog += "Victory! Earned " + to_string(coins) + " coins.\n";
}

// This returns the whole text log of everything that happened in the battle
string BattleSystem::getBattleLog() const {
    return battleLog;
}

// This returns which turn number we're currently on
int BattleSystem::getTurnNumber() const {
    return currentTurn;
}

// This returns the battle result (ONGOING / VICTORY / DEFEAT / FLED)
BattleResult BattleSystem::getResult() const {
    return result;
}

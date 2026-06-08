#include "mainmenuscreen.h"
#include "battlescreen.h"
#include "lobbyscreen.h"
#include "enemy.h"
#include "character.h"
#include "archer.h"
#include "tank.h"
#include "warrior.h"
#include "inventory.h"
#include "dialoguescreen.h"
#include "savemanager.h"
#include "partymanager.h"
#include "shop.h"

#include <QApplication>
#include <QStackedWidget>
#include <QWidget>
#include <QScreen>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDir>
#include <QDateTime>
#include <vector>
#include <functional>
#include <cstring>

using namespace std;

// ─────────────────────────────────────────────────────────────
// Forward declarations
// ─────────────────────────────────────────────────────────────

void startChapter1(QStackedWidget *screens, QString playerName, int W, int H,
                   PlayerRecord *playerRecord, Inventory *sharedInventory,
                   SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                   LobbyScreen *lobby);

void startChapter2(QStackedWidget *screens, QString playerName, int W, int H,
                   PlayerRecord *playerRecord, Inventory *sharedInventory,
                   SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                   LobbyScreen *lobby);

void startChapter2Campsite(QStackedWidget *screens, QString playerName, int W, int H,
                           PlayerRecord *playerRecord, Inventory *sharedInventory,
                           SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                           LobbyScreen *lobby);

void startChapter2ForestBattle(QStackedWidget *screens, QString playerName, int W, int H,
                               PlayerRecord *playerRecord, Inventory *sharedInventory,
                               SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                               LobbyScreen *lobby);

void startChapter3(QStackedWidget *screens, QString playerName, int W, int H,
                   PlayerRecord *playerRecord, Inventory *sharedInventory,
                   SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                   LobbyScreen *lobby);

void showChapterCompletedTransition(QStackedWidget *screens, int W, int H,
                                    QString chapterTitle, QString subtitleText,
                                    std::function<void()> onFinished);

LobbyScreen* createLobby(QStackedWidget *screens, QString playerName,
                         int W, int H,
                         PlayerRecord *playerRecord, Inventory *inventory,
                         SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                         vector<Character*> allCharacters);

void showSaveSlotOverlay(QStackedWidget *screens, int W, int H,
                         PlayerRecord *playerRecord, Inventory *inventory,
                         SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                         bool isSaving,
                         vector<Character*> allCharacters,
                         std::function<void()> onLoadSuccess = nullptr);
// ─────────────────────────────────────────────────────────────
// Small helpers
// ─────────────────────────────────────────────────────────────

void cleanupScreen(QStackedWidget *screens, QWidget *widget)
{
    if (!screens || !widget) return;
    if (widget == screens->widget(0)) return;
    screens->removeWidget(widget);
    widget->deleteLater();
}

DialogueScreen* showDialogue(QStackedWidget *screens,
                             const vector<DialogueLine>& lines,
                             int W, int H,
                             bool useEncounterAtEnd,
                             QString bgPath = ":/assets/background/forest_battle.png")
{
    DialogueScreen *dialogue = new DialogueScreen(lines, W, H, useEncounterAtEnd, bgPath);
    screens->addWidget(dialogue);
    screens->setCurrentWidget(dialogue);
    return dialogue;
}

Warrior* createMC(QString playerName)
{
    Warrior* mc = new Warrior(playerName.toStdString(), 120, 18, 12, 15);
    mc->setProfilePaths(
        ":/assets/profilepic/mc_pfp.PNG",
        ":/assets/profilepic/mc_pfp_dead.PNG",
        ":/assets/portraits/mc_neutral.PNG"
        );
    return mc;
}

Archer* createEthan()
{
    Archer* ethan = new Archer("Ethan", 100, 16, 8, 20);
    ethan->setProfilePaths(
        ":/assets/profilepic/ethan_pfp.PNG",
        ":/assets/profilepic/ethan_pfp_dead.PNG",
        ":/assets/portraits/ethan_neutral.png"
        );
    return ethan;
}

Tank* createHubert()
{
    Tank* hubert = new Tank("Hubert", 160, 14, 18, 8);
    hubert->setProfilePaths(
        ":/assets/profilepic/hubert_pfp.PNG",
        ":/assets/profilepic/hubert_pfp_dead.PNG",
        ":/assets/portraits/hubert_neutral.PNG"
        );
    return hubert;
}

void saveProgress(PlayerRecord *playerRecord, Inventory *inventory,
                  SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                  int chapter)
{
    playerRecord->coins = inventory->getCoins();
    playerRecord->currentChapter = chapter;
    saveManager->saveGame(0, *playerRecord, *inventory, *shop, *partyManager);
}

// ─────────────────────────────────────────────────────────────
// Dialogue data functions
// ─────────────────────────────────────────────────────────────

vector<DialogueLine> getChapter1IntroLines(QString playerName)
{
    string p = playerName.toStdString();
    return {
        {"Narrator", "",
         "In the kingdom of Dawn, there lies a peaceful town known as PAPOI Town surrounded by vast forests and rolling hills. Adventurers from all over the world once gathered here to seek fame, fortune, and adventure."},
        {"Narrator", "",
         "After years away from this place, one traveller finally returns."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Wow, it's been a long time since I last came here."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "I wonder how much it has changed."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Maybe I should look around before heading into town."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "??? What's that sound?"},
        {"Slime", ":/assets/portraits/slime_neutral.png",
         "*blerp*"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Woah, a slime? Looks like I have to fight."}
    };
}

vector<DialogueLine> getChapter1AfterBattleLines(QString playerName)
{
    string p = playerName.toStdString();
    return {
        {p, ":/assets/portraits/mc_neutral.PNG",
         "That should do it."},
        {"Narrator", "",
         "MC continues to travel and continue her journey. Suddenly, another slime jumps from behind."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Huh?"},
        {"???", "",
         "Hey, WATCH OUT...!"},
        {"Narrator", "",
         "An arrow shoots past the MC and hits the slime. The slime goes poof."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Are you okay?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Yeah, thanks for the help. My name is " + p + ". And you?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "I'm Ethan. Always watch your back in this area. New here?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "No. I've been in this place before."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Is that so? I assume you must have known the town pretty well then."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "It has already been a long time ago so it feels unfamiliar."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Is that so?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Hmmm..."},
        {"Narrator", "",
         "Ethan walks ahead. MC looks confused while watching him."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Well, come on! Follow me."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "You shouldn't be wandering around alone. Let's head back to town."}
    };
}

vector<DialogueLine> getChapter2TownLines(QString playerName)
{
    string p = playerName.toStdString();
    return {
        {"Narrator", "",
         "[Scene: PAPOI Town - City Hall Plaza]"},
        {"Narrator", "",
         "The gates open as townsfolk walk around the plaza."},
        {"Narrator", "",
         "Though years had passed, PAPOI remained lively as ever. Merchants filled the streets, blacksmiths hammered away at their work, and adventurers gathered in search of their next quest."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Look over there. There's an adventurer guild, and the shop next to it sells potions."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Oh! There's also the inn if you need a place to stay."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Looks like the town's doing pretty well."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "You arrived at the perfect time actually."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "There's a town meeting today."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Come on!!"},
        {"Narrator", "",
         "A crowd gathers in front of the city hall."},
        {"Crowd", "",
         "What's this about? I wonder if it's good news."},
        {"Crowd", "",
         "I hope it's not about taxes..."},
        {"Mayor", "",
         "HELLO CITIZENS OF PAPOI TOWN, I have a mission for you!"},
        {"Mayor", "",
         "Due to the rising number of monsters, I need brave warriors to slay their wicked leader deep in the Maple Forest."},
        {"Mayor", "",
         "The winner can snatch 500 coins! So be ready now, folks..."},
        {"Mayor", "",
         "Let the competition... begin!"},
        {"Mayor", "",
         "Good luck to all participants."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "500 gold coins..."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "That's a pretty good reward."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Ahhh... I really want it..."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "I can buy a lot of food and berries."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Hahaha, yeah I agree."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "You also can buy new weapons or clothes."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Hmmm... so...?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "What do you say? Want to team up?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "You mean become partners?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Exactly!"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "You fight well and I know the forest."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Together we're unstoppable!"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Sounds fun. Count me in then!"},
        {"Narrator", "",
         "Quest Accepted: Hunting Competition"},
        {"Narrator", "",
         "Objective: Defeat a wave of slimes."}
    };
}

vector<DialogueLine> getChapter2CampsiteLines(QString playerName)
{
    string p = playerName.toStdString();
    return {
        {"Narrator", "",
         "[Scene: Forest Campsite at Night]"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "We should find a place to rest before tomorrow."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Over there."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Looks like someone already set up camp."},
        {"Narrator", "",
         "MC and Ethan approach the campsite."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Wait..."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Is that person sleeping?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "In the MIDDLE of the Forest?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Seriously?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Hey. Wake up."},
        {"Narrator", "",
         "The stranger slowly opens one eye."},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Ugh, five more minutes..."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "GET UP!"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA!"},
        {"Narrator", "",
         "Hubert jumps up."},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Why are you yelling?!"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Don't you know it's dangerous right here in Maple Forest? There are monsters roaming around!"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Eh, who cares. I can take care of them with ease."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Who are you?"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Hubert. Knight at your service."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "How cool! What's a knight doing here? Shouldn't you be at a palace?"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Taking a summer break right now. I have all the time in the world."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Then I suppose you might be interested in joining our team?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "There's a competition coming up and it's worth 500 coins. We can split the rewards later!"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "I'm quite lazy, but sure why not. I need to take a stretch anyway."},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "What's the mission here?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Well, we are supposed to find the leader of this forest who is controlling all the other monsters."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Not really sure how they look like though."},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Right... I'll be the tank for our team here. Seems like you're missing one."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Awesome! The dream team is happening."},
        {"Narrator", "",
         "Hubert joins the party."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "We've got company, get ready to fight!"}
    };
}

vector<DialogueLine> getChapter2AfterForestBattleLines(QString playerName)
{
    string p = playerName.toStdString();
    return {
        {p, ":/assets/portraits/mc_neutral.PNG",
         "That's a lot of them! Are we sure we can take on their leader?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Positive! We're going to get that 500 coins!"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Who knows, probably a piece of cake-"},
        {"Narrator", "",
         "A crowd of other heroes starts running from the opposite side."},
        {"Person #1", "",
         "RUN! SAVE YOURSELVES!"},
        {"Person #2", "",
         "AHHHHHH!"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Well, it seems that we will have the fight earlier."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Let's gooo!"},
        {"Narrator", "",
         "The party marches forward amidst the chaos."}
    };
}

vector<DialogueLine> getChapter3PreBattleLines(QString playerName)
{
    string p = playerName.toStdString();
    return {
        {"Narrator", "",
         "As the party marches deeper into the forest, the forest becomes much more silent. The faint gush of wind accompanied their journey with the bright afternoon light."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "There's nothing here, I wonder what scared them earlier?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Probably a big powerful slime?"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "But one won't make you scream for life like that."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Hm... Maybe bigger wolves and snakes?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Whatever it is, I'm sure we can defeat it. We might need a plan."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         p + ", this is your time to shine!"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Well we should go on defense more in this occasion."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "And we mustn't make hasty moves, make sure to cover each other. We won't know how the enemy would attack us."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Great idea! I suppose we were a bit too aggressive before."},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "YOU were making hasty decisions, just give me a heads up before you decide to shoot an arrow right next to me."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Yeah, yeah. I'm sorry about that."},
        {"Narrator", "",
         "Suddenly the ground grumbles once more, shaking to its core with an accompanied loud roar."},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "I suppose we are getting nearer to the target."},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "I thought we were going to have a nap here first, zzzz."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Hubert, wake up! This is serious."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Look guys! There's a giant gorilla right in front of us!"},
        {"Gorilla", ":/assets/portraits/gorilla_neutral.png",
         "*huffs and puffs*"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Get ready for the fight guys!"},
        {"Gorilla", ":/assets/portraits/gorilla_neutral.png",
         "ROARRRRRRRRRRRRRRRRRRR!"}
    };
}

vector<DialogueLine> getChapter3PostBattleLines(QString playerName)
{
    string p = playerName.toStdString();
    return {
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "We did it guys, victory! I knew that this is the dream team!"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "*huff* You sure are energetic, thanks for the backup guys."},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "That gorilla sure is tough, time to take a nap again. I'm tired."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Hey not so fast Hubert. I think it's better to head back to town first and inform the mayor."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Otherwise, other parties will claim this first."}
    };
}

vector<DialogueLine> getChapter3Epilog(QString playerName)
{
    string p = playerName.toStdString();
    return {
        {"Narrator", "",
         "After the long and tiring battle, they went back to PAPOI Town as heroes. The mayor gave them their rewards, and all the citizens cheered for them all. Now the town is safe once and for all. Or is it?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Well at least now PAPOI Town is out of danger and I got to meet such nice friends. It's worth a trip back here after all."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "If it wasn't for you, I don't think I'll manage to do it alone. Now I can go shopping!"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Well, I can finally take a good rest now. Then I can polish back my shield."},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         p + ", what are you going to do after this? Will you stay in this town for a while?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "I think I will be here for a while, Ethan. I really want to try their famous barbecue ribs one last time before I go."},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Go so soon? What's the rush?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Well, I like to travel a lot, it's my sort of thing. A journey awaits me!"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Aw man, AT LEAST you have to try the pumpkin pies before you go. Come onnn, Hubert will treat you out!"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Who says, even I don't eat those anymore. Besides I think our captain " + p + " here has a lot on them no?"},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "I think I'll think about it, yes, are you guys up for dinner together?"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "I'm up!"},
        {"Hubert", ":/assets/portraits/hubert_neutral.PNG",
         "Fine, but let's go to Cave Tavern, the food there is the best around here."},
        {p, ":/assets/portraits/mc_neutral.PNG",
         "Sure!"},
        {"Ethan", ":/assets/portraits/ethan_neutral.png",
         "Sure!"}
    };
}

// ─────────────────────────────────────────────────────────────
// Chapter transition
// ─────────────────────────────────────────────────────────────

void showChapterCompletedTransition(QStackedWidget *screens, int W, int H,
                                    QString chapterTitle, QString subtitleText,
                                    std::function<void()> onFinished)
{
    QWidget *overlay = new QWidget(screens);
    overlay->setGeometry(0, 0, W, H);
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 150);");
    overlay->show();
    overlay->raise();

    QLabel *chapterText = new QLabel(chapterTitle, overlay);
    chapterText->setGeometry(0, H/2 - 80, W, 40);
    chapterText->setAlignment(Qt::AlignCenter);
    chapterText->setStyleSheet(
        "background:none; color:#8888aa; font-size:24px;"
        "font-family:'Courier New'; font-weight:bold; letter-spacing:5px;");

    QLabel *completedText = new QLabel("COMPLETED", overlay);
    completedText->setGeometry(0, H/2 - 25, W, 60);
    completedText->setAlignment(Qt::AlignCenter);
    completedText->setStyleSheet(
        "background:none; color:#ffdd44; font-size:48px;"
        "font-family:'Courier New'; font-weight:bold; letter-spacing:6px;");

    QLabel *subtitle = new QLabel(subtitleText, overlay);
    subtitle->setGeometry(0, H/2 + 45, W, 30);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(
        "background:none; color:white; font-size:18px; font-family:'Courier New';");

    chapterText->show();
    completedText->show();
    subtitle->show();

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(overlay);
    overlay->setGraphicsEffect(effect);
    effect->setOpacity(0.0);

    QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity", overlay);
    fadeIn->setDuration(500);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(2200, screens, [=]() {
        QPropertyAnimation *fadeOut = new QPropertyAnimation(effect, "opacity", overlay);
        fadeOut->setDuration(700);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        QObject::connect(fadeOut, &QPropertyAnimation::finished, [=]() {
            overlay->deleteLater();
            if (onFinished) onFinished();
        });
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

// ─────────────────────────────────────────────────────────────
// Chapters
// ─────────────────────────────────────────────────────────────

void startChapter1(QStackedWidget *screens, QString playerName, int W, int H,
                   PlayerRecord *playerRecord, Inventory *sharedInventory,
                   SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                   LobbyScreen *lobby)
{
    DialogueScreen *introDialogue = showDialogue(
        screens, getChapter1IntroLines(playerName),
        W, H, true, ":/assets/background/forest_battle.png");

    QObject::connect(introDialogue, &DialogueScreen::dialogueFinished, [=]() {
        vector<Character*> party;
        party.push_back(createMC(playerName));

        vector<vector<string>> waves = {{"Slime", "Slime", "Slime"}};

        BattleScreen *battle = new BattleScreen(
            party, waves, sharedInventory,
            STORY_BATTLE, W, H,
            ":/assets/background/forest_battle.png");

        screens->addWidget(battle);
        screens->setCurrentWidget(battle);
        cleanupScreen(screens, introDialogue);

        QObject::connect(battle, &BattleScreen::battleFinished, [=](bool victory) {
            if (!victory) {
                cleanupScreen(screens, battle);
                screens->setCurrentWidget(lobby);
                return;
            }

            saveProgress(playerRecord, sharedInventory, saveManager, shop, partyManager, 0);
            if (lobby) lobby->refreshStats();

            DialogueScreen *afterBattle = showDialogue(
                screens, getChapter1AfterBattleLines(playerName),
                W, H, false, ":/assets/background/forest_battle.png");
            cleanupScreen(screens, battle);

            QObject::connect(afterBattle, &DialogueScreen::dialogueFinished, [=]() {
                showChapterCompletedTransition(screens, W, H,
                                               "CHAPTER 01", "A New Encounter",
                                               [=]() {
                                                   playerRecord->currentChapter = 1;
                                                   saveProgress(playerRecord, sharedInventory, saveManager, shop, partyManager, 1);
                                                   cleanupScreen(screens, afterBattle);
                                                   if (lobby) {
                                                       lobby->refreshStats();
                                                       screens->setCurrentWidget(lobby);  // ← balik ke lobby
                                                   }
                                               });
            });
        });
    });
}

void startChapter2(QStackedWidget *screens, QString playerName, int W, int H,
                   PlayerRecord *playerRecord, Inventory *sharedInventory,
                   SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                   LobbyScreen *lobby)
{
    DialogueScreen *townDialogue = showDialogue(
        screens, getChapter2TownLines(playerName),
        W, H, true, ":/assets/background/cityhall.png");

    QObject::connect(townDialogue, &DialogueScreen::dialogueFinished, [=]() {
        vector<Character*> party;
        party.push_back(createMC(playerName));
        party.push_back(createEthan());

        vector<vector<string>> slimeWaves = {
            {"Slime", "Slime", "Slime"},
            {"Slime", "Slime", "Slime"}
        };

        BattleScreen *slimeBattle = new BattleScreen(
            party, slimeWaves, sharedInventory,
            STORY_BATTLE, W, H,
            ":/assets/background/forest_battle.png");

        screens->addWidget(slimeBattle);
        screens->setCurrentWidget(slimeBattle);
        cleanupScreen(screens, townDialogue);

        QObject::connect(slimeBattle, &BattleScreen::battleFinished, [=](bool victory) {
            if (!victory) {
                cleanupScreen(screens, slimeBattle);
                screens->setCurrentWidget(lobby);
                return;
            }

            saveProgress(playerRecord, sharedInventory, saveManager, shop, partyManager, 1);
            if (lobby) lobby->refreshStats();
            cleanupScreen(screens, slimeBattle);
            startChapter2Campsite(screens, playerName, W, H,
                                  playerRecord, sharedInventory,
                                  saveManager, shop, partyManager, lobby);
        });
    });
}

void startChapter2Campsite(QStackedWidget *screens, QString playerName, int W, int H,
                           PlayerRecord *playerRecord, Inventory *sharedInventory,
                           SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                           LobbyScreen *lobby)
{
    DialogueScreen *campsiteDialogue = showDialogue(
        screens, getChapter2CampsiteLines(playerName),
        W, H, true, ":/assets/background/campsite.png");

    QObject::connect(campsiteDialogue, &DialogueScreen::dialogueFinished, [=]() {
        cleanupScreen(screens, campsiteDialogue);
        startChapter2ForestBattle(screens, playerName, W, H,
                                  playerRecord, sharedInventory,
                                  saveManager, shop, partyManager, lobby);
    });
}

void startChapter2ForestBattle(QStackedWidget *screens, QString playerName, int W, int H,
                               PlayerRecord *playerRecord, Inventory *sharedInventory,
                               SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                               LobbyScreen *lobby)
{
    vector<Character*> fullParty;
    fullParty.push_back(createMC(playerName));
    fullParty.push_back(createEthan());
    fullParty.push_back(createHubert());

    vector<vector<string>> forestWaves = {{"Snake", "Wolf", "Snake"}};

    BattleScreen *forestBattle = new BattleScreen(
        fullParty, forestWaves, sharedInventory,
        STORY_BATTLE, W, H,
        ":/assets/background/campsite.png");

    screens->addWidget(forestBattle);
    screens->setCurrentWidget(forestBattle);

    QObject::connect(forestBattle, &BattleScreen::battleFinished, [=](bool victory) {
        if (!victory) {
            cleanupScreen(screens, forestBattle);
            screens->setCurrentWidget(lobby);
            return;
        }

        saveProgress(playerRecord, sharedInventory, saveManager, shop, partyManager, 1);
        if (lobby) lobby->refreshStats();

        DialogueScreen *afterForest = showDialogue(
            screens, getChapter2AfterForestBattleLines(playerName),
            W, H, false, ":/assets/background/campsite.png");
        cleanupScreen(screens, forestBattle);

        QObject::connect(afterForest, &DialogueScreen::dialogueFinished, [=]() {
            showChapterCompletedTransition(screens, W, H,
                                           "CHAPTER 02", "The Dream Team",
                                           [=]() {
                                               playerRecord->currentChapter = 2;
                                               saveProgress(playerRecord, sharedInventory, saveManager, shop, partyManager, 2);
                                               if (lobby) lobby->refreshStats();
                                               cleanupScreen(screens, afterForest);
                                               startChapter3(screens, playerName, W, H,
                                                             playerRecord, sharedInventory,
                                                             saveManager, shop, partyManager, lobby);
                                           });
        });
    });
}

void startChapter3(QStackedWidget *screens, QString playerName, int W, int H,
                   PlayerRecord *playerRecord, Inventory *sharedInventory,
                   SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                   LobbyScreen *lobby)
{
    DialogueScreen *preBattle = showDialogue(
        screens, getChapter3PreBattleLines(playerName),
        W, H, true, ":/assets/background/forest_night.png");

    QObject::connect(preBattle, &DialogueScreen::dialogueFinished, [=]() {
        cleanupScreen(screens, preBattle);

        vector<Character*> party;
        party.push_back(createMC(playerName));
        party.push_back(createEthan());
        party.push_back(createHubert());

        vector<vector<string>> waves = {{"Gorilla"}};

        BattleScreen *gorillaBattle = new BattleScreen(
            party, waves, sharedInventory,
            STORY_BATTLE, W, H,
            ":/assets/background/forest_night.png");

        screens->addWidget(gorillaBattle);
        screens->setCurrentWidget(gorillaBattle);

        QObject::connect(gorillaBattle, &BattleScreen::battleFinished, [=](bool victory) {
            if (!victory) {
                cleanupScreen(screens, gorillaBattle);
                screens->setCurrentWidget(lobby);
                return;
            }

            saveProgress(playerRecord, sharedInventory, saveManager, shop, partyManager, 2);
            if (lobby) lobby->refreshStats();

            DialogueScreen *postBattle = showDialogue(
                screens, getChapter3PostBattleLines(playerName),
                W, H, false, ":/assets/background/forest_night.png");
            cleanupScreen(screens, gorillaBattle);

            QObject::connect(postBattle, &DialogueScreen::dialogueFinished, [=]() {
                cleanupScreen(screens, postBattle);

                DialogueScreen *epilog = showDialogue(
                    screens, getChapter3Epilog(playerName),
                    W, H, false, ":/assets/background/papoitown.png");

                QObject::connect(epilog, &DialogueScreen::dialogueFinished, [=]() {
                    showChapterCompletedTransition(screens, W, H,
                                                   "CHAPTER 03", "The Dream Team",
                                                   [=]() {
                                                       playerRecord->currentChapter = 3;
                                                       saveProgress(playerRecord, sharedInventory, saveManager, shop, partyManager, 3);
                                                       if (lobby) lobby->refreshStats();
                                                       cleanupScreen(screens, epilog);
                                                       screens->setCurrentWidget(lobby);
                                                   });
                });
            });
        });
    });
}

// ─────────────────────────────────────────────────────────────
// Helper: create lobby and connect battleRequested
// ─────────────────────────────────────────────────────────────

LobbyScreen* createLobby(QStackedWidget *screens, QString playerName,
                         int W, int H,
                         PlayerRecord *playerRecord, Inventory *inventory,
                         SaveManager *saveManager, Shop *shop, PartyManager *partyManager,
                         vector<Character*> allCharacters)
{
    LobbyScreen *lobby = new LobbyScreen(
        *playerRecord, allCharacters, *shop, *inventory,
        playerRecord->coins, *saveManager, *partyManager, 0);

    screens->addWidget(lobby);
    screens->setCurrentWidget(lobby);

    // Lobby handles save internally via SaveSlotPopup — no need to connect saveRequested

    // Connect battle requested
    QObject::connect(lobby, &LobbyScreen::battleRequested,
                     [=](const QString &locationId) {
                         if (locationId == "maple_forest") {
                             if (playerRecord->currentChapter == 0) {
                                 startChapter1(screens, playerName, W, H,
                                               playerRecord, inventory,
                                               saveManager, shop, partyManager, lobby);
                             } else if (playerRecord->currentChapter == 1) {
                                 startChapter2(screens, playerName, W, H,
                                               playerRecord, inventory,
                                               saveManager, shop, partyManager, lobby);
                             } else if (playerRecord->currentChapter == 2) {
                                 startChapter3(screens, playerName, W, H,
                                               playerRecord, inventory,
                                               saveManager, shop, partyManager, lobby);
                             } else {
                                 // All chapters completed
                                 lobby->refreshStats();
                             }
                         }
                         // Tambah location lain nanti (dungeon, sun_castle, dll)
                     });

    return lobby;
}

// ─────────────────────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────────────────────

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ── Working directory fix (from teammate) — finds assets folder
    {
        QDir dir = QDir::current();
        for (int i = 0; i < 6; ++i) {
            if (dir.exists("assets")) {
                QDir::setCurrent(dir.absolutePath());
                break;
            }
            if (!dir.cdUp()) break;
        }
    }

    QApplication::setApplicationName("DawnFall");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("DawnFallStudio");

    // ── Global stylesheet (from teammate)
    a.setStyleSheet(R"(
        QWidget {
            font-family: 'Bahnschrift', 'Trebuchet MS', sans-serif;
            font-size: 13px;
            color: #e2e8f0;
        }
        QDialog     { background: #0f172a; }
        QMessageBox { background: #0f172a; }
        QMessageBox QLabel { color: #e2e8f0; }
        QMessageBox QPushButton {
            background: rgba(255,255,255,12);
            border: 1px solid rgba(255,255,255,25);
            border-radius: 6px; color: #e2e8f0;
            padding: 5px 18px; min-width: 70px;
        }
        QMessageBox QPushButton:hover { background: rgba(255,255,255,22); }
        QScrollBar:vertical {
            background: rgba(255,255,255,10);
            width: 6px; border-radius: 3px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(255,255,255,40);
            border-radius: 3px; min-height: 20px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical { height: 0; }
    )");

    QScreen *screen = QApplication::primaryScreen();
    QRect geo = screen->geometry();
    int W = geo.width();
    int H = geo.height();

    QStackedWidget *screens = new QStackedWidget();
    screens->setWindowTitle("DawnFall");
    screens->showFullScreen();

    // ── Shared game objects
    Shop         *shop         = new Shop();
    Inventory    *inventory    = new Inventory();
    SaveManager  *saveManager  = new SaveManager();
    PartyManager *partyManager = new PartyManager();

    // ── PlayerRecord — fresh start, player loads via Load Game menu
    PlayerRecord *playerRecord = new PlayerRecord();
    memset(playerRecord, 0, sizeof(PlayerRecord));
    playerRecord->occupied       = true;
    playerRecord->currentChapter = 0;
    playerRecord->coins          = 0;
    int uidNum = (int)(QDateTime::currentMSecsSinceEpoch() % 10000);
    snprintf(playerRecord->UID, sizeof(playerRecord->UID), "DF-%04d", uidNum);

    // All characters for lobby/shop/party
    vector<Character*> allCharacters;
    allCharacters.push_back(new Warrior("MC",     120, 18, 12, 15));
    allCharacters.push_back(new Archer("Ethan",   100, 16,  8, 20));
    allCharacters.push_back(new Tank("Hubert",    160, 14, 18,  8));

    // ── Main menu
    MainMenuScreen *menu = new MainMenuScreen(
        *saveManager, *inventory, *shop, *partyManager);
    screens->addWidget(menu);
    screens->setCurrentIndex(0);

    // ── New Game
    QObject::connect(menu, &MainMenuScreen::newGameClicked, [=]() {
        // Name input overlay
        QWidget *nameOverlay = new QWidget(screens);
        nameOverlay->setGeometry(0, 0, W, H);
        nameOverlay->setStyleSheet("background: rgba(0,0,0,180);");

        QLabel *panel = new QLabel(nameOverlay);
        panel->setGeometry(W/2 - 200, H/2 - 100, 400, 200);
        panel->setStyleSheet(
            "background:#0d0d1a; border:2px solid #ffdd44; border-radius:10px;");

        QLabel *title = new QLabel("Enter your name", nameOverlay);
        title->setGeometry(W/2 - 180, H/2 - 80, 360, 30);
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet(
            "color:#ffdd44; font-size:18px; font-weight:bold; font-family:'Courier New';");

        QLineEdit *nameInput = new QLineEdit(nameOverlay);
        nameInput->setGeometry(W/2 - 150, H/2 - 30, 300, 40);
        nameInput->setPlaceholderText("MC");
        nameInput->setMaxLength(20);
        nameInput->setStyleSheet(
            "background:#1a1a2e; color:white; border:2px solid #4a4a8a;"
            "border-radius:6px; font-size:16px; font-family:'Courier New'; padding:4px;");

        QPushButton *btnConfirm = new QPushButton("CONFIRM", nameOverlay);
        btnConfirm->setGeometry(W/2 - 80, H/2 + 30, 160, 40);
        btnConfirm->setStyleSheet(
            "QPushButton{background:#1a1a2e; color:#ffdd44; border:2px solid #ffdd44;"
            "border-radius:6px; font-size:14px; font-family:'Courier New'; font-weight:bold;}"
            "QPushButton:hover{background:#ffdd44; color:#0d0d1a;}");

        nameOverlay->show();
        nameOverlay->raise();
        nameInput->setFocus();

        QObject::connect(btnConfirm, &QPushButton::clicked, [=]() {
            QString playerName = nameInput->text().trimmed();
            if (playerName.isEmpty()) playerName = "MC";

            nameOverlay->hide();
            nameOverlay->deleteLater();

            // Reset for new game
            strncpy(playerRecord->playerUsername,
                    playerName.toUtf8().constData(),
                    sizeof(playerRecord->playerUsername) - 1);
            playerRecord->currentChapter = 0;
            playerRecord->coins          = 0;

            inventory->setQuantity("Health Potion", 3);
            inventory->setQuantity("Mega Potion",   1);
            inventory->setQuantity("Revive Stone",  1);
            inventory->setCoins(0);

            createLobby(screens, playerName, W, H,
                        playerRecord, inventory,
                        saveManager, shop, partyManager, allCharacters);
        });

        QObject::connect(nameInput, &QLineEdit::returnPressed,
                         btnConfirm, &QPushButton::click);
    });

    // ── Load Game — handled by LoadSlotPopup inside MainMenuScreen
    QObject::connect(menu, &MainMenuScreen::loadSlotChosen,
                     [=](int slot, PlayerRecord record) {
                         Q_UNUSED(slot)
                         *playerRecord = record;
                         QString playerName = QString(playerRecord->playerUsername);
                         if (playerName.isEmpty()) playerName = "MC";
                         createLobby(screens, playerName, W, H,
                                     playerRecord, inventory,
                                     saveManager, shop, partyManager, allCharacters);
                     });

    // ── Exit
    QObject::connect(menu, &MainMenuScreen::exitClicked, []() {
        QApplication::quit();
    });

    return a.exec();
}

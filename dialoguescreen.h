#ifndef DIALOGUESCREEN_H
#define DIALOGUESCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <vector>
#include <string>
using namespace std;

struct DialogueLine {
    string characterName;
    string portraitPath;  // path to portrait asset (or empty for placeholder)
    string text;
};

// DialogueScreen — visual novel style story screen
// Member 4 — Frontend 2: Battle UI, Animations & Dialogue
class DialogueScreen : public QWidget {
    Q_OBJECT
public:
    DialogueScreen(vector<DialogueLine> lines,
                   int W,
                   int H,
                   bool showEncounterAtEnd,
                   QString backgroundPath = ":/assets/background/forest_battle.png",
                   QWidget *parent = nullptr);
private:
    int W = 800;
    int H = 600;

signals:
    void dialogueFinished();

private:
    vector<DialogueLine> lines;
    int                  currentLine;
    QLabel              *bgLabel;
    QLabel              *portrait;
    QLabel              *characterName;
    QLabel              *dialogueText;
    QLabel              *continueHint;
    QPushButton         *btnNext;
    QTimer              *typewriterTimer;
    QString              fullText;
    int                  charIndex;

    void setupUI();
    void setupConnections();
    void showLine(int index);
    void typewriterTick();   // called by QTimer to reveal text letter by letter
    void showEnemyEncounterTransition();
    bool showEncounterAtEnd;
    QString backgroundPath;
};

#endif // DIALOGUESCREEN_H

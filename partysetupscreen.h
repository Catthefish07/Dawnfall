#ifndef PARTYSETUPSCREEN_H
#define PARTYSETUPSCREEN_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QVector>
#include <QString>

#include "character.h"
#include "partymanager.h"
#include "lobbyscreen.h"   // loadAsset + PixmapButton

// ── Character display info ────────────────────────────────────────────────────
struct CharDisplayInfo {
    QString    name;
    QString    portraitPath;   // assets/portraits/xxx_neutral
    QString    sideartPath;    // assets/sideart/xxx_pixelSide
    Character *charPtr;
    bool       unlocked;
};

// ── PartySlot — one of the 3 active party slots (top row) ────────────────────
class PartySlot : public QWidget
{
    Q_OBJECT
public:
    explicit PartySlot(int slotIndex, QWidget *parent = nullptr);
    void setCharacter(const CharDisplayInfo &info);
    void clearCharacter();
    bool       isEmpty()    const { return m_isEmpty; }
    int        slotIndex()  const { return m_slotIndex; }
    Character *character()  const { return m_charPtr; }

signals:
    void slotClicked(int slotIndex);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void paintEvent     (QPaintEvent *) override;

private:
    int        m_slotIndex;
    bool       m_isEmpty  = true;
    Character *m_charPtr  = nullptr;
    QLabel    *m_bgLbl    = nullptr;
    QLabel    *m_artLbl   = nullptr;
    QLabel    *m_nameLbl  = nullptr;
    QLabel    *m_emptyLbl = nullptr;
};

// ── RosterCard — portrait card in the bottom roster (uses pixel_character) ───
class RosterCard : public QWidget
{
    Q_OBJECT
public:
    explicit RosterCard(const CharDisplayInfo &info, QWidget *parent = nullptr);
    void setHighlighted(bool on);
    const CharDisplayInfo &info() const { return m_info; }

signals:
    void cardClicked(const CharDisplayInfo &info);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void paintEvent     (QPaintEvent *) override;
    void enterEvent     (QEnterEvent *) override;
    void leaveEvent     (QEvent      *) override;

private:
    CharDisplayInfo m_info;
    QLabel *m_portraitLbl = nullptr;
    QLabel *m_nameLbl     = nullptr;
    bool    m_highlighted = false;
    bool    m_hovered     = false;
};

// ── PartySetupScreen ──────────────────────────────────────────────────────────
class PartySetupScreen : public QDialog
{
    Q_OBJECT
public:
    explicit PartySetupScreen(vector<Character*> allCharacters,
                              PartyManager      &partyManager,
                              QWidget           *parent = nullptr);

signals:
    void partyConfirmed(vector<Character*> party);

private slots:
    void onSlotClicked        (int slotIndex);
    void onRosterCardClicked  (const CharDisplayInfo &info);
    void onQuickSetup();
    void onReplace();
    void onDeploy();

private:
    void buildCharList();
    void buildUi();
    void buildPartySlots();
    void buildStatsPanel();
    void buildRoster    (QVBoxLayout *root);
    void buildBottomBar (QVBoxLayout *root);
    void updateStatsPanel(const CharDisplayInfo &info);
    void refreshSlots();
    void applyStyle();

    vector<Character*>       m_allCharacters;
    PartyManager            &m_partyManager;
    QVector<CharDisplayInfo>  m_charList;

    int m_selectedSlot   = -1;
    int m_selectedRoster = -1;

    QVector<PartySlot*>  m_slots;
    QVector<RosterCard*> m_rosterCards;

    // Stats panel widgets
    QWidget *m_statsPanel    = nullptr;
    QLabel  *m_statsPortrait = nullptr;
    QLabel  *m_statsName     = nullptr;
    QLabel  *m_statsHp       = nullptr;
    QLabel  *m_statsAtk      = nullptr;
    QLabel  *m_statsDef      = nullptr;
    QLabel  *m_statsSpd      = nullptr;

    // Bottom buttons
    PixmapButton *m_quickSetupBtn = nullptr;
    PixmapButton *m_replaceBtn    = nullptr;
    PixmapButton *m_deployBtn     = nullptr;
};

#endif // PARTYSETUPSCREEN_H
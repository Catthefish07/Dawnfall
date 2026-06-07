#ifndef QUESTSCREEN_H
#define QUESTSCREEN_H

// ─────────────────────────────────────────────────────────────────────────────
// QuestScreen – quest log dialog, opened from the lobby's quest icon.
//
// ANSWER TO YOUR QUESTION:
//   The original stub header (with QListWidget / setupUI / setupConnections)
//   is NOT used — it was a placeholder left by a teammate. We replace it
//   entirely with this proper implementation. The old class is gone; keep
//   only this file.
//
// QuestRow and QuestScreen are defined here because they only exist to
// support the quest log UI. No other screen needs them.
// ─────────────────────────────────────────────────────────────────────────────

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QVector>
#include <QString>

// Forward-declare QLabel so the compiler knows it is a class, not a value.
// Full include is in the .cpp where it is actually used.
class QLabel;

// ─────────────────────────────────────────────────────────────────────────────
// Quest status
// ─────────────────────────────────────────────────────────────────────────────
enum class QuestStatus { Locked, Active, Done };

// ─────────────────────────────────────────────────────────────────────────────
// Data for one quest
// ─────────────────────────────────────────────────────────────────────────────
struct QuestEntry {
    QString     id;
    QString     locationId;
    QString     title;
    QString     detail;        // one-line story/battle hint
    QuestStatus status;
    int         rewardCoins;
    int         rewardExp;
};

// ─────────────────────────────────────────────────────────────────────────────
// QuestRow  – one row in the quest list
// ─────────────────────────────────────────────────────────────────────────────
class QuestRow : public QWidget
{
    Q_OBJECT
public:
    explicit QuestRow(const QuestEntry &entry, QWidget *parent = nullptr);
    void setStatus(QuestStatus s);

signals:
    void claimClicked(const QString &id);

private:
    void applyStyle();

    QuestEntry   m_entry;
    QLabel      *m_badge  = nullptr;   // ✓ / ! / 🔒  status circle
    QLabel      *m_title  = nullptr;
    QLabel      *m_detail = nullptr;
    QLabel      *m_reward = nullptr;
    QPushButton *m_claim  = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────
// QuestScreen  – the full quest log dialog
// ─────────────────────────────────────────────────────────────────────────────
class QuestScreen : public QDialog
{
    Q_OBJECT
public:
    // playerChapter: m_record.chapter from LobbyScreen.
    // Quests whose location requiredChapter > playerChapter are Locked.
    explicit QuestScreen(int playerChapter, QWidget *parent = nullptr);

    void setPlayerChapter(int ch);
    void refresh();

signals:
    void questClosed();

private slots:
    void onClaimClicked(const QString &id);

private:
    void buildQuests();
    void buildUi();
    void populateList();
    void applyStyle();

    int                 m_playerChapter = 0;
    QVector<QuestEntry> m_quests;

    QScrollArea *m_scroll      = nullptr;
    QWidget     *m_listWidget  = nullptr;
    QVBoxLayout *m_listLayout  = nullptr;
    QLabel      *m_progressLbl = nullptr;
};

#endif // QUESTSCREEN_H
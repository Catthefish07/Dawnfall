#ifndef MAINMENUSCREEN_H
#define MAINMENUSCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QPixmap>

// ── Custom image-based button ─────────────────────────────────────────────────
class ImageButton : public QPushButton {
    Q_OBJECT
public:
    explicit ImageButton(const QString &normalImg,
                         const QString &hoverImg,
                         QWidget *parent = nullptr);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent      *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_normal;
    QPixmap m_hover;
    bool    m_hovered = false;
};

// ── Main menu screen ──────────────────────────────────────────────────────────
class MainMenuScreen : public QWidget {
    Q_OBJECT
public:
    explicit MainMenuScreen(QWidget *parent = nullptr);

signals:
    void newGameClicked();
    void loadGameClicked();
    void exitClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;  // handles fullscreen sizing

private slots:
    void updateBackground();

private:
    QLabel      *bgLabel     = nullptr;
    QPixmap      m_dayPixmap;
    QPixmap      m_nightPixmap;
    QTimer      *m_bgTimer   = nullptr;
    int          m_W         = 0;
    int          m_H         = 0;
    QString      m_assetRoot;

    // Phase 1 (START / EXIT)
    ImageButton *btnStart    = nullptr;
    ImageButton *btnExit     = nullptr;

    // Phase 2 (NEW / LOAD)
    ImageButton *btnNew      = nullptr;
    ImageButton *btnLoad     = nullptr;

    void setupConnections();
    void showPhase1();
    void showPhase2();
    bool hasSaveFile()  const;
    bool isNightTime()  const;
};

#endif
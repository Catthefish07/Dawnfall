#include "dialoguescreen.h"
#include <QTimer>
#include <QShortcut>

DialogueScreen::DialogueScreen(vector<DialogueLine> lines,
                               int W,
                               int H,
                               bool showEncounterAtEnd,
                               QString backgroundPath,
                               QWidget *parent)
    : QWidget(parent),
    lines(lines),
    currentLine(0),
    charIndex(0),
    W(W),
    H(H),
    showEncounterAtEnd(showEncounterAtEnd),
    backgroundPath(backgroundPath)
{
    setupUI();
    setupConnections();

    if (!lines.empty()) showLine(0);

    // QShortcut — ga butuh focus, works selama widget visible
    QShortcut *spaceShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(spaceShortcut, &QShortcut::activated, this, [this]() {
        if (btnNext && btnNext->isEnabled()) btnNext->click();
    });

    QShortcut *enterShortcut = new QShortcut(QKeySequence(Qt::Key_Return), this);
    connect(enterShortcut, &QShortcut::activated, this, [this]() {
        if (btnNext && btnNext->isEnabled()) btnNext->click();
    });
}


void DialogueScreen::setupUI() {
    setFixedSize(W, H);
    setStyleSheet("background-color: #0d0d1a;");

    float sx = W / 800.0f;
    float sy = H / 600.0f;
    auto x  = [&](int v){ return (int)(v * sx); };
    auto y  = [&](int v){ return (int)(v * sy); };
    auto fs = [&](int v){ return QString("font-size:%1px;").arg((int)(v * sx)); };

    // BACKGROUND
    bgLabel = new QLabel(this);
    bgLabel->setGeometry(0, 0, W, y(430));
    QPixmap bg(backgroundPath);
    if (!bg.isNull())
        bgLabel->setPixmap(bg.scaled(W, y(410), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    else
        bgLabel->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
                               "stop:0 #0a0a1a, stop:1 #1a1a3a);");

    // DIALOGUE BOX
    QLabel *dlgBox = new QLabel(this);
    dlgBox->setGeometry(0, y(420), W, y(200));
    dlgBox->setStyleSheet("background:#0d0d1a; border-top:3px solid #4a4a8a;");

    // PORTRAIT — besar di atas dialogue box, sebelah kiri
    portrait = new QLabel(this);
    portrait->setGeometry(x(-40), y(-160), x(360), y(580));
    portrait->setStyleSheet("background:none; border:none;");
    portrait->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    // CHARACTER NAME
    characterName = new QLabel("???", this);
    characterName->setGeometry(x(155), y(440), x(300), y(24));
    characterName->setStyleSheet("color:#ffdd44;" + fs(14) + "font-weight:bold; font-family:'Courier New';");

    // DIALOGUE TEXT
    dialogueText = new QLabel("", this);
    dialogueText->setGeometry(x(155), y(470), x(600), y(100));
    dialogueText->setStyleSheet("color:white;" + fs(13) + "font-family:'Courier New';");
    dialogueText->setWordWrap(true);
    dialogueText->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // NEXT BUTTON
    btnNext = new QPushButton("▶", this);
    btnNext->setGeometry(W/2, y(540), x(40), y(30));
    btnNext->setStyleSheet(
        "QPushButton{background:transparent; color:#ffdd44;" + fs(18) + "border:none;}"
                                                                        "QPushButton:hover{color:white;}");

    typewriterTimer = new QTimer(this);
    typewriterTimer->setInterval(30);

    continueHint = new QLabel("Press SPACE to continue", this);
    continueHint->setGeometry(W/2 - x(100), y(570), x(240), y(20));
    continueHint->setAlignment(Qt::AlignCenter);
    continueHint->setStyleSheet(
        "background:none;"
        "color:rgba(255,255,255,150);"
        + fs(8) +
        "font-family:'Courier New';"
        );
}

void DialogueScreen::setupConnections() {
    connect(btnNext, &QPushButton::clicked, this, [=]() {
        if (typewriterTimer->isActive()) {
            typewriterTimer->stop();
            dialogueText->setText(fullText);
            charIndex = fullText.length();
        } else {
            currentLine++;

            if (currentLine >= (int)lines.size()) {
                if (showEncounterAtEnd) {
                    showEnemyEncounterTransition();
                } else {
                    emit dialogueFinished();
                }
            } else {
                showLine(currentLine);
            }
        }
    });

    connect(typewriterTimer, &QTimer::timeout, this, &DialogueScreen::typewriterTick);
}

void DialogueScreen::showLine(int index) {
    if (index < 0 || index >= (int)lines.size()) return;
    const DialogueLine& line = lines[index];

    characterName->setText(QString::fromStdString(line.characterName));
    portrait->setText(QString::fromStdString(line.characterName.substr(0,2)));

    // Load portrait if asset exists
    if (!line.portraitPath.empty()) {
        QPixmap px(QString::fromStdString(line.portraitPath));
        if (!px.isNull())
            portrait->setPixmap(px.scaled(portrait->width(), portrait->height(),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
    } else {
        portrait->clear();
    }

    // Start typewriter
    fullText   = QString::fromStdString(line.text);
    charIndex  = 0;
    dialogueText->setText("");
    typewriterTimer->start();
}

void DialogueScreen::typewriterTick() {
    if (charIndex < fullText.length()) {
        dialogueText->setText(fullText.left(++charIndex));
    } else {
        typewriterTimer->stop();
    }
}

void DialogueScreen::showEnemyEncounterTransition()
{
    btnNext->setEnabled(false);

    QLabel *overlay = new QLabel(this);
    overlay->setGeometry(0, 0, W, H);
    overlay->setStyleSheet("background: rgba(0, 0, 0, 170);");
    overlay->show();
    overlay->raise();

    QLabel *banner = new QLabel(this);
    banner->setGeometry(0, H / 2 - 75, W, 150);
    banner->setStyleSheet(
        "background: qlineargradient("
        "x1:0, y1:0, x2:1, y2:0,"
        "stop:0 rgba(8, 20, 45, 220),"
        "stop:0.5 rgba(80, 170, 220, 235),"
        "stop:1 rgba(8, 20, 45, 220)"
        ");"
        "border-top: 3px solid #9adfff;"
        "border-bottom: 3px solid #9adfff;"
        );
    banner->show();
    banner->raise();

    QLabel *topLine = new QLabel(this);
    topLine->setGeometry(0, H / 2 - 66, W, 2);
    topLine->setStyleSheet("background: rgba(255, 230, 160, 150);");
    topLine->show();
    topLine->raise();

    QLabel *bottomLine = new QLabel(this);
    bottomLine->setGeometry(0, H / 2 + 66, W, 2);
    bottomLine->setStyleSheet("background: rgba(255, 230, 160, 150);");
    bottomLine->show();
    bottomLine->raise();

    QLabel *title = new QLabel("Enemy encountered!", this);
    title->setGeometry(0, H / 2 - 22, W, 44);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "background:none;"
        "color:#e8f8ff;"
        "font-size:30px;"
        "font-family:'Courier New';"
        "font-weight:bold;"
        "letter-spacing:2px;"
        );
    title->show();
    title->raise();

    QGraphicsOpacityEffect *overlayEffect = new QGraphicsOpacityEffect(overlay);
    QGraphicsOpacityEffect *bannerEffect = new QGraphicsOpacityEffect(banner);
    QGraphicsOpacityEffect *topEffect = new QGraphicsOpacityEffect(topLine);
    QGraphicsOpacityEffect *bottomEffect = new QGraphicsOpacityEffect(bottomLine);
    QGraphicsOpacityEffect *titleEffect = new QGraphicsOpacityEffect(title);

    overlay->setGraphicsEffect(overlayEffect);
    banner->setGraphicsEffect(bannerEffect);
    topLine->setGraphicsEffect(topEffect);
    bottomLine->setGraphicsEffect(bottomEffect);
    title->setGraphicsEffect(titleEffect);

    overlayEffect->setOpacity(0.0);
    bannerEffect->setOpacity(0.0);
    topEffect->setOpacity(0.0);
    bottomEffect->setOpacity(0.0);
    titleEffect->setOpacity(0.0);

    QPropertyAnimation *fadeOverlayIn = new QPropertyAnimation(overlayEffect, "opacity", this);
    fadeOverlayIn->setDuration(350);
    fadeOverlayIn->setStartValue(0.0);
    fadeOverlayIn->setEndValue(1.0);

    QPropertyAnimation *fadeBannerIn = new QPropertyAnimation(bannerEffect, "opacity", this);
    fadeBannerIn->setDuration(350);
    fadeBannerIn->setStartValue(0.0);
    fadeBannerIn->setEndValue(1.0);

    QPropertyAnimation *fadeTopIn = new QPropertyAnimation(topEffect, "opacity", this);
    fadeTopIn->setDuration(350);
    fadeTopIn->setStartValue(0.0);
    fadeTopIn->setEndValue(1.0);

    QPropertyAnimation *fadeBottomIn = new QPropertyAnimation(bottomEffect, "opacity", this);
    fadeBottomIn->setDuration(350);
    fadeBottomIn->setStartValue(0.0);
    fadeBottomIn->setEndValue(1.0);

    QPropertyAnimation *fadeTitleIn = new QPropertyAnimation(titleEffect, "opacity", this);
    fadeTitleIn->setDuration(450);
    fadeTitleIn->setStartValue(0.0);
    fadeTitleIn->setEndValue(1.0);

    fadeOverlayIn->start(QAbstractAnimation::DeleteWhenStopped);
    fadeBannerIn->start(QAbstractAnimation::DeleteWhenStopped);
    fadeTopIn->start(QAbstractAnimation::DeleteWhenStopped);
    fadeBottomIn->start(QAbstractAnimation::DeleteWhenStopped);
    fadeTitleIn->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(1300, this, [=]() {
        QPropertyAnimation *fadeOverlayOut = new QPropertyAnimation(overlayEffect, "opacity", this);
        fadeOverlayOut->setDuration(450);
        fadeOverlayOut->setStartValue(1.0);
        fadeOverlayOut->setEndValue(0.0);

        QPropertyAnimation *fadeBannerOut = new QPropertyAnimation(bannerEffect, "opacity", this);
        fadeBannerOut->setDuration(450);
        fadeBannerOut->setStartValue(1.0);
        fadeBannerOut->setEndValue(0.0);

        QPropertyAnimation *fadeTopOut = new QPropertyAnimation(topEffect, "opacity", this);
        fadeTopOut->setDuration(450);
        fadeTopOut->setStartValue(1.0);
        fadeTopOut->setEndValue(0.0);

        QPropertyAnimation *fadeBottomOut = new QPropertyAnimation(bottomEffect, "opacity", this);
        fadeBottomOut->setDuration(450);
        fadeBottomOut->setStartValue(1.0);
        fadeBottomOut->setEndValue(0.0);

        QPropertyAnimation *fadeTitleOut = new QPropertyAnimation(titleEffect, "opacity", this);
        fadeTitleOut->setDuration(450);
        fadeTitleOut->setStartValue(1.0);
        fadeTitleOut->setEndValue(0.0);

        connect(fadeOverlayOut, &QPropertyAnimation::finished, this, [=]() {
            emit dialogueFinished();
        });

        fadeOverlayOut->start(QAbstractAnimation::DeleteWhenStopped);
        fadeBannerOut->start(QAbstractAnimation::DeleteWhenStopped);
        fadeTopOut->start(QAbstractAnimation::DeleteWhenStopped);
        fadeBottomOut->start(QAbstractAnimation::DeleteWhenStopped);
        fadeTitleOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

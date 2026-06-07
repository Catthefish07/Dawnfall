#include "avatarpicker.h"
#include "lobbyscreen.h"   //for loadAsset()
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>


AvatarCell::AvatarCell(const AvatarInfo &info, QWidget *parent)
    : QWidget(parent)
    , m_info(info)
{
    setFixedSize(90, 108);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_TranslucentBackground);
    m_pixmap = loadAsset(info.assetPath);
}

void AvatarCell::setSelected(bool selected)
{
    m_selected = selected;
    update();
}

void AvatarCell::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect r = rect();

    //bg panel
    QColor bg = m_selected  ? QColor(255, 215, 0,  60)
                : m_hovered   ? QColor(255, 255, 255, 30)
                            : QColor(255, 255, 255, 12);
    QColor border = m_selected  ? QColor(255, 215, 0, 200)
                    : m_hovered   ? QColor(255, 255, 255, 80)
                                : QColor(255, 255, 255, 30);

    p.setPen(QPen(border, m_selected ? 2 : 1));
    p.setBrush(bg);
    p.drawRoundedRect(r.adjusted(1, 1, -1, -1), 8, 8);

    //portrait image (top 68px of the 96px cell)
    QRect imgRect(8, 6, 74, 74);
    if (!m_pixmap.isNull())
        p.drawPixmap(imgRect, m_pixmap.scaled(
                                  imgRect.size(),
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation));

    if (!m_info.owned) {
        p.setOpacity(0.6);
        p.fillRect(imgRect, QColor(0, 0, 0, 180));
        p.setOpacity(1.0);
        QFont lf = p.font();
        lf.setPointSize(20);
        p.setFont(lf);
        p.setPen(QColor(255, 215, 0));
        p.drawText(imgRect, Qt::AlignCenter, "🔒");
        QFont sf = p.font();
        sf.setPointSize(7);
        p.setFont(sf);
        p.setPen(QColor(255, 215, 0, 180));
        p.drawText(QRect(0, imgRect.bottom()-2, 90, 14),
                   Qt::AlignCenter, "Buy in Shop");
    }

    //name label at bottom
    QRect nameRect(0, 82, 90, 22);
    p.setPen(m_selected ? QColor(255, 215, 0) : QColor(220, 220, 220));
    QFont f = p.font();
    f.setPointSize(8);
    f.setBold(m_selected);
    p.setFont(f);
    p.drawText(nameRect, Qt::AlignCenter, m_info.name);
}

void AvatarCell::enterEvent(QEnterEvent *)
{
    m_hovered = true;
    update();
}

void AvatarCell::leaveEvent(QEvent *)
{
    m_hovered = false;
    update();
}

void AvatarCell::mousePressEvent(QMouseEvent *)
{
    emit chosen(m_info);
}


//avatarpicker
AvatarPicker::AvatarPicker(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);

    //9 playable characters
    m_avatars = {
                 { 0, "MC",   "assets/profilepic/mc_pfp", true },
                 { 1, "Ethan",      "assets/profilepic/ethan_pfp", true },
                 { 2, "Hubert",    "assets/profilepic/hubert_pfp", true },
                 { 3, "Lynn",     "assets/profilepic/ninelilea_pfp", true },
                 { 4, "Kae",    "assets/profilepic/kae_pfp", true },
                 { 5, "Zey",    "assets/profilepic/zey_pfp", true },
                 { 6, "Den", "assets/profilepic/den_pfp", true },
                 { 7, "Ced",  "assets/profilepic/ced_pfp", true },
                 { 8, "Anak Agung",   "assets/profilepic/agung_pfp",  false },
                 };

    buildUi();
}

void AvatarPicker::buildUi()
{
    //outer card with dark styled background
    QWidget *card = new QWidget(this);
    card->setObjectName("avatarCard");
    card->setStyleSheet(
        "QWidget#avatarCard {"
        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "      stop:0 #1e2d40, stop:1 #0f1923);"
        "  border-radius: 14px;"
        "  border: 2px solid rgba(255,255,255,0.20);"
        "}");

    QVBoxLayout *vl = new QVBoxLayout(card);
    vl->setContentsMargins(12, 10, 12, 12);
    vl->setSpacing(8);

    //title row
    QHBoxLayout *titleRow = new QHBoxLayout;
    titleRow->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLbl = new QLabel("Choose Character", card);
    titleLbl->setStyleSheet(
        "color: #ffd700; font-size: 15px; font-weight: bold; font-family: 'YourFont'; background: transparent;");

    QPushButton *closeBtn = new QPushButton("✕", card);
    closeBtn->setFixedSize(22, 22);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  background: rgba(255,255,255,0.10); border: none;"
        "  border-radius: 11px; color: white; font-size: 11px; font-weight: bold;"
        "}"
        "QPushButton:hover { background: rgba(200,50,50,0.85); }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);

    titleRow->addWidget(titleLbl, 1);
    titleRow->addWidget(closeBtn);
    vl->addLayout(titleRow);

    //grid: 3 columns × 3 rows
    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(6);

    for (int i = 0; i < m_avatars.size(); ++i) {
        AvatarCell *cell = new AvatarCell(m_avatars[i], card);
        cell->setSelected(i == m_currentIndex);
        connect(cell, &AvatarCell::chosen, this, &AvatarPicker::onCellChosen);
        m_cells.append(cell);
        grid->addWidget(cell, i / 3, i % 3);
    }
    vl->addLayout(grid);

    //fit dialog to card
    QVBoxLayout *dlgLayout = new QVBoxLayout(this);
    dlgLayout->setContentsMargins(0, 0, 0, 0);
    dlgLayout->addWidget(card);

    adjustSize();
}

void AvatarPicker::setCurrentIndex(int index)
{
    m_currentIndex = index;
    for (int i = 0; i < m_cells.size(); ++i)
        m_cells[i]->setSelected(i == index);
}

void AvatarPicker::showBelow(QWidget *anchor)
{
    adjustSize();

    //position just below the anchor widget
    QPoint global = anchor->mapToGlobal(QPoint(0, anchor->height() + 4));
    move(global);

    //keep within the primary screen
    if (QScreen *scr = QApplication::primaryScreen()) {
        QRect sg = scr->availableGeometry();
        QRect dg = geometry();
        if (dg.right()  > sg.right())  move(sg.right()  - dg.width(),  y());
        if (dg.bottom() > sg.bottom()) move(x(), sg.bottom() - dg.height());
        if (x() < sg.left())           move(sg.left(), y());
        if (y() < sg.top())            move(x(), sg.top());
    }

    show();
    raise();
    activateWindow();
}

void AvatarPicker::onCellChosen(const AvatarInfo &info)
{
    if (!info.owned) {
        QMessageBox::information(this, "Locked",
                                 QString("%1 is locked!\nVisit the Shop to unlock this character.")
                                     .arg(info.name));
        return;   // ← block selection, don't emit, don't close
    }
    m_currentIndex = info.index;
    for (int i = 0; i < m_cells.size(); ++i)
        m_cells[i]->setSelected(i == info.index);
    emit avatarSelected(info.index, info.assetPath, info.name);
    close();
}

void AvatarPicker::setOwnedStates(const bool owned[], int count)
{
    for (int i = 0; i < qMin(count, m_avatars.size()); ++i) {
        m_avatars[i].owned = owned[i];
        if (i < m_cells.size())
            m_cells[i]->update();   //repaint to show/hide lock
    }
}
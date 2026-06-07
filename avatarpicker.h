#ifndef AVATARPICKER_H
#define AVATARPICKER_H
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <QString>
#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QScreen>
#include <QApplication>

//AvatarPicker *picker = new AvatarPicker(this);
//connect(picker, &AvatarPicker::avatarSelected,
//          this,   &LobbyScreen::onAvatarSelected);
//picker->showBelow(m_playerIdBtn);   // anchors under the button
//avatarSelected() carries:
//index 0 based character index (will store in PlayerRecord)

//one avatar entry
struct AvatarInfo {
    int index;       //0-8
    QString name;        //display name
    QString assetPath;   //base path WITHOUT extension (loadAsset .PNG/.png)
    bool owned = true;
};

//small clickable cell inside the picker grid
class AvatarCell : public QWidget
{
    Q_OBJECT
public:
    explicit AvatarCell(const AvatarInfo &info, QWidget *parent = nullptr);
    void setSelected(bool selected);

signals:
    void chosen(const AvatarInfo &info);

protected:
    void paintEvent    (QPaintEvent  *) override;
    void enterEvent    (QEnterEvent  *) override;
    void leaveEvent    (QEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    AvatarInfo m_info;
    QPixmap    m_pixmap;
    bool       m_hovered  = false;
    bool       m_selected = false;
};

//picker dialog
class AvatarPicker : public QDialog
{
    Q_OBJECT
public:
    explicit AvatarPicker(QWidget *parent = nullptr);

    //positions the dialog below widget
    void showBelow(QWidget *anchorWidget);

    //change which cell appears highlighted (pass PlayerRecord.avatarIndex)
    void setCurrentIndex(int index);

    //if owned unlocked
    void setOwnedStates(const bool owned[], int count);

signals:
    //emitted when the player clicks a character portrait
    void avatarSelected(int index, const QString &assetPath, const QString &name);

private slots:
    void onCellChosen(const AvatarInfo &info);

private:
    void buildUi();

    QVector<AvatarInfo>  m_avatars;
    QVector<AvatarCell*> m_cells;
    int                  m_currentIndex = 0;
};

#endif
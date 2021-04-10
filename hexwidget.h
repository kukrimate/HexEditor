#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QWidget>
#include <QScrollBar>
#include <QFile>
#include "selection.h"

class HexWidget : public QWidget
{
    Q_OBJECT
private:
    QScrollBar scroll_bar;
    Selection selection;

    // Underlying file
    QFile file;

    // For rendering fonts
    QFont font;
    QFontMetrics font_metrics;

    // Cursor position
    qint64 cursor_offs;

    // Grid translation offsets
    int grid_x, grid_y;
    int cell_width, cell_height;

    // Current number of displayed lines
    qint64 displayedLines();

    // Check if offset is currently on screen
    bool isOnScreen(qint64 offset);

    // Translate on-GUI coordinates into an offset into file
    qint64 translateGuiCoords(int x, int y);

public:
    explicit HexWidget(QString fileName, QWidget *parent = nullptr);
    ~HexWidget() override;

    qint64 fileSize();
    void gotoOffset(qint64 offset, bool extend=false);

    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
    virtual void keyPressEvent(QKeyEvent *) override;
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void resizeEvent(QResizeEvent *) override;
    virtual void paintEvent(QPaintEvent *) override;
};

#endif // HEXWIDGET_H

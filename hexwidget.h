#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QWidget>
#include <QScrollBar>
#include <QFile>
#include "selection.h"

enum CursorDeflect {
    ForceLeft   = 0,
    ForceRight  = 1,
    PreserveEol = 2,
};

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

    // Cursor position (twice the granularity of file offsets)
    qint64 cursor_pos;

    // Grid translation offsets
    int grid_x, grid_y;
    int cell_width, cell_height;

    // Current number of displayed lines
    qint64 displayedLines();

    // Check if offset is currently on screen
    bool isPosOnScreen(qint64 offset);

    // Translate on-GUI coordinates into an offset into file
    qint64 translateGuiCoords(int x, int y, CursorDeflect &deflect);

public:
    explicit HexWidget(QString fileName, QWidget *parent = nullptr);
    ~HexWidget() override;

    qint64 fileSize();

    void cursorToOffset(qint64 offset,
                        CursorDeflect deflect,
                        bool extend_selection=false);

    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
    virtual void keyPressEvent(QKeyEvent *) override;
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void resizeEvent(QResizeEvent *) override;
    virtual void paintEvent(QPaintEvent *) override;
};

#endif // HEXWIDGET_H

#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QWidget>
#include <QScrollBar>
#include <QFile>

class Selection
{
private:
    qint64 pivot, bound;

public:
    Selection() : pivot(0), bound(0) {}
    qint64 pivotVal() { return pivot; }
    qint64 begin() { return pivot < bound ? pivot : bound; }
    qint64 end() { return pivot > bound ? pivot : bound; }
    bool valid() { return begin() != end(); }
    bool inRange(qint64 val) { return val >= begin() && val < end(); }
    void setPivot(qint64 val) {
        pivot = val;
        bound = val;
    }
    void extend(qint64 val) { bound = val; }
};

enum CursorDeflect {
    NoDeflect   = 0,
    ToPrevious  = 1,
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

    // Cursor position
    qint64 cursor_pos;
    CursorDeflect cursor_deflect;

    // Grid translation offsets
    int grid_x, grid_y;
    int cell_width, cell_height;

    // Current number of displayed lines
    qint64 displayedLines();

    // Check if offset is currently on screen
    bool isOnScreen(qint64 offset);

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

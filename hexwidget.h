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

    /**
     * @brief displayedLines Current number of displayed lines
     * @return count
     */
    qint64 displayedLines();

    /**
     * @brief isOnScreen Check if an offset is currently on screen
     * @param offset offset
     * @return is it on screen?
     */
    bool isOnScreen(qint64 offset);

    /**
     * @brief translateGuiCoords Translate on-GUI coordinates into an offset
     * @param x x-coord
     * @param y y-coord
     * @return file offset
     */
    qint64 translateGuiCoords(int x, int y);

public:
    explicit HexWidget(QString fileName, QWidget *parent = nullptr);
    ~HexWidget() override;

    qint64 fileSize();
    void gotoOffset(qint64 offset);

    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
    virtual void keyPressEvent(QKeyEvent *) override;
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void resizeEvent(QResizeEvent *) override;
    virtual void paintEvent(QPaintEvent *) override;
};

#endif // HEXWIDGET_H

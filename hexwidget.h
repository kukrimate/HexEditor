#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QWidget>
#include <QScrollBar>
#include <QFile>

class HexWidget : public QWidget
{
    Q_OBJECT
private:
    QScrollBar scroll_bar;

    // Underlying file
    QFile file;
    // Number of lines it takes to display our file
    qint64 total_lines;

    // For rendering fonts
    QFont font;
    QFontMetrics font_metrics;

    // Current number of lines that can fit on screen
    qint64 displayed_lines;

    // Cursor position
    qint64 cursor_line, cursor_col;

    void moveCursorLines(qint64 lines);

public:
    explicit HexWidget(QString fileName, QWidget *parent = nullptr);
    ~HexWidget() override;

    qint64 fileSize();
    void gotoOffset(qint64 offset);

    virtual void keyPressEvent(QKeyEvent *) override;
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void resizeEvent(QResizeEvent *) override;
    virtual void paintEvent(QPaintEvent *) override;
};

#endif // HEXWIDGET_H

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

    // For rendering fonts
    QFont font;
    QFontMetrics font_metrics;

    // Cursor position
    qint64 cursor_offs;

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

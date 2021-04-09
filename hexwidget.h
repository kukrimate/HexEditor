#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QWidget>
#include <QScrollBar>
#include <QFile>

class HexWidget : public QWidget
{
    Q_OBJECT
private:
    // Underlying file
    QFile file;
    // Current window into file
    qint64 window_offs;
    // Line metrics
    int total_lines, can_fit_lines;
    // For rendering fonts
    QFont font;
    QFontMetrics fm;
    // Scrolling
    QScrollBar scroll_bar;
    // Cursor position
    qint64 cursor_offs;

    void handleResize();
private slots:
    void handleScroll(int);
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

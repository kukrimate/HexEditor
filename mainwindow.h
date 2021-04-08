#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void handleOpen();
    void handleTabClose();
    void handleGoto();
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gotodialog.h"

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
    GotoDialog gotoDialog;
    virtual void keyPressEvent(QKeyEvent *) override;

private slots:
    void handleOpen();
    void handleTabClose();
    void handleGoto();
};

#endif // MAINWINDOW_H

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
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    GotoDialog gotoDialog;
    virtual bool eventFilter(QObject *, QEvent *) override;

private slots:
    void handleOpen();
    void handleTabChange();
    void handleTabClose();
    void handleGoto();
};

#endif // MAINWINDOW_H

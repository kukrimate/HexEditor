#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QTabWidget>
#include "gotodialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // Menu bar
    QAction action_open;
    QAction action_save;
    QAction action_save_as;
    QAction action_quit;
    QMenu file_menu;

    QAction action_copy;
    QAction action_cut;
    QAction action_paste;
    QMenu edit_menu;

    QAction action_goto;
    QMenu find_menu;

    QMenuBar menu_bar;


    // Central widget
    QWidget central_widget;
    QVBoxLayout central_widget_layout;
    QTabWidget editor_tabs;

    // Dialogs
    GotoDialog gotoDialog;

    // Methods
    virtual bool eventFilter(QObject *, QEvent *) override;

private slots:
    void handleOpen();
    void handleTabChange();
    void handleTabClose();
    void handleGoto();
};

#endif // MAINWINDOW_H

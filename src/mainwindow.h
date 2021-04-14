/*
 * HexEditor -- Qt based hex editor
 * Copyright (C) 2021  Mate Kukri
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
    QAction action_paste_insert;
    QAction action_copy_offset;
    QAction action_fill;
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
    void handleCopy();
    void handleGoto();
};

#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "hexwidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QSizePolicy>
#include <QKeyEvent>
#include <QMessageBox>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    action_open("&Open"),
    action_save("&Save"),
    action_save_as("S&ave As"),
    action_quit("&Quit"),
    file_menu("&File"),
    action_copy("&Copy"),
    action_cut("C&ut"),
    action_paste("&Paste"),
    edit_menu("&Edit"),
    action_goto("&Goto offset"),
    find_menu("Fi&nd"),
    menu_bar(this),
    central_widget(this),
    editor_tabs(&central_widget),
    gotoDialog(this)
{
    action_open.setShortcut(QKeySequence("Ctrl+O"));
    file_menu.addAction(&action_open);
    action_save.setShortcut(QKeySequence("Ctrl+S"));
    file_menu.addAction(&action_save);
    file_menu.addAction(&action_save_as);
    file_menu.addSeparator();
    action_quit.setShortcut(QKeySequence("Ctrl+Q"));
    file_menu.addAction(&action_quit);
    menu_bar.addMenu(&file_menu);

    action_copy.setShortcut(QKeySequence("Ctrl+C"));
    edit_menu.addAction(&action_copy);
    action_cut.setShortcut(QKeySequence("Ctrl+X"));
    edit_menu.addAction(&action_cut);
    action_paste.setShortcut(QKeySequence("Ctrl+V"));
    edit_menu.addAction(&action_paste);
    menu_bar.addMenu(&edit_menu);

    action_goto.setShortcut(QKeySequence("Ctrl+G"));
    find_menu.addAction(&action_goto);
    menu_bar.addMenu(&find_menu);

    setMenuBar(&menu_bar);

    editor_tabs.setTabsClosable(true);
    editor_tabs.setMovable(true);
    central_widget_layout.addWidget(&editor_tabs);
    central_widget_layout.setMargin(0);
    central_widget.setLayout(&central_widget_layout);
    setCentralWidget(&central_widget);
    setWindowTitle(tr("HexEditor"));
    resize(800, 600);

    // Hook up event handlers
    QObject::connect(&action_open, SIGNAL(triggered(bool)), this, SLOT(handleOpen()));
    QObject::connect(&action_quit, SIGNAL(triggered(bool)), this, SLOT(close()));
    QObject::connect(&action_goto, SIGNAL(triggered(bool)), this, SLOT(handleGoto()));
    QObject::connect(&editor_tabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabChange()));
    QObject::connect(&editor_tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(handleTabClose()));
    qApp->installEventFilter(this);


    // For testing
    move(QApplication::screens().at(0)->geometry().center() - rect().center());
    this->setWindowFlag(Qt::WindowType::Dialog);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *in_event)
{
    if (in_event->type() == QEvent::Type::KeyPress) {
        QKeyEvent *event = reinterpret_cast<QKeyEvent*>(in_event);

        if (event->modifiers() == Qt::KeyboardModifier::ControlModifier) {
            if (event->key() == Qt::Key_W) {
                handleTabClose();
                return 1;
            }
        }

        if (event->modifiers() == Qt::KeyboardModifier::AltModifier) {
            int idx = 0;
            switch (event->key()) {
            case Qt::Key_1: idx = 1; break;
            case Qt::Key_2: idx = 2; break;
            case Qt::Key_3: idx = 3; break;
            case Qt::Key_4: idx = 4; break;
            case Qt::Key_5: idx = 5; break;
            case Qt::Key_6: idx = 6; break;
            case Qt::Key_7: idx = 7; break;
            case Qt::Key_8: idx = 8; break;
            case Qt::Key_9: idx = 9; break;
            }
            if (idx > 0 && idx <= editor_tabs.count()) {
                editor_tabs.setCurrentIndex(idx - 1);
                return 1;
            }
        }
    }
    return QObject::eventFilter(obj, in_event);
}

void MainWindow::handleOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (fileName == "")
        return;

    try {
        auto editor = new HexWidget(fileName);
        int newIdx = editor_tabs.addTab(editor, QFileInfo(fileName).fileName());
        editor_tabs.setCurrentIndex(newIdx);
    } catch (QString err) {
        QMessageBox msgBox(this);
        msgBox.setText(err);
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
    }
}

void MainWindow::handleTabChange()
{
    HexWidget *hex_widget = reinterpret_cast<HexWidget*>(editor_tabs.currentWidget());
    if (hex_widget) {
        hex_widget->setFocus(Qt::FocusReason::NoFocusReason);
    }
}

void MainWindow::handleTabClose()
{
    HexWidget *hex_widget = reinterpret_cast<HexWidget*>(editor_tabs.currentWidget());
    if (hex_widget) {
        delete hex_widget;
    }
}

void MainWindow::handleGoto()
{
    HexWidget *hex_widget = reinterpret_cast<HexWidget*>(editor_tabs.currentWidget());
    if (hex_widget) {
        gotoDialog.setFileSize(hex_widget->fileSize());
        if (gotoDialog.exec() == QDialog::Accepted) {
            hex_widget->cursorToOffset(gotoDialog.getEnteredOffset(), CursorDeflect::NoDeflect);
        }
    }
}

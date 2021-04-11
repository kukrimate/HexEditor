#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hexwidget.h"
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
    ui(new Ui::MainWindow),
    gotoDialog(this)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);

    move(QApplication::screens().at(0)->geometry().center() - rect().center());
    this->setWindowFlag(Qt::WindowType::Dialog);
}

MainWindow::~MainWindow()
{
    delete ui;
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
            if (idx > 0 && idx <= ui->editorTabs->count()) {
                ui->editorTabs->setCurrentIndex(idx - 1);
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
        int newIdx = ui->editorTabs->addTab(editor, QFileInfo(fileName).fileName());
        ui->editorTabs->setCurrentIndex(newIdx);
    } catch (QString err) {
        QMessageBox msgBox(this);
        msgBox.setText(err);
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
    }
}

void MainWindow::handleTabChange()
{
    HexWidget *hex_widget = reinterpret_cast<HexWidget*>(ui->editorTabs->currentWidget());
    if (hex_widget) {
        hex_widget->setFocus(Qt::FocusReason::NoFocusReason);
    }
}

void MainWindow::handleTabClose()
{
    HexWidget *hex_widget = reinterpret_cast<HexWidget*>(ui->editorTabs->currentWidget());
    if (hex_widget) {
        delete hex_widget;
    }
}

void MainWindow::handleGoto()
{
    HexWidget *hex_widget = reinterpret_cast<HexWidget*>(ui->editorTabs->currentWidget());
    if (hex_widget) {
        gotoDialog.setFileSize(hex_widget->fileSize());
        if (gotoDialog.exec() == QDialog::Accepted) {
            hex_widget->cursorToOffset(gotoDialog.getEnteredOffset(), CursorDeflect::ForceLeft);
        }
    }
}

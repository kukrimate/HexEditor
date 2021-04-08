#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gotodialog.h"
#include "hexwidget.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QSizePolicy>
#include <QKeyEvent>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::WindowType::Dialog);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::KeyboardModifier::ControlModifier) {
        if (event->key() == Qt::Key_W) {
            handleTabClose();
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
        }
    }
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

void MainWindow::handleTabClose()
{
    int idx = ui->editorTabs->currentIndex();
    if (idx >= 0)
        delete ui->editorTabs->widget(idx);
}

void MainWindow::handleGoto()
{
    HexWidget *hw =
            reinterpret_cast<HexWidget*>(ui->editorTabs->currentWidget());
    if (!hw)
        return;

    GotoDialog gotoDialog(hw->fileSize(), this);
    if (gotoDialog.exec() == QDialog::Accepted) {
        hw->gotoOffset(gotoDialog.getEnteredOffset());
    }
}

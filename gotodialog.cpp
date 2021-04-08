#include "gotodialog.h"
#include "ui_gotodialog.h"
#include <QMessageBox>

GotoDialog::GotoDialog(qint64 fileSize, QWidget *parent) :
    QDialog(parent), ui(new Ui::GotoDialog), fileSize(fileSize)
{
    ui->setupUi(this);
}

GotoDialog::~GotoDialog()
{
    delete ui;
}

qint64 GotoDialog::getEnteredOffset()
{
    return enteredOffset;
}

void GotoDialog::validateThenAccept()
{
    const char *str = ui->offsetLineEdit->text().toStdString().c_str();
    char *endptr;

    errno = 0;
    enteredOffset = std::strtoll(str, &endptr, 0);
    if (*str != 0 && *endptr == 0 && errno == 0 && enteredOffset >= 0 && enteredOffset < fileSize) {
        accept();
    } else {
        QMessageBox msgBox(this);
        msgBox.setText("Invalid offset!");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
    }
}

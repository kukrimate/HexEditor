#include "gotodialog.h"
#include <QMessageBox>

GotoDialog::GotoDialog(QWidget *parent) :
    QDialog(parent),
    offset_box(this),
    offset_line_edit_label("Goto offset:", &offset_box),
    offset_line_edit(&offset_box),
    button_box(QDialogButtonBox::StandardButton::Ok
               | QDialogButtonBox::StandardButton::Cancel, this),
    file_size(0)
{
    // Setup offset box
    offset_box_layout.addWidget(&offset_line_edit_label);
    offset_box_layout.addWidget(&offset_line_edit);
    offset_box.setLayout(&offset_box_layout);

    // Setup main UI
    layout.addWidget(&offset_box);
    layout.addStretch();
    layout.addWidget(&button_box);
    setLayout(&layout);
    setWindowTitle(tr("Goto offset"));
    resize(400, 95);

    // Connect event handlers
    QObject::connect(&button_box, SIGNAL(rejected()), this, SLOT(reject()));
    QObject::connect(&button_box, SIGNAL(accepted()), this, SLOT(validateThenAccept()));
}

void GotoDialog::setFileSize(qint64 fileSize)
{
    this->file_size = fileSize;
}

qint64 GotoDialog::getEnteredOffset()
{
    return enetered_offset;
}

void GotoDialog::validateThenAccept()
{
    const char *str = offset_line_edit.text().toStdString().c_str();
    char *endptr;

    errno = 0;
    enetered_offset = std::strtoll(str, &endptr, 0);
    if (*str != 0 && *endptr == 0 && errno == 0 && enetered_offset >= 0 && enetered_offset < file_size) {
        accept();
    } else {
        QMessageBox msgBox(this);
        msgBox.setText("Invalid offset!");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
    }
}

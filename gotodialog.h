#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

class GotoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GotoDialog(QWidget *parent = nullptr);
    void setFileSize(qint64 file_size);
    qint64 getEnteredOffset();

private:
    // UI
    QWidget offset_box;
    QLabel offset_line_edit_label;
    QLineEdit offset_line_edit;
    QHBoxLayout offset_box_layout;
    QDialogButtonBox button_box;
    QVBoxLayout layout;

    // Saved values
    qint64 file_size, enetered_offset;

private slots:
    void validateThenAccept();
};

#endif // GOTODIALOG_H

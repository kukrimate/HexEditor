#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>

namespace Ui {
class GotoDialog;
}

class GotoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GotoDialog(QWidget *parent = nullptr);
    ~GotoDialog();
    void setFileSize(qint64 fileSize);
    qint64 getEnteredOffset();

private:
    Ui::GotoDialog *ui;
    qint64 fileSize, enteredOffset;
private slots:
    void validateThenAccept();
};

#endif // GOTODIALOG_H

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
    explicit GotoDialog(qint64 fileSize, QWidget *parent = nullptr);
    ~GotoDialog();
    qint64 getEnteredOffset();

private:
    Ui::GotoDialog *ui;
    qint64 fileSize, enteredOffset;
private slots:
    void validateThenAccept();
};

#endif // GOTODIALOG_H

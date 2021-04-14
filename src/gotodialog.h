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

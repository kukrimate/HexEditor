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

#include "hexwidget.h"
#include <QDebug>
#include <QApplication>
#include <QObject>
#include <QPainter>
#include <QStyle>
#include <QKeyEvent>
#include <QKeySequence>

static int    FONT_SIZE = 10;
static int    BYTES_PER_LINE = 16;
static int    SPLITAT = 8;
static int    GAP = 10;
static int    BIGGAP = 20;
static QColor BLACK(0, 0, 0);
static QColor WHITE(255, 255, 255);
static QColor BLUE(0, 70, 255);
static QColor GRAY(119, 119, 119);

#define BPL_MASK (BYTES_PER_LINE - 1)

HexWidget::HexWidget(QString fileName, QMenu &context_menu, QWidget *parent)
    : QWidget(parent),
      scroll_bar(this),
      context_menu(context_menu),
      file(fileName),
      font("DejaVu Sans Mono", FONT_SIZE),
      font_metrics(font),
      cursor_pos(0),
      cursor_deflect(CursorDeflect::NoDeflect)
{
    file.open(QFile::ReadOnly);
    if (file.error() != QFile::FileError::NoError) {
        throw file.errorString();
    }

    // Setup scrollbar
    qint64 total_lines = (file.size() + BYTES_PER_LINE - 1) / BYTES_PER_LINE;
    if (total_lines > INT_MAX) {
        // TODO: fixme, somehow, scrollbars suck in Qt ;(
        throw QString("File too big to display!");
    }
    QObject::connect(&scroll_bar, SIGNAL(valueChanged(int)), this, SLOT(repaint()));
    scroll_bar.setRange(0, static_cast<int>(total_lines - 1));
    scroll_bar.show();


    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

HexWidget::~HexWidget()
{
    file.close();
}

qint64 HexWidget::fileSize()
{
    return file.size();
}

void HexWidget::cursorToOffset(qint64 offset, CursorDeflect deflect, bool extend)
{
    if (offset < 0)
        return;

    // Update selection accordingly
    if (extend || QApplication::keyboardModifiers() == Qt::KeyboardModifier::ShiftModifier) {
        selection.extend(offset);
    } else {
        selection.setPivot(offset);
    }

    // Save previous cursor position
    auto prev_cursor_offs = cursor_pos;
    auto prev_cursor_deflect = cursor_deflect;

    if (offset >= file.size()) {
        // Always deflect at EOF
        offset = file.size();
        cursor_deflect = CursorDeflect::ToPrevious;
    } else if (selection.valid() && cursor_pos > selection.pivotVal()) {
        // After the selection
        cursor_deflect = CursorDeflect::ToPrevious;
    } else if (deflect != CursorDeflect::PreserveEol) {
        // No preserve instructed
        cursor_deflect = deflect;
    } else if (offset & BPL_MASK) {
        // Ignore preserve when not at EOL
        cursor_deflect = CursorDeflect::NoDeflect;
    }
    cursor_pos = offset;

    if (!isCursorOnScreen(cursor_pos, cursor_deflect)) {
        if (!isCursorOnScreen(prev_cursor_offs, prev_cursor_deflect)) {
            // Cursor was not on screen -> just put the exact line on the
            // top of the screen
            scroll_bar.setValue(static_cast<int>(cursor_pos / BYTES_PER_LINE));
        } else {
            // If the new cursor is not on screen, we know the exact number of
            // lines we need to move the screen
            auto delta = static_cast<int>(cursor_pos / BYTES_PER_LINE
                                          - prev_cursor_offs / BYTES_PER_LINE);
            scroll_bar.setValue(scroll_bar.value() + delta);
        }
    }

    repaint();
}

std::optional<QByteArray> HexWidget::getSelectedBytes()
{
    if (!selection.valid())
        return {};

    file_lock.lock();
    file.seek(selection.begin());
    QByteArray bytes = file.read(selection.end() - selection.begin());
    file_lock.unlock();
    return std::optional(bytes);
}

qint64 HexWidget::maxDisplayedLines()
{
    return (this->height() - 30) / font_metrics.height();
}

bool HexWidget::isCursorOnScreen(qint64 pos, CursorDeflect deflect)
{
    qint64 screen_offs = scroll_bar.value() * BYTES_PER_LINE;
    qint64 screen_end = screen_offs + maxDisplayedLines() * BYTES_PER_LINE;

    // Take deflection into account
    if (deflect == CursorDeflect::ToPrevious && pos > 0) {
        --pos;
    }
    return pos >= screen_offs && pos < screen_end;
}

qint64 HexWidget::guiToOffset(int gui_x, int gui_y, CursorDeflect &deflect)
{
    // Clamp coordinates
    if (gui_x < grid_x) {
        gui_x = grid_x;
    }
    if (gui_y < grid_y) {
        gui_y = grid_y;
    }

    // Relativize coordinates to the GUI
    qint64 x = gui_x - grid_x;
    qint64 y = gui_y - grid_y;

    // Account for the split in the middle
    if (x / cell_width > SPLITAT) {
        x -= GAP;
    }

    // Calculate cell x coord
    x /= cell_width;
    if (x >= BYTES_PER_LINE) {
        x = BYTES_PER_LINE;
        deflect = CursorDeflect::ToPrevious;
    } else {
        deflect = CursorDeflect::NoDeflect;
    }

    // Calculate cell y coord
    y /= cell_height;
    if (y >= maxDisplayedLines()) {
        y = maxDisplayedLines();
    }

    return (scroll_bar.value() + y) * BYTES_PER_LINE + x;
}

void HexWidget::contextMenuEvent(QContextMenuEvent *event)
{
    context_menu.exec(this->mapToGlobal(event->pos()));
}

void HexWidget::mousePressEvent(QMouseEvent *event)
{
    // We only care about left clicks for now
    if (event->button() != Qt::MouseButton::LeftButton)
        return;

    // Move to the click offset
    CursorDeflect deflect;
    qint64 off = guiToOffset(event->x(), event->y(), deflect);
    cursorToOffset(off, deflect);
}

void HexWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->x() >= grid_x
            && event->x() < grid_x + BYTES_PER_LINE * cell_width + 2 * GAP
            && event->y() >= grid_y) {
        setCursor(Qt::CursorShape::IBeamCursor);
    } else {
        setCursor(Qt::CursorShape::ArrowCursor);
    }

    if (event->buttons() == Qt::MouseButton::LeftButton) {
        CursorDeflect deflect;
        qint64 off = guiToOffset(event->x(), event->y(), deflect);
        cursorToOffset(off, deflect, true);
    }
}

void HexWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        cursorToOffset(cursor_pos - BYTES_PER_LINE, CursorDeflect::PreserveEol);
        break;
    case Qt::Key_Down:
        cursorToOffset(cursor_pos + BYTES_PER_LINE, CursorDeflect::PreserveEol);
        break;
    case Qt::Key_PageUp:
        cursorToOffset(cursor_pos - BYTES_PER_LINE * maxDisplayedLines(), CursorDeflect::PreserveEol);
        break;
    case Qt::Key_PageDown:
        cursorToOffset(cursor_pos + BYTES_PER_LINE * maxDisplayedLines(), CursorDeflect::PreserveEol);
        break;
    case Qt::Key_Left:
        cursorToOffset(cursor_pos - 1, CursorDeflect::NoDeflect);
        break;
    case Qt::Key_Right:
        cursorToOffset(cursor_pos + 1, CursorDeflect::NoDeflect);
        break;
    case Qt::Key_Home:
        cursorToOffset((cursor_pos - cursor_deflect) & ~BPL_MASK, CursorDeflect::NoDeflect);
        break;
    case Qt::Key_End:
        cursorToOffset(((cursor_pos - cursor_deflect) | BPL_MASK) + 1, CursorDeflect::ToPrevious);
        break;
    }
}

void HexWidget::wheelEvent(QWheelEvent *event)
{
    scroll_bar.event(event);
}

void HexWidget::resizeEvent(QResizeEvent *)
{
    // Resize scrollbar
    scroll_bar.setGeometry(this->width() - scroll_bar.width(), 0, scroll_bar.width(), this->height());
}

void HexWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(BLUE);
    painter.setFont(font);

    int x = BIGGAP;
    int y = BIGGAP;

    // Draw heading
    QString offs_heading("Offset(hex)");
    painter.drawText(x, y, offs_heading);
    x += font_metrics.width(offs_heading) + BIGGAP;

    int byte_start = x;
    for (int i = 0; i < BYTES_PER_LINE; ++i) {
        auto num_str = QString::asprintf("%02X", i);
        painter.drawText(x, y, num_str);
        x += font_metrics.width(num_str);
        if (i == SPLITAT - 1) {
            x += BIGGAP;
        } else {
            x += GAP;
        }
    }

    int ascii_start = x + BIGGAP;
    QString ascii_heading("ASCII");
    painter.drawText(ascii_start, y, ascii_heading);
    y += GAP; // Leave extra gap after the heading

    // Compute cursor translation values
    grid_x = byte_start - GAP;
    grid_y = y + GAP;
    cell_width = font_metrics.width("00") + GAP;
    cell_height = font_metrics.height();

    // Draw file contents
    file_lock.lock();
    file.seek(scroll_bar.value() * BYTES_PER_LINE);
    for (int line_idx = 0; line_idx < maxDisplayedLines(); ++line_idx) {
        // Read data
        auto hexline_offs = file.pos();
        auto hexline = file.read(BYTES_PER_LINE);
        if (hexline.size() == 0)
            break;

        // Draw offset
        auto offs_str = QString::asprintf("%08llx", hexline_offs);
        painter.setPen(BLUE);
        x = BIGGAP;
        y += font_metrics.height();
        painter.drawText(x, y, offs_str);
        painter.setPen(BLACK);

        // Draw hex bytes
        x = byte_start;

        for (int col_idx = 0; col_idx < hexline.size(); ++col_idx) {
            qint64 cell_offs = hexline_offs + col_idx;

            // The byte as a string
            auto bstr = QString::asprintf("%02X", static_cast<unsigned char>(hexline.at(col_idx)));

            // Byte value x
            auto bstr_x = x;
            x += font_metrics.width(bstr);
            auto bstr_end_x = x;

            // Draw cursor
            if (cursor_deflect == CursorDeflect::NoDeflect && cursor_pos == cell_offs) {
                painter.fillRect(bstr_x, y + 4, -2, -font_metrics.capHeight() - 8, BLACK);
            } else if (cursor_deflect == CursorDeflect::ToPrevious && cursor_pos == cell_offs + 1) {
                painter.fillRect(x, y + 4, 2, -font_metrics.capHeight() - 8, BLACK);
            }


            // Add gap width if this is not the last column
            if (col_idx != hexline.size() - 1) {
                x += (col_idx == SPLITAT - 1 ? BIGGAP : GAP);
            }

            // Draw byte
            if (selection.inRange(cell_offs)) {
                int sel_width = x - bstr_x;
                if (cell_offs == selection.end() - 1) {
                    sel_width = bstr_end_x - bstr_x;
                }
                painter.fillRect(bstr_x, y + 4, sel_width, -font_metrics.height(), BLUE);
                painter.setPen(WHITE);
                painter.drawText(bstr_x, y, bstr);
                painter.setPen(BLACK);
            } else {
                painter.drawText(bstr_x, y, bstr);
            }
        }

        // Draw ASCII
        QString asciiLine;
        for (char byteVal : hexline) {
            if (31 < byteVal && byteVal < 127) {
                // Printable ASCII char
                asciiLine.append(QChar::fromLatin1(byteVal));
            } else {
                // Non-printable marker
                asciiLine.append('.');
            }
        }
        painter.drawText(ascii_start, y, asciiLine);
    }
    file_lock.unlock();
}

#include "hexwidget.h"
#include <QDebug>
#include <QApplication>
#include <QObject>
#include <QPainter>
#include <QStyle>
#include <QKeyEvent>

static int    FONT_SIZE = 10;
static int    BYTES_PER_LINE = 16;
static int    SPLITAT = 8;
static int    GAP = 10;
static int    BIGGAP = 20;
static QColor BLACK(0, 0, 0);
static QColor BLUE(0, 70, 255);
static QColor GRAY(119, 119, 119);

HexWidget::HexWidget(QString fileName, QWidget *parent)
    : QWidget(parent),
      scroll_bar(this),
      file(fileName),
      font("DejaVu Sans Mono", FONT_SIZE),
      font_metrics(font)
{
    setFocusPolicy(Qt::StrongFocus);

    file.open(QFile::ReadOnly);
    if (file.error() != QFile::FileError::NoError) {
        throw file.errorString();
    }

    // Calculate the number of lines for our file
    total_lines = (file.size() + BYTES_PER_LINE - 1) / BYTES_PER_LINE;
    displayed_lines = (this->height() - 30) / font_metrics.height();

    // Set cursor position
    cursor_line = 0;
    cursor_col = 0;

    // Setup scrollbar
    if (total_lines > INT_MAX) {
        // TODO: fixme, somehow, scrollbars suck in Qt ;(
        throw QString("File too big to display!");
    }
    QObject::connect(&scroll_bar, SIGNAL(valueChanged(int)), this, SLOT(repaint()));
    scroll_bar.setRange(0, static_cast<int>(total_lines - 1));
    scroll_bar.show();
}

HexWidget::~HexWidget()
{
    file.close();
}

qint64 HexWidget::fileSize()
{
    return file.size();
}

void HexWidget::gotoOffset(qint64 offset)
{
    scroll_bar.setValue(static_cast<int>(offset / BYTES_PER_LINE));
    repaint();
}

void HexWidget::moveCursorLines(qint64 lines)
{
    // Change cursor position
    cursor_line += lines;
    if (cursor_line < 0)
        cursor_line = 0;
    if (cursor_line >= total_lines)
        cursor_line = total_lines - 1;

    // Scroll if neeeded
    if (cursor_line < scroll_bar.value() || cursor_line >= scroll_bar.value() + displayed_lines) {
        scroll_bar.setValue(static_cast<int>(scroll_bar.value() + lines));
    }
    // If it's still outside the range, we just jump there
    if (cursor_line < scroll_bar.value() || cursor_line >= scroll_bar.value() + displayed_lines) {
        scroll_bar.setValue(static_cast<int>(cursor_line));
    }

    repaint();
}

void HexWidget::keyPressEvent(QKeyEvent *event)
{
    QKeyEvent *key_event = reinterpret_cast<QKeyEvent*>(event);
    switch (key_event->key()) {
    case Qt::Key_Up:
        moveCursorLines(-1);
        break;
    case Qt::Key_Down:
        moveCursorLines(1);
        break;
    case Qt::Key_Left:
        if (--cursor_col < 0) {
            cursor_col = BYTES_PER_LINE - 1;
            moveCursorLines(-1);
        } else {
            repaint();
        }
        break;
    case Qt::Key_Right:
        if (++cursor_col >= BYTES_PER_LINE) {
            cursor_col = 0;
            moveCursorLines(1);
        } else {
            repaint();
        }
        break;
    case Qt::Key_PageUp:
        moveCursorLines(-displayed_lines);
        break;
    case Qt::Key_PageDown:
        moveCursorLines(displayed_lines);
        break;
    case Qt::Key_Home:
        cursor_col = 0;
        repaint();
        break;
    case Qt::Key_End:
        cursor_col = BYTES_PER_LINE - 1;
        repaint();
        break;
    }
}

void HexWidget::wheelEvent(QWheelEvent *event)
{
    scroll_bar.event(event);
}

void HexWidget::resizeEvent(QResizeEvent *)
{
    // Update line count
    displayed_lines = (this->height() - 30) / font_metrics.height();
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

    // First line of the hexdump
    x = BIGGAP;
    y += font_metrics.height() + GAP;

    // Start reading bytes from the correct offset
    file.seek(scroll_bar.value() * BYTES_PER_LINE);

    for (int line_idx = 0; line_idx < displayed_lines; ++line_idx) {
        // Read data
        auto hexline = file.read(BYTES_PER_LINE);
        if (hexline.size() == 0)
            break;

        // Draw offset
        auto offs_str = QString::asprintf("%08llx", file.pos() - hexline.size());
        painter.setPen(BLUE);
        painter.drawText(x, y, offs_str);
        painter.setPen(BLACK);

        // Draw hex bytes
        x = byte_start;
        for (int col_idx = 0; col_idx < hexline.size(); ++col_idx) {
            // Draw cursor
            if (cursor_line == scroll_bar.value() + line_idx && cursor_col == col_idx) {
                painter.fillRect(x, y + 4, -2, -font_metrics.capHeight() - 8, BLACK);
            }

            // Draw byte
            auto bstr = QString::asprintf("%02X", static_cast<unsigned char>(hexline.at(col_idx)));
            painter.drawText(x, y, bstr);
            x += font_metrics.width(bstr);
            if (col_idx == SPLITAT - 1) {
                x += BIGGAP;
            } else {
                x += GAP;
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


        // Move to next line
        x = BIGGAP;
        y += font_metrics.height();
    }
}

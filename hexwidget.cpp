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
    : QWidget(parent), file(fileName), window_offs(0),
      font("DejaVu Sans Mono", FONT_SIZE), fm(font),
      scroll_bar(Qt::Orientation::Vertical, this),
      cursor_offs(0)
{
    setFocusPolicy(Qt::StrongFocus);

    file.open(QFile::ReadOnly);
    if (file.error() != QFile::FileError::NoError)
        throw file.errorString();

    // Calculate the number of lines for our file
    qint64 total_lines64 = (file.size() + BYTES_PER_LINE - 1) / BYTES_PER_LINE;
    if (total_lines64 > INT_MAX) // TODO: fixme, somehow, scrollbars suck in Qt ;(
        throw QString("File too big to display!");
    total_lines = static_cast<int>(total_lines64);

    QObject::connect(&scroll_bar, SIGNAL(valueChanged(int)), this, SLOT(handleScroll(int)));
    scroll_bar.show();
    handleResize();
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
    // Ignore invalid goto
    if (offset < 0 || offset > file.size())
        return;

    // Move cursor offset
    cursor_offs = offset;

    // See if we need to scroll
    scroll_bar.setValue(static_cast<int>(offset / BYTES_PER_LINE));
    repaint();
}

void HexWidget::handleResize()
{
    // Update line count
    can_fit_lines = (this->height() - 30) / fm.height();

    // Resize scrollbar
    int sb_max = total_lines - 1;
    scroll_bar.setRange(0, sb_max);
    scroll_bar.setGeometry(this->width() - scroll_bar.width(), 0, scroll_bar.width(), this->height());
}

void HexWidget::handleScroll(int pos)
{
    window_offs = pos * BYTES_PER_LINE;
    repaint();
}

void HexWidget::keyPressEvent(QKeyEvent *event)
{
    QKeyEvent *key_event = reinterpret_cast<QKeyEvent*>(event);
    qint64 new_offs;
    switch (key_event->key()) {
    case Qt::Key_Up:
        gotoOffset(cursor_offs - BYTES_PER_LINE);
        break;
    case Qt::Key_Down:
        gotoOffset(cursor_offs + BYTES_PER_LINE);
        break;
    case Qt::Key_Left:
        gotoOffset(cursor_offs - 1);
        break;
    case Qt::Key_Right:
        gotoOffset(cursor_offs + 1);
        break;
    case Qt::Key_PageUp:
        new_offs = cursor_offs - can_fit_lines * BYTES_PER_LINE;
        if (new_offs < 0) {
            new_offs = cursor_offs % BYTES_PER_LINE;
        }
        gotoOffset(new_offs);
        break;
    case Qt::Key_PageDown:
        gotoOffset(cursor_offs + can_fit_lines * BYTES_PER_LINE);
        break;
    case Qt::Key_Home:
        break;
    case Qt::Key_End:
        break;
    }
}

void HexWidget::wheelEvent(QWheelEvent *event)
{
    scroll_bar.event(event);
}

void HexWidget::resizeEvent(QResizeEvent *)
{
    handleResize();
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
    x += fm.width(offs_heading) + BIGGAP;

    int byte_start = x;
    for (int i = 0; i < BYTES_PER_LINE; ++i) {
        auto num_str = QString::asprintf("%02X", i);
        painter.drawText(x, y, num_str);
        x += fm.width(num_str);
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
    y += fm.height() + GAP;

    // Start reading bytes from the correct offset
    file.seek(window_offs);

    for (int line_idx = 0; line_idx < can_fit_lines; ++line_idx) {
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
            if (cursor_offs == window_offs + BYTES_PER_LINE * line_idx + col_idx) {
                painter.fillRect(x, y + 4, -2, -fm.capHeight() - 8, BLACK);
            }

            // Draw byte
            auto bstr = QString::asprintf("%02X", static_cast<unsigned char>(hexline.at(col_idx)));
            painter.drawText(x, y, bstr);
            x += fm.width(bstr);
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
        y += fm.height();
    }
}

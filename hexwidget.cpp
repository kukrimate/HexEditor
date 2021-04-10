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
static QColor WHITE(255, 255, 255);
static QColor BLUE(0, 70, 255);
static QColor GRAY(119, 119, 119);

#define REM_MASK(x) ((x) - 1)

HexWidget::HexWidget(QString fileName, QWidget *parent)
    : QWidget(parent),
      scroll_bar(this),
      file(fileName),
      font("DejaVu Sans Mono", FONT_SIZE),
      font_metrics(font),
      cursor_offs(0)
{
    setFocusPolicy(Qt::StrongFocus);

    file.open(QFile::ReadOnly);
    if (file.error() != QFile::FileError::NoError) {
        throw file.errorString();
    }

    setMouseTracking(true);

    // Setup scrollbar
    qint64 total_lines = (file.size() + BYTES_PER_LINE - 1) / BYTES_PER_LINE;
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

void HexWidget::gotoOffset(qint64 offset, bool extend)
{
    if (offset < 0 || offset > file.size())
        return;

    // Update cursor offset
    qint64 prev_cursor_offs = cursor_offs;
    cursor_offs = offset;

    if (!isOnScreen(cursor_offs)) {
        if (!isOnScreen(prev_cursor_offs)) {
            // Cursor was not on screen -> just put the exact line on the
            // top of the screen
            scroll_bar.setValue(static_cast<int>(offset / BYTES_PER_LINE));
        } else {
            // If the new cursor is not on screen, we know the exact number of
            // lines we need to move the screen
            auto delta = static_cast<int>(cursor_offs / BYTES_PER_LINE - prev_cursor_offs / BYTES_PER_LINE);
            scroll_bar.setValue(scroll_bar.value() + delta);
        }
    }

    if (extend || QApplication::keyboardModifiers() == Qt::KeyboardModifier::ShiftModifier) {
        selection.extend(cursor_offs);
    } else {
        selection.setPivot(cursor_offs);
    }

    repaint();
}

qint64 HexWidget::displayedLines()
{
    return (this->height() - 30) / font_metrics.height();
}

bool HexWidget::isOnScreen(qint64 offset)
{
    qint64 screen_offs = scroll_bar.value() * BYTES_PER_LINE;
    qint64 screen_end = screen_offs + displayedLines() * BYTES_PER_LINE;
    return offset >= screen_offs && offset < screen_end;
}

qint64 HexWidget::translateGuiCoords(int gui_x, int gui_y)
{
    // Outside the hex grid
    if (gui_x < grid_x || gui_y < grid_y || gui_x >= grid_x + BYTES_PER_LINE * cell_width + GAP)
        return -1;

    int x = gui_x - grid_x, y = gui_y - grid_y;
    if (x / cell_width > SPLITAT) {
        x -= GAP;
    }
    x /= cell_width;
    y /= cell_height;
    return (scroll_bar.value() + y) * BYTES_PER_LINE + x;
}

void HexWidget::mousePressEvent(QMouseEvent *event)
{
    // We only care about left clicks for now
    if (event->button() != Qt::MouseButton::LeftButton)
        return;

    // Move to the click offset
    gotoOffset(translateGuiCoords(event->x(), event->y()));
}

void HexWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->x() >= grid_x
            && event->x() < grid_x + BYTES_PER_LINE * cell_width + GAP
            && event->y() >= grid_y) {
        setCursor(Qt::CursorShape::IBeamCursor);
    } else {
        setCursor(Qt::CursorShape::ArrowCursor);
    }

    if (event->buttons() == Qt::MouseButton::LeftButton) {
        gotoOffset(translateGuiCoords(event->x(), event->y()), true);
    }
}

void HexWidget::keyPressEvent(QKeyEvent *event)
{
    QKeyEvent *key_event = reinterpret_cast<QKeyEvent*>(event);
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
        gotoOffset(cursor_offs - BYTES_PER_LINE * displayedLines());
        break;
    case Qt::Key_PageDown:
        gotoOffset(cursor_offs + BYTES_PER_LINE * displayedLines());
        break;
    case Qt::Key_Home:
        gotoOffset(cursor_offs & ~REM_MASK(BYTES_PER_LINE));
        break;
    case Qt::Key_End:
        gotoOffset((cursor_offs & ~REM_MASK(BYTES_PER_LINE)) + BYTES_PER_LINE - 1);
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

    bool drew_cursor = false;

    // Draw file contents
    file.seek(scroll_bar.value() * BYTES_PER_LINE);
    for (int line_idx = 0; line_idx < displayedLines(); ++line_idx) {
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
            bool is_selected = selection.inRange(cell_offs);

            // The byte as a string
            auto bstr = QString::asprintf("%02X", static_cast<unsigned char>(hexline.at(col_idx)));

            // Byte value x
            auto bstr_x = x;
            x += font_metrics.width(bstr);
            auto bstr_end_x = x;

            // Draw cursor
            if (!drew_cursor) {
                if (selection.valid() && selection.end() == cell_offs + 1) {
                    if (cursor_offs == cell_offs + 1) {
                        painter.fillRect(x, y + 4, 2, -font_metrics.capHeight() - 8, BLACK);
                        drew_cursor = true;
                    }
                } else {
                    if (cursor_offs == cell_offs) {
                        painter.fillRect(bstr_x, y + 4, -2, -font_metrics.capHeight() - 8, BLACK);
                        drew_cursor = true;
                    }
                }
            }

            // Add gap width if this is not the last column
            if (col_idx != hexline.size() - 1) {
                x += (col_idx == SPLITAT - 1 ? BIGGAP : GAP);
            }

            // Draw byte
            if (is_selected) {
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

    // Special case for cursor after the last bytes
    if (cursor_offs == file.size()) {
        painter.fillRect(x, y + 4, 2, -font_metrics.capHeight() - 8, BLACK);
    }
}

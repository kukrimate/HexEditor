#include "hexwidget.h"
#include <QDebug>
#include <QApplication>
#include <QObject>
#include <QPainter>
#include <QStyle>
#include <QWheelEvent>

static int    FONT_SIZE = 10;
static int    BYTES_PER_LINE = 16;
static int    SPLITAT = 8;
static int    GAP = 10;
static int    BIGGAP = 20;
static QColor BLACK(0, 0, 0);
static QColor BLUE(0, 0, 255);

HexWidget::HexWidget(QString fileName, QWidget *parent)
    : QWidget(parent), file(fileName), file_offs(0),
      font("DejaVu Sans Mono", FONT_SIZE), fm(font),
      scrollBar(Qt::Orientation::Vertical, this)
{
    file.open(QFile::ReadOnly);
    if (file.error() != QFile::FileError::NoError)
        throw file.errorString();

    // Calculate the number of lines for our file
    qint64 totalLines64 = (file.size() + BYTES_PER_LINE - 1) / BYTES_PER_LINE;
    if (totalLines64 > INT_MAX) // TODO: fixme, somehow, scrollbars suck in Qt ;(
        throw QString("File too big to display!");
    totalLines = static_cast<int>(totalLines64);

    // Only way I can figure out to get key events here ;(
    qApp->installEventFilter(this);

    QObject::connect(&scrollBar, SIGNAL(valueChanged(int)), this, SLOT(handleScroll(int)));
    scrollBar.show();
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
    assert(offset >= 0 && offset < fileSize());
    scrollBar.setValue(static_cast<int>(offset / BYTES_PER_LINE));
}

void HexWidget::handleResize()
{
    // Update line count
    canFitLines = (this->height() - 30) / fm.height();

    // Resize scrollbar
    int sbMax = totalLines - 1;
    scrollBar.setRange(0, sbMax);
    scrollBar.setGeometry(this->width() - scrollBar.width(), 0, scrollBar.width(), this->height());
}

void HexWidget::handleScroll(int pos)
{
    file_offs = pos * BYTES_PER_LINE;
    repaint();
}

bool HexWidget::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Type::KeyPress) {
        scrollBar.event(event);
    }
    return QObject::eventFilter(object, event);
}

void HexWidget::wheelEvent(QWheelEvent *event)
{
    scrollBar.event(event);
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
    for (int i = 0; i < 16; ++i) {
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
    file.seek(file_offs);

    for (int line_idx = 0; line_idx < canFitLines; ++line_idx) {
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

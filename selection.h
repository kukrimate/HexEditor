/*
 * Wrapper for maintaining and changing a selection
 */

#ifndef SELECTION_H
#define SELECTION_H

#include <QObject>

class Selection
{
public:
    Selection();

    qint64 begin();
    qint64 end();
    bool valid();
    bool inRange(qint64 val);
    void setPivot(qint64 val);
    void extend(qint64 val);

private:
    qint64 pivot, bound;
};

#endif // SELECTION_H

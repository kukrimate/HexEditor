#include "selection.h"

Selection::Selection() : pivot(0), bound(0) {}

qint64 Selection::pivotVal()
{
    return pivot;
}

qint64 Selection::begin()
{
    return pivot < bound ? pivot : bound;
}

qint64 Selection::end()
{
    return pivot > bound ? pivot : bound;
}

bool Selection::valid()
{
    return begin() != end();
}

bool Selection::inRange(qint64 val)
{
    return val >= begin() && val < end();
}

void Selection::setPivot(qint64 val)
{
    pivot = val;
    bound = val;
}

void Selection::extend(qint64 val)
{
    bound = val;
}

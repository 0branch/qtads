// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <QTimer>

#include "htmlsys.h"

/* Tads HTML layer class whose interface needs to be implemented by the
 * interpreter.
 *
 * See htmltads/htmlsys.h and htmltads/notes/porting.htm for information
 * about this class.
 */
class QTadsTimer: public QTimer, public CHtmlSysTimer
{
    Q_OBJECT

public slots:
    // We connect the timeout() signal to this slot.
    void trigger()
    {
        // If we have a callback, call it.
        if (func_ != nullptr) {
            invoke_callback();
        }
    }

public:
    QTadsTimer(void (*func)(void*), void* ctx, QObject* parent = nullptr)
        : QTimer(parent)
        , CHtmlSysTimer(func, ctx)
    {
        connect(this, &QTimer::timeout, this, &QTadsTimer::trigger);
    }

    // We bring this into public scope since we need to evaluate the callback
    // pointer in order to unregister the timer.
    using CHtmlSysTimer::func_;
};

/*
    Copyright 2003-2020 Nikos Chantziaras <realnc@gmail.com>

    This file is part of QTads.

    QTads is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    QTads is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with QTads. If not, see <https://www.gnu.org/licenses/>.
*/

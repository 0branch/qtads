// This is copyrighted software. More information is at the end of this file.
#include "hos_qt.h"

#include <QDebug>

#include "vmuni.h"

#ifdef TADSHTML_DEBUG
void os_dbg_sys_msg(const textchar_t* msg)
{
    qDebug() << msg;
}
#endif

auto os_get_default_charset() -> oshtml_charset_id_t
{
    // We always assume UTF-8, so the value we return here doesn't actually
    // represent anything.
    return 0;
}

/* Get the next character in a string.
 */
auto os_next_char(const oshtml_charset_id_t /*id*/, const textchar_t* p, const size_t /*len*/)
    -> const textchar_t*
{
    if (p == nullptr) {
        return nullptr;
    }
    // We always use UTF-8, so we only need to figure out how many bytes to
    // skip by looking at the first byte. We don't need to actually iterate
    // over them.
    if ((0x80 & *p) == 0x00) {
        p += 1;
    } else if ((0xE0 & *p) == 0xC0) {
        p += 2;
    } else if ((0xF0 & *p) == 0xE0) {
        p += 3;
    } else if ((0xF8 & *p) == 0xF0) {
        p += 4;
    } else {
        qWarning() << Q_FUNC_INFO << "corrupt UTF-8 sequence";
        p += 1;
    }
    return p;
}

/* Get the previous character in a string.
 */
auto os_prev_char(oshtml_charset_id_t /*id*/, const textchar_t* p, const textchar_t* pstart)
    -> const textchar_t*
{
    if (p == nullptr) {
        return nullptr;
    }
    // Move back one byte.
    --p;
    // Skip continuation bytes. Make sure we don't go past the beginning.
    // We need to iterate over every byte since it's not possible to determine
    // the length of the character without actually looking at the first byte.
    while (p != pstart and (*p & 0xC0) == 0x80) {
        --p;
    }
    return p;
}

/* Determine if the character at the given string position is a word
 * character - i.e., a character that's part of a written word.
 */
auto os_is_word_char(oshtml_charset_id_t id, const textchar_t* p, size_t len) -> int
{
    if (p == nullptr) {
        return false;
    }
    const QString& c = QString::fromUtf8(p, os_next_char(id, p, len) - p);
    if (c.isEmpty()) {
        return false;
    }
    return t3_is_alpha(c.at(0).unicode());
}

/* Get the current system time.
 */
auto os_get_time() -> os_timer_t
{
    using namespace std::chrono;
    // Use an initial zero point instead of just returning now().time_since_epoch(), so that we
    // start counting from 0. This keeps the numbers small, which is important since we implement
    // os_get_sys_clock_ms() in terms of this function. On systems where long int is 32 bits it
    // would overflow.
    static auto zero_point = steady_clock::now();
    return duration_cast<milliseconds>(steady_clock::now() - zero_point).count();
}

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

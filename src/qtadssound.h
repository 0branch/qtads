// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <QElapsedTimer>
#include <QObject>

#include "config.h"
#include "qtimerchrono.h"
#include "tadshtml.h"
#ifndef NO_AUDIO
    #include "Aulib/Stream.h"
#endif

auto initSound() -> bool;

void quitSound();

/* Provides the common code for all three types of digitized sound (WAV,
 * Ogg Vorbis and MP3).
 */
class QTadsSound: public QObject
{
    Q_OBJECT

public:
    enum SoundType
    {
        WAV,
        OGG,
        MPEG,
        MIDI
    };

#ifndef NO_AUDIO
private:
    Aulib::Stream* fAudStream;
    SoundType fType;
    bool fPlaying;
    std::chrono::milliseconds fFadeOut{};
    bool fCrossFade;
    QTimerChrono fFadeOutTimer;
    QElapsedTimer fTimePos;

    // TADS callback to invoke on stop.
    void (*fDone_func)(void*, int repeat_count);

    // CTX to pass to the TADS callback.
    void* fDone_func_ctx;

    // How many times we repeated the sound.
    int fRepeats;

    // How many times should we repeat the sound.
    // 0 means repeat forever.
    int fRepeatsWanted;

    // Total length of the sound in milliseconds.
    std::chrono::milliseconds fLength{};

    // Aulib finish callback.
    void fFinishCallback(Aulib::Stream& strm);

    // Aulib loop callback.
    void fLoopCallback(Aulib::Stream& strm);

    void emitReadyToFadeOut()
    {
        emit readyToFadeOut();
    }

private slots:
    void fDoFadeOut();

    void fPrepareFadeOut();

signals:
    void readyToFadeOut();

public:
    QTadsSound(QObject* parent, Aulib::Stream* stream, SoundType type);
    ~QTadsSound() override;
#endif

public:
    auto startPlaying(
        void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat, int vol,
        int fadeIn, int fadeOut, bool crossFade) -> int;

    void cancelPlaying(bool sync, int fadeOut, bool fadeOutInBg);

    void addCrossFade(int ms);

    static auto createSound(
        const class CHtmlUrl* url, const textchar_t* filename, unsigned long seekpos,
        unsigned long filesize, class CHtmlSysWin* win, SoundType type) -> class CHtmlSysSound*;
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

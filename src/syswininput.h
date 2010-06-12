/* Copyright (C) 2010 Nikos Chantziaras.
 *
 * This file is part of the QTads program.  This program is free software; you
 * can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; see the file COPYING.  If not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef SYSWININPUT_H
#define SYSWININPUT_H

#include "syswin.h"


/* An input-capable CHtmlSysWinQt.
 */
class CHtmlSysWinInputQt: public CHtmlSysWinQt {
	Q_OBJECT
	friend class CHtmlSysWinQt;

  private:
	// Our display widget casted for easier access.
	class QTadsDisplayWidgetInput* fCastDispWidget;

	// We have a finished user input.
	bool fInputReady;

	// We are accepting input.
	bool fAcceptInput;

	// If we're accepting input, should we get a whole line, or a single
	// keypress.
	bool fSingleKeyInput;

	// In single keypress input mode, these store the last pressed key.  Only
	// one of fLastKeyEvent and fLastKeyText can be valid.
	//
	// fLastKeyEvent is used in cases where the user pressed a non-text key,
	// like backspace, space, enter, the up-arrow button, etc.  In that case,
	// fLastKeyEvent contains that key press in form of a Qt::Key and
	// fLastKeyText will be a null QChar.
	//
	// If the user pressed a text key (for example "C", "8" or "!"), then
	// fLastKeyEvent will be zero and fLastKeyText will contain the character
	// that corresponds to the pressed key.
	Qt::Key fLastKeyEvent;
	QChar fLastKeyText;

	// Pending HREF event, if any.
	QString fHrefEvent;

	// The input tag we use to communicate with the base code.
	class CHtmlTagTextInput* fTag;

	// The externally managed input buffer.
	class CHtmlInputBuf* fTadsBuffer;

	void
	fStartLineInput( class CHtmlInputBuf* tadsBuffer, class CHtmlTagTextInput* tag );

	void
	fStartKeypressInput();

  protected:
	virtual void
	resizeEvent( QResizeEvent* event );

	virtual void
	keyPressEvent( QKeyEvent* e );

	void
	singleKeyPressEvent( QKeyEvent* event );

  public:
	CHtmlSysWinInputQt( class CHtmlFormatter* formatter, QWidget* parent );

	virtual
	~CHtmlSysWinInputQt()
	{ }

	// Change the height of the text cursor.
	void
	setCursorHeight( unsigned height );

	void
	processCommand( const textchar_t* cmd, size_t len, int append, int enter, int os_cmd_id );

	// Read a line of input.
	bool
	getInput( class CHtmlInputBuf* tadsBuffer );

	// Uses os_getc_raw() semantics, but with a timeout.
	//
	// If 'timeout' is 0 or negative, then the routine behaves exactly like
	// os_getc_raw().  If 'timeout' is positive, then we only wait for a key
	// for 'timeout' milliseconds.  If the operation times out before a key has
	// been pressed, we return 0 and set 'timedOut' to true.  If a key is
	// pressed before the timeout is reached, we return the same as
	// os_getc_raw() and set 'timedOut' to false.
	//
	// If an HREF events happens while we're waiting for input, -1 is returned.
	// The caller should use the pendingHrefEvent() method to get the HREF
	// event in this case.
	int
	getKeypress( unsigned long timeout = 0, bool useTimeout = false, bool* timedOut = 0 );

	// Return the currently pending HREF event (is there is one.)  This method
	// will clear the event, so subsequent calls will return an empty string.
	QString
	pendingHrefEvent()
	{
		QString ret(this->fHrefEvent);
		this->fHrefEvent.clear();
		return ret;
	}

	//
	// CHtmlSysWin interface implementation.
	//
	virtual void
	set_html_input_color( HTML_color_t clr, int use_default );
};


#endif
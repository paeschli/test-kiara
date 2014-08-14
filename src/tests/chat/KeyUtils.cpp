/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * KeyUtils.cpp
 *
 *  Created on: 02.10.2012
 *      Author: Dmitri Rubinstein
 */

#include "KeyUtils.hpp"
#include <cstdio>

#ifdef _MSC_VER
#include <conio.h>
#else
/* According to POSIX.1-2001 */
#  include <sys/select.h>

/* According to earlier standards */
#  include <sys/time.h>
#  include <sys/types.h>
#  include <unistd.h>

#  include <termios.h>
#endif

namespace KeyUtils
{

#ifdef _MSC_VER
// TODO: Windows specific code
int kbhit()
{
	return _kbhit();
}

int getch()
{
	return _getch();
}

void disableEcho(bool disable)
{

}

#else

// Keyboard Code from
// http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/

void disableEcho(bool disable)
{
    fflush(stdout);
    static bool kbInitialized = false;
    static struct termios newt, oldt;
    if (!kbInitialized)
    {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VMIN] = 1;
        newt.c_cc[VTIME] = 0;

        kbInitialized = true;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, disable ? &newt : &oldt);
}

int kbhit()
{
    // check keyboard
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);

    return FD_ISSET(STDIN_FILENO, &fds);
}

int getch()
{
    return getchar();
}

#endif

} // namespace KeyUtils

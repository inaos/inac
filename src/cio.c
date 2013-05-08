/*
 * Copyright (c) 2013, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the INAOS GmbH nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <libinac/lib.h>
#include "config.h"

#ifdef INA_OS_WIN32
static short int __fg_colors[INA_CIO_COLOR_UNDEFINED + 1];
static short int __bg_colors[INA_CIO_COLOR_UNDEFINED + 1 ];

static void __ina_init_colors(void)
{
    __fg_colors[INA_CIO_COLOR_BLACK]     = 0;
    __fg_colors[INA_CIO_COLOR_BLUE]      = FOREGROUND_BLUE;
    __fg_colors[INA_CIO_COLOR_RED]       = FOREGROUND_RED;
    __fg_colors[INA_CIO_COLOR_MAGENTA]   = FOREGROUND_BLUE | FOREGROUND_RED;
    __fg_colors[INA_CIO_COLOR_GREEN]     = FOREGROUND_GREEN;
    __fg_colors[INA_CIO_COLOR_CYAN]      = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    __fg_colors[INA_CIO_COLOR_YELLOW]    = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
    __fg_colors[INA_CIO_COLOR_WHITE]     = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    __fg_colors[INA_CIO_COLOR_UNDEFINED] = 0;

    __bg_colors[INA_CIO_COLOR_BLACK]     = 0;
    __bg_colors[INA_CIO_COLOR_BLUE]      = BACKGROUND_BLUE;
    __bg_colors[INA_CIO_COLOR_RED]       = BACKGROUND_RED;
    __bg_colors[INA_CIO_COLOR_MAGENTA]   = BACKGROUND_BLUE | BACKGROUND_RED;
    __bg_colors[INA_CIO_COLOR_GREEN]     = BACKGROUND_GREEN;
    __bg_colors[INA_CIO_COLOR_CYAN]      = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
    __bg_colors[INA_CIO_COLOR_YELLOW]    = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
    __bg_colors[INA_CIO_COLOR_WHITE]     = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
    __bg_colors[INA_CIO_COLOR_UNDEFINED] = 0;
}
#else
#define __INA_MAX_CMD_BUFLEN  (32)
#define __INA_LAST_ROW        (25)
#define __INA_LAST_COL        (80)
/* ANSI color codes */
static const char * __CSI = "\033[";
static const char * __cmd_clear = "2J";

static char __cmd[__INA_MAX_CMD_BUFLEN];
static char __fg_colors[INA_CIO_COLOR_UNDEFINED + 1][__INA_MAX_CMD_BUFLEN];
static char __bg_colors[INA_CIO_COLOR_UNDEFINED + 1][__INA_MAX_CMD_BUFLEN];

static void __ina_init_colors(void)
{
    sprintf(__fg_colors[INA_CIO_COLOR_BLACK], "%s%s", __CSI, "30m");
    sprintf(__fg_colors[INA_CIO_COLOR_BLUE], "%s%s", __CSI, "34m");
    sprintf(__fg_colors[INA_CIO_COLOR_RED], "%s%s", __CSI, "31m");
    sprintf(__fg_colors[INA_CIO_COLOR_MAGENTA], "%s%s", __CSI, "35m");
    sprintf(__fg_colors[INA_CIO_COLOR_GREEN], "%s%s", __CSI, "32m");
    sprintf(__fg_colors[INA_CIO_COLOR_CYAN], "%s%s", __CSI, "36m");
    sprintf(__fg_colors[INA_CIO_COLOR_YELLOW], "%s%s", __CSI, "33m");
    sprintf(__fg_colors[INA_CIO_COLOR_WHITE], "%s%s", __CSI, "37m");
    sprintf(__fg_colors[INA_CIO_COLOR_UNDEFINED], "%s%s", __CSI, "30m");

    sprintf(__bg_colors[INA_CIO_COLOR_BLACK], "%s%s", __CSI, "40m");
    sprintf(__bg_colors[INA_CIO_COLOR_BLUE], "%s%s", __CSI, "44m");
    sprintf(__bg_colors[INA_CIO_COLOR_RED], "%s%s", __CSI, "41m");
    sprintf(__bg_colors[INA_CIO_COLOR_MAGENTA], "%s%s", __CSI, "45m");
    sprintf(__bg_colors[INA_CIO_COLOR_GREEN], "%s%s", __CSI, "42m");
    sprintf(__bg_colors[INA_CIO_COLOR_CYAN], "%s%s", __CSI, "46m");
    sprintf(__bg_colors[INA_CIO_COLOR_YELLOW], "%s%s", __CSI, "43m");
    sprintf(__bg_colors[INA_CIO_COLOR_WHITE], "%s%s", __CSI, "47m");
    sprintf(__bg_colors[INA_CIO_COLOR_UNDEFINED], "%s%s", __CSI, "40m");
}
#endif

static ina_cio_attribs_t __attribs;
static int               __initialized = INA_NO;

INA_API(ina_rc_t) ina_cio_init(void)
{
    if (!__initialized) {
        __ina_init_colors();
        __attribs.fg_color = INA_CIO_COLOR_WHITE;
	__attribs.bg_color = INA_CIO_COLOR_BLACK;
	__initialized = INA_YES;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cio_clear(void)
{
       INA_ASSERT(__initialized);

#ifdef INA_OS_WIN32
    COORD pos = { 0, 0 };
    DWORD cars;
    HANDLE hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    if( hStdOut != INVALID_HANDLE_VALUE
        && GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
        dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

        FillConsoleOutputCharacter(
                hStdOut,
                ' ',
                dwConSize,
                pos,
                &cars
            );

        FillConsoleOutputAttribute(
                hStdOut,
                __bg_colors[__attribs.bg_color] | 
		__fg_colors[__attribs.fg_color],
                dwConSize,
                pos,
                &cars
        );
    }
#else
    ina_str_cpy(__cmd, (char*)__CSI);
    ina_str_cat(__cmd, (char*)__cmd_clear);
    printf( "%s", __cmd);
#endif
    return ina_cio_move_to_row_and_col(0, 0);
}

INA_API(ina_rc_t) ina_cio_get_limits(ina_cio_pos_t *pos)
{
    INA_ASSERT(__initialized);
    INA_ASSERT_NOTNULL(pos);

    pos->row = pos->col = 0;

#ifdef INA_OS_WIN2
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    pos->row = info.srWindow.Bottom + 1;
    pos->col = info.srWindow.Right + 1;
#else
    pos->row = __INA_LAST_ROW;
    pos->col = __INA_LAST_COL;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cio_show_cursor(int show) 
{
#ifdef INA_OS_WIN32
    CONSOLE_CURSOR_INFO info;

    info.dwSize = 10;
    info.bVisible = (BOOL) show;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
else
    printf("%s?25%c", CSI, show ?'h':'l');
#endif
    return INA_SUCCESS;
}

/*
 * Set attributes
 */
INA_API(ina_rc_t) ina_cio_set_attribs(const ina_cio_attribs_t *attribs)
{
    INA_ASSERT(__initialized);
    INA_ASSERT_NOTNULL(attribs);

    __attribs.bg_color = attribs->bg_color;
    __attribs.fg_color = attribs->fg_color;

#ifdef INA_OS_WIN32
    SetConsoleTextAttribute(
        GetStdHandle(STD_OUTPUT_HANDLE),
        __bg_colors[__attribs.bg_color] | __fg_colors[__attribs.fg_color]
    );
#else
    printf("%s%s", __fg_colors[__attribs.fg_color],
            __bg_colors[__attribs.bg_color]);
#endif
    return INA_SUCCESS;
}

/*
 * Get attributes
 */
INA_API(ina_rc_t) ina_cio_get_attribs(ina_cio_attribs_t *attribs)
{
    INA_ASSERT(__initialized);
    INA_ASSERT_NOTNULL(attribs);

    attribs->bg_color = __attribs.bg_color;
    attribs->fg_color = __attribs.fg_color;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cio_get_pos(ina_cio_pos_t *pos)
{
    INA_ASSERT(__initialized);
    INA_ASSERT_NOTNULL(pos);

#ifdef SO_WINDOWS
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    pos->row = info.dwCursorPosition.Y;
    pos->col = infoPantalla.dwCursorPosition.X;
#else
    /* FIXME */
    pos->row = 0;
    pos->col = 0;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cio_move_to_pos(const ina_cio_pos_t *pos)
{
    INA_ASSERT(__initialized);
    INA_ASSERT_NOTNULL(pos);
    return ina_cio_move_to_row_and_col(pos->row, pos->col);
}

INA_API(ina_rc_t) ina_cio_move_to_row_and_col(uint8_t row, uint8_t col)
{
    INA_ASSERT(__initialized);
#ifdef INA_OS_WIN32
    COORD pos;

    pos.X = col;
    pos.Y = row;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
#else
    printf( "%s%d;%dH", __CSI, row + 1, col + 1);
#endif
    return INA_SUCCESS;
}

INA_API(int) ina_cio_printf(int8_t row, int8_t col, 
                                    ina_cio_color_t fg_color, 
                                    ina_cio_color_t bg_color, 
                                    const char* fmt, ...)
{
    ina_cio_pos_t pos;
    ina_cio_attribs_t attribs;
    ina_cio_attribs_t new_attribs;
    va_list args;
    int size;

    INA_ASSERT(__initialized);

    ina_cio_get_pos(&pos);

    if (row >= 0)  {
        pos.row = (uint8_t)row;
    }
    if (col >= 0) {
        pos.col = (uint8_t)col;
    }

    ina_cio_get_attribs(&attribs);
    
    if (fg_color != INA_CIO_COLOR_UNDEFINED) {
        new_attribs.fg_color = fg_color;
    } else {
        new_attribs.fg_color = attribs.fg_color;
    }
    if (bg_color != INA_CIO_COLOR_UNDEFINED) {
        new_attribs.bg_color = bg_color;
    } else {
        new_attribs.bg_color = attribs.bg_color;
    }
    ina_cio_set_attribs(&new_attribs);

    va_start(args, fmt);
    size = vprintf(fmt, args);
    va_end(args);
    ina_cio_set_attribs(&attribs);
    return size;
}

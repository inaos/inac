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

static int __ina_get_cursor_pos(ina_cio_pos_t *const pos);

#ifdef INA_OS_WIN32
#include <io.h>

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
    __fg_colors[INA_CIO_COLOR_UNDEFINED] = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;

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
#include <termios.h>
#include <fcntl.h>

#define _isatty isatty
#define _fileno fileno
#define __INA_RD_EOF   (-1)
#define __INA_RD_EIO   (-2)
#define __INA_MAX_CMD_BUFLEN  (32)
#define __INA_LAST_ROW        (25)
#define __INA_LAST_COL        (80)
/* ANSI color codes */
static const char * __CSI = "\033[";
static const char * __cmd_clear = "2J";

static struct termios orig_termios;
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
    sprintf(__fg_colors[INA_CIO_COLOR_UNDEFINED], "%s%s", __CSI, "0m");

    sprintf(__bg_colors[INA_CIO_COLOR_BLACK], "%s%s", __CSI, "40m");
    sprintf(__bg_colors[INA_CIO_COLOR_BLUE], "%s%s", __CSI, "44m");
    sprintf(__bg_colors[INA_CIO_COLOR_RED], "%s%s", __CSI, "41m");
    sprintf(__bg_colors[INA_CIO_COLOR_MAGENTA], "%s%s", __CSI, "45m");
    sprintf(__bg_colors[INA_CIO_COLOR_GREEN], "%s%s", __CSI, "42m");
    sprintf(__bg_colors[INA_CIO_COLOR_CYAN], "%s%s", __CSI, "46m");
    sprintf(__bg_colors[INA_CIO_COLOR_YELLOW], "%s%s", __CSI, "43m");
    sprintf(__bg_colors[INA_CIO_COLOR_WHITE], "%s%s", __CSI, "47m");
    sprintf(__bg_colors[INA_CIO_COLOR_UNDEFINED], "%s%s", __CSI, "0m");
}
#endif

static ina_cio_attribs_t __attribs;
static int               __initialized = INA_NO;

INA_API(ina_rc_t) ina_cio_init(void)
{
    if (__initialized != INA_YES) {
#ifndef INA_OS_WIN32
        struct termios new_termios;

        /* take two copies - one for now, one for later */
        /*tcgetattr(0, &orig_termios);
        memcpy(&new_termios, &orig_termios, sizeof(new_termios));*/

        /* register cleanup handler, and set the new terminal mode */
        /*cfmakeraw(&new_termios);*/
        /*tcsetattr(0, TCSANOW, &new_termios);*/
#endif
        __ina_init_colors();
        __attribs.fg_color = INA_CIO_COLOR_UNDEFINED;
        __attribs.bg_color = INA_CIO_COLOR_UNDEFINED;
	    __initialized = INA_YES;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cio_clear(void)
{
#ifdef INA_OS_WIN32
    COORD pos = { 0, 0 };
    DWORD cars;
    HANDLE hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    INA_ASSERT(__initialized);

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
    INA_ASSERT(__initialized);
    ina_str_cpy(__cmd, (char*)__CSI);
    ina_str_cat(__cmd, (char*)__cmd_clear);
    printf( "%s", __cmd);
#endif
    return ina_cio_move_to_row_and_col(0, 0);
}

INA_API(ina_rc_t) ina_cio_reset(void)
{
    ina_cio_attribs_t attribs = { INA_CIO_COLOR_UNDEFINED, 
                                  INA_CIO_COLOR_UNDEFINED, 
                                  INA_CIO_RESET};
#ifndef INA_OS_WIN32
    /*tcsetattr(0, TCSANOW, &orig_termios);*/
#endif
    return ina_cio_set_attribs(&attribs);
}

    
	
INA_API(ina_rc_t) ina_cio_get_limits(ina_cio_pos_t *pos)
{
#ifdef INA_OS_WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
#endif

    INA_ASSERT(__initialized);
    INA_ASSERT_NOTNULL(pos);
    pos->row = pos->col = 0;

#ifdef INA_OS_WIN32
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
#else
    printf("%s?25%c", __CSI, show ?'h':'l');
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
    __attribs.flags = attribs->flags;
    if (__attribs.flags&INA_CIO_RESET) {
        __attribs.bg_color = INA_CIO_COLOR_UNDEFINED;
        __attribs.fg_color = INA_CIO_COLOR_UNDEFINED;
        __attribs.flags = 0;
    }

#ifdef INA_OS_WIN32
    SetConsoleTextAttribute(
        GetStdHandle(STD_OUTPUT_HANDLE),
        __bg_colors[__attribs.bg_color] | __fg_colors[__attribs.fg_color]
    );
#else
    if (__attribs.fg_color != INA_CIO_COLOR_UNDEFINED || attribs->flags&INA_CIO_RESET) {
        printf("%s", __fg_colors[__attribs.fg_color]);
    }
    if (__attribs.bg_color != INA_CIO_COLOR_UNDEFINED || attribs->flags&INA_CIO_RESET) {
        printf("%s", __bg_colors[__attribs.bg_color]);
    }
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
    attribs->flags = __attribs.flags;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cio_get_pos(ina_cio_pos_t *pos)
{
    INA_ASSERT(__initialized);
    INA_ASSERT_NOTNULL(pos);
    __ina_get_cursor_pos(pos);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_cio_move_to_pos(const ina_cio_pos_t *pos)
{
    INA_ASSERT(__initialized);
    INA_ASSERT_NOTNULL(pos);
    return ina_cio_move_to_row_and_col(pos->row, pos->col);
}

INA_API(ina_rc_t) ina_cio_move_to_row_and_col(int16_t row, int16_t col)
{
#ifdef INA_OS_WIN32
    COORD pos;
#endif    
    INA_ASSERT(__initialized);

    if (col < 0 && row < 0) {
        return INA_SUCCESS;
    }
    if (col < 0 || row < 0) {
        ina_cio_pos_t pos;
        ina_cio_get_pos(&pos);
        if (row < 0) {
            return ina_cio_move_to_row_and_col(pos.row, col);
        }
        return ina_cio_move_to_row_and_col(row, pos.col);
    }
#ifdef INA_OS_WIN32

    pos.X = col;
    pos.Y = row;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
#else
    printf( "%s%d;%dH", __CSI, row + 1, col + 1);
#endif
    return INA_SUCCESS;
}

INA_API(int) ina_cio_printf(int16_t row, int16_t col, 
                                    ina_cio_color_t fg_color, 
                                    ina_cio_color_t bg_color, 
                                    const char* fmt, ...)
{
    ina_cio_pos_t pos;
    ina_cio_attribs_t attribs;
    ina_cio_attribs_t new_attribs;
    va_list args;
    int size;
    int setattribs = INA_NO;
    int setpos = INA_NO;

    INA_ASSERT(__initialized);

    pos.col = 0;
    pos.row = 0;

    if (_isatty(_fileno(stdout))) {
        
        if (row >= 0 && row != pos.row)  {
            pos.row = (uint8_t)row;
            setpos = INA_YES;
        }
        if (col >= 0 && row != pos.row) {
            pos.col = (uint8_t)col;
            setpos = INA_YES;
        }
        if (setpos == INA_YES) {
            ina_cio_move_to_pos(&pos);
        }
        
        ina_cio_get_attribs(&attribs);
    
        if (attribs.fg_color != fg_color) {
            new_attribs.fg_color = fg_color;
            setattribs = INA_YES;
        } else {
            new_attribs.fg_color = INA_CIO_COLOR_UNDEFINED;
        }

        if (attribs.bg_color != bg_color) {
            new_attribs.bg_color = bg_color;
            setattribs = INA_YES;
        } else {
            new_attribs.bg_color = INA_CIO_COLOR_UNDEFINED;
        }
        
        if (setattribs == INA_YES) {
            ina_cio_set_attribs(&new_attribs);
        }
    }
    va_start(args, fmt);
    size = vprintf(fmt, args);
    va_end(args);
    
    if (setattribs == INA_YES) {
       ina_cio_reset();
    }
    return size;
}

#ifdef INA_OS_WIN32
static int
__ina_get_cursor_pos(ina_cio_pos_t *const pos)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    pos->row = info.dwCursorPosition.Y;
    pos->col = info.dwCursorPosition.X;
    return 0;
}
#else
INA_INLINE int rd(const int fd)
{
    unsigned char   buffer[4];
    ssize_t         n;

    while (1) {

        n = read(fd, buffer, 1);
        if (n > (ssize_t)0) {
            return buffer[0];
        } else if (n == (ssize_t)0) {
            return __INA_RD_EOF;
        } else if (n != (ssize_t)-1) {
            return __INA_RD_EIO;
        } else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
            return __INA_RD_EIO;
        }
    }
}

INA_INLINE int wr(const int fd, const char *const data, const size_t bytes)
{
    const char       *head = data;
    const char *const tail = data + bytes;
    ssize_t           n;

    while (head < tail) {
        n = write(fd, head, (size_t)(tail - head));
        if (n > (ssize_t)0) {
            head += n;
        } else if (n != (ssize_t)-1) {
            return EIO;
        } else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
            return errno;
        }
    }
    return 0;
}
static int
__ina_get_cursor_pos(ina_cio_pos_t *const pos)
{
    struct termios  saved, temporary;
    int tty, retval, result, rows, cols, saved_errno;
 
    tty = _fileno(stdout);

    /* Bad tty? */
    if (tty == -1) {
        return ENOTTY;
    }

    saved_errno = errno;

    /* Save current terminal settings. */
    do {
        result = tcgetattr(tty, &saved);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = saved_errno;
        return retval;
    }

    /* Get current terminal settings for basis, too. */
    do {
        result = tcgetattr(tty, &temporary);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = saved_errno;
        return retval;
    }

    /* Disable ICANON, ECHO, and CREAD. */
    temporary.c_lflag &= ~ICANON;
    temporary.c_lflag &= ~ECHO;
    temporary.c_cflag &= ~CREAD;

    /* That loop is only executed once. When broken out,
     * the terminal settings will be restored, and the function
     * will return retval to caller. It's better than goto.
    */
    do {

        /* Set modified settings. */
        do {
            result = tcsetattr(tty, TCSANOW, &temporary);
        } while (result == -1 && errno == EINTR);
        if (result == -1) {
            retval = errno;
            break;
        }

        /* Request cursor coordinates from the terminal. */
        retval = wr(tty, "\033[6n", 4);
        if (retval) {
            break;
        }

        /* Assume coordinate reponse parsing fails. */
        retval = EIO;

        /* Expect an ESC. */
        result = rd(tty);
        if (result != 27) {
            break;
        }

        /* Expect [ after the ESC. */
        result = rd(tty);
        if (result != '[') {
            break;
        }

        /* Parse rows. */
        rows = 0;
        result = rd(tty);
        while (result >= '0' && result <= '9') {
            rows = 10 * rows + result - '0';
            result = rd(tty);
        }

        if (result != ';') {
            break;
        }

        /* Parse cols. */
        cols = 0;
        result = rd(tty);
        while (result >= '0' && result <= '9') {
            cols = 10 * cols + result - '0';
            result = rd(tty);
        }

        if (result != 'R') {
            break;
        }

        /* Success! */
        pos->row = rows;
        pos->col = cols;
        retval = 0;

    } while (0);

    /* Restore saved terminal settings. */
    do {
        result = tcsetattr(tty, TCSANOW, &saved);
    } while (result == -1 && errno == EINTR);
    if (result == -1 && !retval) {
        retval = errno;
    }
    return retval;
}
#endif

#define __INA_CIO_READ_BUFFER_CHUNK_SIZE 128
#ifdef INA_OS_WIN32
static void __ina_cio_w32_read_input(ina_str_t *line, HANDLE hStdin, char **ptr_buffer, 
                                     size_t *buf_cur, size_t *buf_len, int *finished)
{
    INPUT_RECORD *irInBuf;
    DWORD dw_event_count;
    DWORD dw_read = 0;
    DWORD i;
    char *buffer = *ptr_buffer;
                
    GetNumberOfConsoleInputEvents(hStdin, &dw_event_count);
    irInBuf = (INPUT_RECORD*)ina_mem_alloc(sizeof(INPUT_RECORD)*dw_event_count);
    ReadConsoleInput(hStdin, irInBuf, dw_event_count, &dw_read);

    for (i = 0; i < dw_read; i++) {
        if (irInBuf[i].EventType == KEY_EVENT) {
            KEY_EVENT_RECORD ker = irInBuf[i].Event.KeyEvent;
            char cta = ker.uChar.AsciiChar;
            if (ker.bKeyDown == 1 && ( (ker.dwControlKeyState & NUMLOCK_ON) 
                    || (ker.dwControlKeyState & SHIFT_PRESSED) 
                    || (ker.dwControlKeyState & CAPSLOCK_ON)
                    || ker.dwControlKeyState == 0) 
                    && ( (cta >=32 && cta <= 126) || cta == '\r' || cta == '\b') ) {
                WORD rep = ker.wRepeatCount;
                char *cur;
                if (cta == '\b') {
                    WORD z;
                    if ((*buf_cur) > 0) {
                        for (z = 0; z < rep; z++) {
                            buffer[(*buf_cur)--] = ' ';
                            printf("%c%c%c", '\b', ' ', '\b');
                        }
                    }
                }
                else {
                    size_t check_size = __INA_CIO_READ_BUFFER_CHUNK_SIZE+(1*rep)+1;
                    cur = buffer + (*buf_cur)++;
                    /* check if there is space for another char, otherwise extend */
                    if (*buf_cur >= check_size) {
                        buf_len += __INA_CIO_READ_BUFFER_CHUNK_SIZE;
                        *ptr_buffer = (char*)ina_mem_realloc(buffer, *buf_len);
                    }
                    memset(cur, cta, rep);
                    /* this is it we finally have a new line! */
                    if (cta == '\r') {
                        *finished = 1;
                    }
                    printf("%c", cta);
                }
            }
        }
    }

    if (*finished == 1) {
        /* 
         * terminate the string we know that we have space 
         *  because we always account for it when checking the buffer 
         */
        buffer[*buf_cur] = '\0';
        *line = ina_str_fromcstr(buffer);
        ina_mem_free(buffer);
    }
}
static ina_rc_t __ina_cio_read_line(ina_str_t *line, int blocking, char **nb_buf, 
                                    size_t *nb_buf_len, size_t *nb_buf_cur)
{
    ina_rc_t ret = INA_SUCCESS;
    HANDLE hStdin;
    DWORD dw_wait_ret = 0;

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) {
        return ENOTTY;
    }

    dw_wait_ret = WaitForSingleObject(hStdin, 1);

    if (dw_wait_ret == WAIT_ABANDONED || dw_wait_ret == WAIT_FAILED) {
        return INA_EWAIT;
    }

    if (blocking) {
        char *buffer;
        size_t buf_cur = 0;
        size_t buf_len = __INA_CIO_READ_BUFFER_CHUNK_SIZE;
        int finished = 0;
        
        buffer = (char*)ina_mem_alloc(sizeof(char)*__INA_CIO_READ_BUFFER_CHUNK_SIZE);
        
        while (1) {
            if (dw_wait_ret == WAIT_OBJECT_0) {
                __ina_cio_w32_read_input(line, hStdin, &buffer, &buf_cur, &buf_len, &finished);
                if (finished) {
                    break;
                }
            }
            dw_wait_ret = WaitForSingleObject(hStdin, 10);
        }
    }
    else {
        int finished = 0;

        if (dw_wait_ret == WAIT_TIMEOUT) {
            ret = INA_EAGAIN;
        }
        else {
            if (nb_buf == NULL) {
                *nb_buf = (char*)ina_mem_alloc(sizeof(char)*__INA_CIO_READ_BUFFER_CHUNK_SIZE);
            }
            __ina_cio_w32_read_input(line, hStdin, nb_buf, nb_buf_cur, nb_buf_len, &finished);
        }
        if (!finished) {
            ret = INA_EAGAIN;
        }
    }    

    return ret;
}
#else
static ina_rc_t __ina_cio_read_line(ina_str_t *line, int blocking, char **nb_buf, 
                                    size_t *nb_buf_len, size_t *nb_buf_pos)
{
    ina_rc_t rc = INA_SUCCESS;
    char *buf = NULL;
    
    while (1) {
        struct timeval tv = { 0L, 0L };
        int rt = 0;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        rt = select(1, &fds, NULL, NULL, &tv);

        if (!rt && blocking == INA_NO) {
            rc =  INA_EAGAIN;
            break;
        }

        if (rt) {
            int r;
            unsigned char c;
        
            if ((r = read(0, &c, sizeof(c))) < 0) {
                if (blocking == INA_NO) {
                    rc =  INA_EAGAIN;
                    break;
                }
            } 
            if (*nb_buf == NULL) {
                *nb_buf_len = __INA_CIO_READ_BUFFER_CHUNK_SIZE;
                *nb_buf_pos = 0;
                *nb_buf = (char*)ina_mem_alloc(sizeof(char)*__INA_CIO_READ_BUFFER_CHUNK_SIZE);
            }

            if (c == '\n') {
                *line = ina_str_fromcstr(*nb_buf);
                ina_mem_free(*nb_buf);
                *nb_buf = NULL;
                *nb_buf_pos = 0;
                *nb_buf_len = 0;
                break;
            }
	    
	    buf = *nb_buf;
            buf[*nb_buf_pos] = (char)c;
	    *nb_buf_pos += 1;

            if (blocking == INA_NO) {
                rc = INA_EAGAIN;
                break;
            }
        }
    }
    return rc;
} 
#endif
INA_API(ina_rc_t) ina_cio_read_line(ina_str_t *line)
{
    char *buf = NULL;
    size_t buf_len = 0;
    size_t buf_pos = 0;
    return __ina_cio_read_line(line, INA_YES, &buf, &buf_len, &buf_pos);
}

INA_API(ina_rc_t) ina_cio_read_line_non_block(ina_str_t *line, char **buf, 
                                              size_t *buf_len, size_t *buf_pos)
{
    return __ina_cio_read_line(line, INA_NO, buf, buf_len, buf_pos);
}


/*
 * Copyright (c) 2013-2018, INAOS GmbH
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
#ifndef _LIBINAC_CIO_H_
#define _LIBINAC_CIO_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INA_CIO_CURRENT_COL (-1)
#define INA_CIO_CURRENT_ROW (-1)

/* Color codes */
typedef enum ina_cio_colors_e  {
    INA_CIO_COLOR_BLACK, 
    INA_CIO_COLOR_BLUE, 
    INA_CIO_COLOR_RED, 
    INA_CIO_COLOR_MAGENTA,
    INA_CIO_COLOR_GREEN,
    INA_CIO_COLOR_CYAN,
    INA_CIO_COLOR_YELLOW, 
    INA_CIO_COLOR_WHITE,
    INA_CIO_COLOR_UNDEFINED
} ina_cio_color_t;

/* Cursor position */
typedef struct ina_cio_pos_s {
    int16_t row;
    int16_t col;
} ina_cio_pos_t;

/* CIO specials cursor attributes codes */
#define INA_CIO_STTONG (1)
#define INA_CIO_BLINK  (2)
#define INA_CIO_RESET  (4)

/* Cursor attributs */
typedef struct ina_cio_attribs_s {
    ina_cio_color_t bg_color; /* background color */
    ina_cio_color_t fg_color; /* forground color */
    uint8_t flags;
} ina_cio_attribs_t;

/*
 * Initialization. This function is called by ina_init()
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_init(void);

/*
 * Initialization. This function is called by ina_init()
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_destroy(void);

/*
 * Clear screen and reset the cursor in the uppper left corner.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_clear(void);

/*
 * Reset screen attributes.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_reset(void);

/*
 * Get limits in rows and columns.
 *
 * Parameters
 *  pos  Data structure to retrieve the limits
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_get_limits(ina_cio_pos_t *pos);

/*
 * Show or hide the cursor
 *
 * Parameters
 *  show  INA_YES to show the cursor, INA_NO to hide it.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_show_cursor(int show);

/*
 * Set attributes
 *
 * Parameters
 *  attribs   Attributes values
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_set_attribs(const ina_cio_attribs_t *attribs);

/*
 * Get current attributes
 *
 * Parameters
 *  attribs  Attributes values
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_get_attribs(ina_cio_attribs_t *attribs);

/*
 * Get current position.
 *
 * Parameters
 *  pos  Position values
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_get_pos(ina_cio_pos_t *pos);

/*
 * Move cursor to given position.
 *
 * Parameters
 *  pos  Position values
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_move_to_pos(const ina_cio_pos_t *pos);

/*
 * Move cursor to given position. Use INA_CIO_CURRENT_COL or INA_CIO_CURRENT_ROW
 * to let col or row position unchanged.
 *
 * Parameters
 *  row   New row index or INA_CIO_CURRENT_ROW to let row index unchanged.
 *  col   New column index or INA_CIO_CURRENT_COL to let column index unchanged.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_move_to_row_and_col(int16_t row, int16_t col);

/*
 * Print a formatted string to the standard output.
 *
 * Writes the C string pointed by format to the standard output (stdout).
 * If format includes format specifiers (subsequences beginning with %), the
 * additional arguments following format are formatted and inserted in the
 * resulting string replacing their respective specifier. See the printf()
 * function.
 *
 * Parameters
 *  row       Row index or -1 for current row
 *  col       Column index or 1 for current column
 *  fg_color  Foreground color
 *  bg_color  Background color
 *  fmt       C string that contains the text to be written to stdout.It can
 *             optionally contain embedded format specifiers that are replaced
 *             by the values specified in subsequent additional arguments and
 *             formatted as requested.
 *  ...       Depending on the format string, the function
 *             may expect a sequence of additional arguments, each containing a
 *             value to be used to replace a format specifier in the format
 *             string (or a pointer to a storage location, for n). There should
 *             be at least as many of these arguments as the number of values
 *             specified in the format specifiers. Additional arguments are
 *             ignored by the function.
 *
 * Return
 *  On success, the total number of characters written is returned. If a writing
 *  error occurs, negative number is returned.
 */
INA_API(int) ina_cio_printf(int16_t row, int16_t col, 
                                    ina_cio_color_t fg_color, 
                                    ina_cio_color_t bg_color, 
                                    const char* fmt, ...);
/*
 * Read line terminated by '\n'
 * This function blocks until a the enter key is pressed by the user
 * The line must be freed by the caller.
 *
 * Parameters
 *  line  String containing the read line
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_cio_read_line(ina_str_t *line);

/*
 * Non blocking read line terminated by '\n'
 * This function is non-blocking - it will use the buffer to store intermediate
 * line. The buffer is freed once the line is complete - however the line must
 * be freed be the caller.
 *
 * Parameters
 *  line     String containing the read line
 *  buf      Output buffer
 *  buf_len  Size in chars of the output buffer
 *  buf_cur  Current buffer position
 *
 * Return
 *  INA_SUCCESS if line read is completed
 *  INA_EAGAIN line read is not completed
 */
INA_API(ina_rc_t) ina_cio_read_line_non_block(ina_str_t *line, char **buf, 
                                              size_t *buf_len, size_t *buf_cur);

INA_API(ina_rc_t) ina_cio_read_char_non_block(char *ch);

#ifdef __cplusplus
}
#endif
#endif

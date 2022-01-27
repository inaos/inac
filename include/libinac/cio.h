/*
 * Copyright 2013-2020 INAOS GmbH, Thalwil
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _LIBINAC_CIO_H_
#define _LIBINAC_CIO_H_

#ifdef __cplusplus
extern "C" {
#endif
/*
 * # Console Input/Output
 *
 * ## Section1
 * INAC CIO provides minimalistic API which allows the programmer to write
 * text-based user interfaces. It is based on a very simple abstraction.
 * The main idea is viewing terminals as a table of fixed-size cells and input
 * being a stream of structured messages. Would be fair to say that the model
 * is inspired by windows console API. The abstraction itself is not perfect
 * and it may create problems in certain areas.
 */
#include <libinac/lib.h>

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
    int row;
    int col;
} ina_cio_pos_t;

/* CIO specials cursor attributes codes */
#define INA_CIO_STTONG (1)
#define INA_CIO_BLINK  (2)
#define INA_CIO_RESET  (4)

/* Cursor attributes */
typedef struct ina_cio_attribs_s {
    ina_cio_color_t bg_color; /* background color */
    ina_cio_color_t fg_color; /* foreground color */
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
INA_API(void) ina_cio_destroy(void);

/*
 * Clear screen and reset the cursor in the upper left corner.
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
INA_API(ina_rc_t) ina_cio_move_to_row_and_col(int row, int col);

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
INA_API(int) ina_cio_printf(int row, int col,
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

INA_API(ina_rc_t) ina_cio_read_char(char *c);

/*
 * Non blocking read line terminated by '\n'
 * This function is non-blocking  however the line must be freed be the caller.
 *
 * Parameters
 *  line     String containing the read line
 *
 * Return
 *  - INA_SUCCESS if line read is completed
 *  - INA_ERR_TRY_AGAIN line read is not completed
 */
INA_API(ina_rc_t) ina_cio_read_line_non_block(ina_str_t *line);

INA_API(ina_rc_t) ina_cio_read_char_non_block(char *c);

#ifdef __cplusplus
}
#endif
#endif

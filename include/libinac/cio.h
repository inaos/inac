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
#ifndef _LIBINAC_CIO_H_
#define _LIBINAC_CIO_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    uint8_t row;
    uint8_t col;
} ina_cio_pos_t;

/* Cursor attributs */
typedef struct ina_cio_attribs_s {
    ina_cio_color_t bg_color; /* background color */
    ina_cio_color_t fg_color; /* forground color */
    uint8_t strong;
    uint8_t blink;
} ina_cio_attribs_t;

/*
 * Initialization
 */
INA_API(ina_rc_t) ina_cio_init(void);

/*
 * Clear screen
 */
INA_API(ina_rc_t) ina_cio_clear(void);

/*
 * Get limits in rows and columns
 */
INA_API(ina_rc_t) ina_cio_get_limits(ina_cio_pos_t *pos);

/*
 * Show or hide the cursor
 */
INA_API(ina_rc_t) ina_cio_show_cursor(int show);

/*
 * Set attributes
 */
INA_API(ina_rc_t) ina_cio_set_attribs(const ina_cio_attribs_t *attribs);

/*
 * Get attributes
 */
INA_API(ina_rc_t) ina_cio_get_attribs(ina_cio_attribs_t *attribs);

/*
 * Get current position.
 */
INA_API(ina_rc_t) ina_cio_get_pos(ina_cio_pos_t *pos);

/*
 * Move cursor to given position
 */
INA_API(ina_rc_t) ina_cio_move_to_pos(const ina_cio_pos_t *pos);

/*
 * Move cursor to given position
 */
INA_API(ina_rc_t) ina_cio_move_to_row_and_col(uint8_t row, uint8_t col);

/*
 * Print a string ti the standard output
 */
INA_API(int) ina_cio_printf(int8_t row, int8_t col, 
                                    ina_cio_color_t fg_color, 
                                    ina_cio_color_t bg_color, 
                                    const char* fmt, ...);

#ifdef __cplusplus
}
#endif
#endif

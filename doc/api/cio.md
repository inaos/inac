

---

```C
#ifndef _LIBINAC_CIO_H_
#define _LIBINAC_CIO_H_

```

Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.


---

```C
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
```
Color codes

---

```C
typedef struct ina_cio_pos_s {
    int row;
```
Cursor position

---

```C
#define INA_CIO_STTONG (1)
#define INA_CIO_BLINK  (2)
#define INA_CIO_RESET  (4)

```
CIO specials cursor attributes codes

---

```C
typedef struct ina_cio_attribs_s {
    ina_cio_color_t bg_color; /* background color */
    ina_cio_color_t fg_color; /* foreground color */
    uint8_t flags;
```
Cursor attributs

---

```C
INA_API(ina_rc_t) ina_cio_init(void);
```

Initialization. This function is called by ina_init()


**Return**

INA_SUCCESS



---

```C
INA_API(void) ina_cio_destroy(void);
```

Initialization. This function is called by ina_init()


**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_clear(void);
```

Clear screen and reset the cursor in the upper left corner.


**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_reset(void);
```

Reset screen attributes.


**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_get_limits(ina_cio_pos_t *pos);
```

Get limits in rows and columns.


**Parameters**
 - `pos`: Data structure to retrieve the limits



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_show_cursor(int show);
```

Show or hide the cursor


**Parameters**
 - `show`: INA_YES to show the cursor, INA_NO to hide it.



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_set_attribs(const ina_cio_attribs_t *attribs);
```

Set attributes


**Parameters**
 - `attribs`: Attributes values



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_get_attribs(ina_cio_attribs_t *attribs);
```

Get current attributes


**Parameters**
 - `attribs`: Attributes values



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_get_pos(ina_cio_pos_t *pos);
```

Get current position.


**Parameters**
 - `pos`: Position values



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_move_to_pos(const ina_cio_pos_t *pos);
```

Move cursor to given position.


**Parameters**
 - `pos`: Position values



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_move_to_row_and_col(int row, int col);
```

Move cursor to given position. Use INA_CIO_CURRENT_COL or INA_CIO_CURRENT_ROW
to let col or row position unchanged.


**Parameters**
 - `row`: New row index or INA_CIO_CURRENT_ROW to let row index unchanged.
 - `col`: New column index or INA_CIO_CURRENT_COL to let column index unchanged.



**Return**

INA_SUCCESS



---

```C
INA_API(int) ina_cio_printf(int row, int col,
                                    ina_cio_color_t fg_color, 
                                    ina_cio_color_t bg_color, 
                                    const char* fmt, ...);
```

Print a formatted string to the standard output.

Writes the C string pointed by format to the standard output (stdout).
If format includes format specifiers (subsequences beginning with %), the
additional arguments following format are formatted and inserted in the
resulting string replacing their respective specifier. See the printf()
function.


**Parameters**
 - `row`: Row index or -1 for current row
 - `col`: Column index or 1 for current column
 - `fg_color`: Foreground color
 - `bg_color`: Background color
 - `fmt`: C string that contains the text to be written to stdout.It can
optionally contain embedded format specifiers that are replaced
by the values specified in subsequent additional arguments and
formatted as requested.
 - `...`: Depending on the format string, the function
may expect a sequence of additional arguments, each containing a
value to be used to replace a format specifier in the format
string (or a pointer to a storage location, for n). There should
be at least as many of these arguments as the number of values
specified in the format specifiers. Additional arguments are
ignored by the function.



**Return**

On success, the total number of characters written is returned. If a writing
error occurs, negative number is returned.



---

```C
INA_API(ina_rc_t) ina_cio_read_line(ina_str_t *line);
```

Read line terminated by '\n'
This function blocks until a the enter key is pressed by the user
The line must be freed by the caller.


**Parameters**
 - `line`: String containing the read line



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_cio_read_line_non_block(ina_str_t *line, char **buf,
                                              size_t *buf_len, size_t *buf_cur);
```

Non blocking read line terminated by '\n'
This function is non-blocking - it will use the buffer to store intermediate
line. The buffer is freed once the line is complete - however the line must
be freed be the caller.


**Parameters**
 - `line`: String containing the read line
 - `buf`: Output buffer
 - `buf_len`: Size in chars of the output buffer
 - `buf_cur`: Current buffer position



**Return**

INA_SUCCESS if line read is completed
INA_EAGAIN line read is not completed


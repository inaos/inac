/*
 * Copyright INAOS GmbH, Thalwil, 2018-2019. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <stdio.h>
#include <libinac/lib.h>

void inaex_wait_for_line_non_blocking(void)
{
    ina_str_t line = ina_str_new(128);
    ina_cio_attribs_t a;

    ina_cio_printf(5,3,INA_CIO_COLOR_BLUE, INA_CIO_COLOR_UNDEFINED, "3. %s", "Enter line (non blocking) :");
    fflush(stdout);

    ina_cio_get_attribs(&a);
    a.fg_color = INA_CIO_COLOR_GREEN;
    ina_cio_set_attribs(&a);
    fflush(stdout);

    while (INA_FAILED(ina_cio_read_line_non_block(&line))) {
        ina_time_sleep(5);
    }
    ina_cio_printf(6, 3, INA_CIO_COLOR_GREEN, INA_CIO_COLOR_UNDEFINED, "   Entered line: %s", line);
    ina_cio_reset();
    ina_str_free(line);
}

void inaex_wait_for_line_blocking(void)
{
    ina_str_t line = NULL;
    ina_cio_attribs_t a;

    ina_cio_printf(7,3,INA_CIO_COLOR_BLUE, INA_CIO_COLOR_UNDEFINED, "4. %s", "Enter line (blocking):");
    fflush(stdout);

    ina_cio_get_attribs(&a);
    a.fg_color = INA_CIO_COLOR_GREEN;
    ina_cio_set_attribs(&a);
    fflush(stdout);

    ina_cio_read_line(&line);
    ina_cio_printf(8, 3, INA_CIO_COLOR_GREEN, INA_CIO_COLOR_UNDEFINED, "   Entered line: %s", line);
    ina_cio_reset();
    ina_str_free(line);
}

void inaex_wait_char_non_blocking(void)
{
    char c;
    ina_cio_attribs_t a;

    ina_cio_printf(9,3,INA_CIO_COLOR_BLUE, INA_CIO_COLOR_UNDEFINED, "4. %s", "Enter any key (non blocking):");
    fflush(stdout);

    ina_cio_get_attribs(&a);
    a.fg_color = INA_CIO_COLOR_GREEN;
    ina_cio_set_attribs(&a);
    fflush(stdout);

    while (INA_FAILED(ina_cio_read_char_non_block(&c))) {
        ina_time_sleep(5);
    }
    ina_cio_printf(8, 3, INA_CIO_COLOR_GREEN, INA_CIO_COLOR_UNDEFINED, "   Entered char: %c", c);
    ina_cio_reset();
}

int main(int argc,  char** argv)
{
    ina_cio_pos_t p;
    char c;

    if (INA_FAILED(ina_app_init(argc, argv, NULL))) {
        return EXIT_FAILURE;
    }

    /* clear the screen */
    ina_cio_clear();

    printf("%s", "-> should be printed at the upper left corner.\n");

    /* get the limits */
    ina_cio_get_limits(&p);
    printf("-> limits rows=%d, cols=%d\n", p.row, p.col);

    /* print */
    ina_cio_printf(4,3,INA_CIO_COLOR_RED, INA_CIO_COLOR_UNDEFINED, "1. %s", "printed red at 4,3");
    ina_cio_printf(3,3,INA_CIO_COLOR_BLUE, INA_CIO_COLOR_UNDEFINED, "2. %s", "printed blue at 3,3");
    fflush(stdout);

    ina_cio_get_pos(&p);
    printf("<-I'am at %d,%d", p.row, p.col);

    inaex_wait_for_line_non_blocking();
    inaex_wait_for_line_blocking();
    inaex_wait_char_non_blocking();

    ina_cio_get_limits(&p);
    ina_cio_move_to_row_and_col(p.row, 0);
    printf("%s", "-> press enter to exit");
    ina_cio_read_char(&c);

    return EXIT_SUCCESS;
}

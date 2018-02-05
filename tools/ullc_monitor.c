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
 * CAUSED AND ON ANYs THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <libinac/lib.h>


static ina_rc_t umon_draw_monitor(ina_ullc_rb_info_t *rbi) 
{
    int16_t fg = INA_CIO_COLOR_UNDEFINED;
    int16_t bg = INA_CIO_COLOR_UNDEFINED;
    int16_t x = 1;
    int16_t y = 0;
    size_t c = 0;

    INA_ASSERT_NOTNULL(rbi);
    
    ina_cio_printf(x, y++, fg, bg, "Version        : %d", rbi->ring_version);
    ina_cio_printf(x, y++, fg, bg, "Write ops      : %ld", rbi->num_write_op);
    ina_cio_printf(x, y++, fg, bg, "Last writer    : %" INA_INT64_T_FMT, rbi->last_writer);
    ina_cio_printf(x, y++, fg, bg, "Read ops       : %ld", rbi->num_read_op);
    ina_cio_printf(x, y++, fg, bg, "Last reader    : %" INA_INT64_T_FMT, rbi->last_reader);
    ina_cio_printf(x, y++, fg, bg, "Num p          : %d", rbi->num_producers);
    ina_cio_printf(x, y++, fg, bg, "Num p alive    : %ld", rbi->num_consumers_alive);
    ina_cio_printf(x, y++, fg, bg, "Num c          : %d", rbi->num_producers);
    ina_cio_printf(x, y++, fg, bg, "Num c alive    : %ld", rbi->num_consumers_alive);
    ina_cio_printf(x, y++, fg, bg, "Size (bytes)   : %d", rbi->mem_size);
    ina_cio_printf(x, y++, fg, bg, "Slots          : %ld", rbi->num_slots);
    ina_cio_printf(x, y++, fg, bg, "Current slot   : %ld", rbi->current_slot);
    
    y++;
    ina_cio_printf(x, y++, fg, bg, "Producers cursors");
    while (c < rbi->num_producers) {
        ina_cio_printf(x, y++, fg, bg, "Producers %d    : %ld", 
            rbi->p_cursors[c].cursor);        
    }

    y++;
    ina_cio_printf(x, y++, fg, bg, "Consumers cursors");
    while (c < rbi->num_producers) {
        ina_cio_printf(x, y++, fg, bg, "Consumers %d    : %ld", 
            rbi->c_cursors[c].cursor);        
    }

    return INA_SUCCESS;    
}

static ina_rc_t umon_start_monitor(void) 
{
    ina_str_t ring = NULL;
    ina_ullc_rb_info_t rbi;
    
    if (!INA_SUCCEED(ina_opt_get_string("ring", &ring))) {
        return ina_err_get_last_rc();
    }
    
    ina_mem_set(&rbi, 0, sizeof(ina_ullc_rb_info_t));

    while (ina_ullc_get_ring_info(ina_str_cstr(ring), &rbi)) {
        if (!INA_SUCCEED(umon_draw_monitor(&rbi))) {
            return ina_err_get_last_rc();
        }
    }
    return INA_SUCCESS;
}

static ina_rc_t umon_reset(void) 
{
    ina_str_t ring = NULL;

    if (!INA_SUCCEED(ina_opt_get_string("name", &ring))) {
        return ina_err_get_last_rc();
    }

    if (!INA_SUCCEED(ina_ullc_reset_ring(ina_str_cstr(ring)))) {
        printf("Failed to reset ring %s\n", ina_str_cstr(ring)); 
        return ina_err_get_last_rc();
    }
    printf("Ring %s reset\n", ina_str_cstr(ring));
    return INA_SUCCESS;
}

int main(int argc,  char** argv) 
{ 
    INA_OPTS(opt,
        INA_OPT_STRING("n", "name", NULL, "ULLC ring name"),
        INA_OPT_FLAG("r", "reset", "Reset ring"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    
    if (!INA_SUCCEED(ina_opt_isset("name"))) {
        return EXIT_FAILURE;
    }
    
    if (INA_SUCCEED(ina_opt_isset("r"))) {
        if (!INA_SUCCEED(umon_reset())) {
            return EXIT_FAILURE;
        }
    } else {
        if (!INA_SUCCEED(umon_start_monitor())) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

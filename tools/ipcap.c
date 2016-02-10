/*
 * Copyright (c) 2016, INAOS GmbH
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

INA_NO_SERVICE;


int main(int argc,  char** argv) 
{ 
    ina_str_t in_filepath;
    ina_pcap_ctx_t *pcap;
    ina_pcap_open_mode_t open_mode;
    ina_pcap_file_compression_t file_compression;

    INA_OPTS(opt,
        INA_OPT_STRING("f", "file", NULL, "Input file"));

    if (!INA_SUCCEED(ina_app_init(argc, argv, 0, opt))) {
        return EXIT_FAILURE;
    }
    
    ina_opt_get_string("f", &in_filepath);

    if (!INA_SUCCEED(ina_pcap_open(ina_str_cstr(in_filepath),
                                   INA_PCAP_OPEN_MODE_AUTO,
                                   4*1024,
                                   INA_PCAP_FILE_COMPRESSION_DETECT,
                                   &pcap))) {
        printf("Error: Can't open %s.\n", ina_str_cstr(in_filepath));
        return EXIT_FAILURE;
    }

    ina_pcap_get_open_mode(pcap, &open_mode);
    ina_pcap_get_file_compression(pcap, &file_compression);

    printf("File %s\n", ina_str_cstr(in_filepath));
    printf(" Open mode   : %s\n",
           (open_mode==INA_PCAP_OPEN_MODE_FIO?"File":"MMAP"));
    printf(" Compression : %s\n",
           (file_compression==INA_PCAP_FILE_COMPRESSION_GZIP?"GZip":"None"));
    ina_pcap_close(&pcap);
    return EXIT_SUCCESS;
}

/*
 * Copyright (c) 2012-2018, INAOS GmbH
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
#include <DbgHelp.h>
#endif

#define __INA_ERR_MESSAGE_EXTRALEN (20)


/* initialized module, returns always INA_SUCCESS */
static ina_rc_t __ina_init(void);

/* global error state */
static ina_error_t __state;

/* initialization flag */
static int32_t __initialized = 0;


INA_API(ina_rc_t) ina_err_succeed(ina_rc_t rc)
{
    if (INA_SUCCESS == rc || INA_RC_REASON(rc) == 0) {
        return INA_YES;
    }
    return INA_NO;
}


INA_API(ina_rc_t) ina_err_clear(ina_rc_t rc)
{
    size_t k;
    ina_rc_t top;
    ina_rc_t ret;

    if (INA_RC_ID(rc) == 0) {
        return INA_SUCCESS;
    }
    __state.rc = INA_SUCCESS;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_reset(void)
{
    __state.rc = 0;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_fmtmsg(ina_rc_t rc, char* str, size_t len)
{
    size_t k;
    struct tm *tm;
    ina_error_t *error;
    char tmc[30];
    char outstr[2048];

    INA_ASSERT_NOTNULL(str);
    INA_ASSERT(len > 0);

    error = &__state;

    if (len < (strlen(error->msg) +
               strlen(error->file) +
               __INA_ERR_MESSAGE_EXTRALEN)) {
        return INA_ERR_EMSGLEN;
    }

    tm = localtime(&error->ts);

    if (tm && strftime(tmc, sizeof(tmc), "%Y-%m-%d %H:%M:%S", tm) > 0) {
        sprintf(outstr, "%s %s:%d - %s (r:%u,f:%u,m:%u,h:%u,i:%d)",
                                    tmc,
                                    error->file,
                                    error->line,
                                    error->msg,
                                    INA_RC_REASON(error->rc),
                                    INA_RC_OSFN(error->rc),
                                    INA_RC_MOD(error->rc),
                                    INA_RC_HANDLED(error->rc),
                                    INA_RC_ID(error->rc));

        strncpy(str, outstr, len-1);
        return INA_SUCCESS;
    }

    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_err_get_last_error(void)
{
    return __state.rc;
}

INA_API(const char*) ina_err_get_last_errormsg(void)
{
    return ina_err_get_errormsg(__state.rc);
}

INA_API(const char*) ina_err_get_errormsg(ina_rc_t rc)
{  
    if (rc == INA_SUCCESS || INA_RC_ID(rc) == 0) {
        return NULL;
    }
    return __state.msg;
}


INA_API(ina_rc_t) ina_err_backtrace(void *data)
{
#ifndef INA_OS_WIN32
    void *fnptr[30];
    int size;
    int i;

    fprintf(stderr, "%s\n", "**** BACKTRACE START ******");
    size = backtrace(fnptr, 30);
    char** fn = backtrace_symbols(fnptr, size);
    for (i = 0; i < size; i++) {
        if (i > 3) {
            fprintf(stderr, "%s\n", fn[i]);
        }
    }
    free(fn);
    fprintf(stderr, "%s\n", "**** BACKTRACE  END ******");
#else
	#ifdef INA_CPU_X86_64
	#else
		EXCEPTION_POINTERS* pExceptionPointers = (EXCEPTION_POINTERS*)data;
		HANDLE process;
		SYMBOL_INFO *symbol;
		unsigned int i;
		DWORD stack[100];
		unsigned short frames = 0;
		STACKFRAME frame = {0};

		process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);
	 
		/* setup initial stack frame */
		frame.AddrPC.Offset = pExceptionPointers->ContextRecord->Eip;
		frame.AddrPC.Mode = AddrModeFlat;
		frame.AddrStack.Offset = pExceptionPointers->ContextRecord->Esp;
		frame.AddrStack.Mode = AddrModeFlat;
		frame.AddrFrame.Offset = pExceptionPointers->ContextRecord->Ebp;
		frame.AddrFrame.Mode = AddrModeFlat;

		symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	 
		while (StackWalk(IMAGE_FILE_MACHINE_I386,
						 process,
						 GetCurrentThread(),
						 &frame,
						 pExceptionPointers->ContextRecord,
						 0,
						 SymFunctionTableAccess,
						 SymGetModuleBase,
						 0 ) )
		{
			stack[frames++] = frame.AddrPC.Offset;
		}
		
		for (i = 0; i < frames; i++) {
			SymFromAddr(process, stack[i], 0, symbol);
			printf("%i: %s - 0x%I64X\n", frames - i - 1, symbol->Name, symbol->Address);
		}
		
		free(symbol);
		SymCleanup(process);
	#endif
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_err_coredump(void *data) {
#ifndef INA_OS_WIN32
    char cmd[160];
    sprintf(cmd, "echo 'where\ndetach' | gdb -q %d > %s.dump", getpid(), "test");
    if (system(cmd)) {
        return INA_FAILURE;
    } 
#else
    EXCEPTION_POINTERS* pExceptionPointers = (EXCEPTION_POINTERS*)data;
    BOOL dumped;
    char suffix[MAX_PATH];
    char final_name[MAX_PATH];
    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
    SYSTEMTIME t;
    HANDLE hFile;

    strcpy(final_name, ina_app_get_name());
    
    GetSystemTime(&t);
    sprintf(suffix,
        "_%4d%02d%02d_%02d%02d%02d.dmp",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

    strcat(final_name, suffix);

    hFile = CreateFileA(final_name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error ina_err_coredump: Could not create file %s!\n", final_name);
        return INA_FAILURE;
    }

    exceptionInfo.ThreadId = GetCurrentThreadId();
    exceptionInfo.ExceptionPointers = pExceptionPointers;
    exceptionInfo.ClientPointers = FALSE;

    dumped = MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
        pExceptionPointers ? &exceptionInfo : NULL,
        NULL,
        NULL
    );

    CloseHandle(hFile);
#endif
    return INA_SUCCESS;
}

static ina_rc_t
__ina_init(void) 
{
    ++__initialized;
    __state.rc = 0;
    return INA_SUCCESS;
}


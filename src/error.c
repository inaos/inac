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

static INA_TLS(ina_rc_t) __rc = 0;


INA_API(ina_rc_t) ina_err_set_last_rc(ina_rc_t rc)
{
    __rc = rc;
    return __rc;
}

INA_API(ina_rc_t) ina_err_get_last_rc(void)
{
    return __rc;
}

INA_API(ina_rc_t) ina_err_clear()
{
    __rc &= ~(INA_ERR_ERROR);
    return __rc;
}

static const char* __ina_get_noun(int id) {
    switch (id) {
        case INA_NN_BLANK: return "";
        default:  return "??";

        case INA_NN_ACCESS: return "ACCESS";
        case INA_NN_ARRAY: return "ACCOUNT";
        case INA_NN_ADMINISTRATOR: return "ADMINISTRATOR";
        case INA_NN_API: return "API";
        case INA_NN_APPLICATION: return "APPLICATION";
        case INA_NN_ARGUMENT: return "ARGUMENT";
        case INA_NN_AUTHENTICATION: return "AUTHENTICATION";
        case INA_NN_BINARY: return "BINARY";
        case INA_NN_BLOB: return "BLOB";
        case INA_NN_BROADCAST: return "BROADCAST";
        case INA_NN_CLIENT: return "CLIENT";
        case INA_NN_CLOUD: return "CLOUD";
        case INA_NN_CODE: return "CODE";
        case INA_NN_COMMIT: return "COMMIT";
        case INA_NN_COMPILATION: return "COMPILATION";
        case INA_NN_COMPILER: return "COMPILER";
        case INA_NN_COMPRESSION: return "COMPRESSION";
        case INA_NN_CONSOLE: return "CONSOLE";
        case INA_NN_DAEMON: return "DAEMON";
        case INA_NN_DATA: return "DATA";
        case INA_NN_DEPENDENCY: return "DEPENDENCY";
        case INA_NN_DESCRIPTOR: return "DESCRIPTOR";
        case INA_NN_DEVICE: return "DEVICE";
        case INA_NN_DIRECTORY: return "DIRECTORY";
        case INA_NN_DISK: return "DISK";
        case INA_NN_DLL: return "DLL";
        case INA_NN_DOMAIN: return "DOMAIN";
        case INA_NN_DOWNLOAD: return "DOWNLOAD";
        case INA_NN_DRIVER: return "DRIVER";
        case INA_NN_EDITOR: return "EDITOR";
        case INA_NN_ENDPOINT: return "ENDPOINT";
        case INA_NN_ENGINE: return "ENGINE";
        case INA_NN_EVALUATION: return "EVALUATION";
        case INA_NN_EVENT: return "EVENT";
        case INA_NN_EXCEPTION: return "EXCEPTION";
        case INA_NN_EXPECTATION: return "EXPECTATION";
        case INA_NN_FETCH: return "FETCH";
        case INA_NN_FILE: return "FILE";
        case INA_NN_FLOAT: return "FLOAT";
        case INA_NN_FOLDER: return "FOLDER";
        case INA_NN_FORMAT: return "FORMAT";
        case INA_NN_FUNCTION: return "FUNCTION";
        case INA_NN_GATEWAY: return "GATEWAY";
        case INA_NN_GROUP: return "GROUP";
        case INA_NN_HANDLE: return "HANDLE";
        case INA_NN_HARDWARE: return "HARDWARE";
        case INA_NN_HEADER: return "HEADER";
        case INA_NN_HOST: return "HOST";
        case INA_NN_IDENTIFIER: return "IDENTIFIER";
        case INA_NN_INDEX: return "INDEX";
        case INA_NN_INPUT: return "INPUT";
        case INA_NN_INTEGER: return "INTEGER";
        case INA_NN_INTERFACE: return "INTERFACE";
        case INA_NN_INTERVAL: return "INTERVAL";
        case INA_NN_IO: return "IO";
        case INA_NN_KEYBOARD: return "KEYBOARD";
        case INA_NN_LENGTH: return "LENGTH";
        case INA_NN_LEVEL: return "LEVEL";
        case INA_NN_LIBRARY: return "LIBRARY";
        case INA_NN_LIMIT: return "LIMIT";
        case INA_NN_LINK: return "LINK";
        case INA_NN_LINKAGE: return "LINKAGE";
        case INA_NN_LINKER: return "LINKER";
        case INA_NN_LOCATION: return "LOCATION";
        case INA_NN_LOGIN: return "LOGIN";
        case INA_NN_LOOP: return "LOOP";
        case INA_NN_MACHINE: return "MACHINE";
        case INA_NN_MEDIA: return "MEDIA";
        case INA_NN_MEMORY: return "MEMORY";
        case INA_NN_MESSAGE: return "MESSAGE";
        case INA_NN_METHOD: return "METHOD";
        case INA_NN_MODULE: return "MODULE";
        case INA_NN_MONITOR: return "MONITOR";
        case INA_NN_NETWORK: return "NETWORK";
        case INA_NN_NODE: return "NODE";
        case INA_NN_NOTHING: return "NOTHING";
        case INA_NN_NUMBER: return "NUMBER";
        case INA_NN_OBJECT: return "OBJECT";
        case INA_NN_OPERATION: return "OPERATION";
        case INA_NN_OPERATOR: return "OPERATOR";
        case INA_NN_PACKAGE: return "PACKAGE";
        case INA_NN_PASSWORD: return "PASSWORD";
        case INA_NN_PATH: return "PATH";
        case INA_NN_PEER: return "PEER";
        case INA_NN_PERMISSION: return "PERMISSION";
        case INA_NN_PLATFORM: return "PLATFORM";
        case INA_NN_POSITION: return "POSITION";
        case INA_NN_PROFILER: return "PROFILER";
        case INA_NN_PROTOCOL: return "PROTOCOL";
        case INA_NN_PROXY: return "PROXY";
        case INA_NN_QUERY: return "QUERY";
        case INA_NN_RANGE: return "RANGE";
        case INA_NN_RATIO: return "RATIO";
        case INA_NN_RECORD: return "RECORD";
        case INA_NN_REPOSITORY: return "REPOSITORY";
        case INA_NN_REQUEST: return "REQUEST";
        case INA_NN_RESOURCE: return "RESOURCE";
        case INA_NN_REVISION: return "REVISION";
        case INA_NN_ROUTE: return "ROUTE";
        case INA_NN_RUNTIME: return "RUNTIME";
        case INA_NN_SCALE: return "SCALE";
        case INA_NN_SCREEN: return "SCREEN";
        case INA_NN_SCRIPT: return "SCRIPT";
        case INA_NN_SEQUENCE: return "SEQUENCE";
        case INA_NN_SERIALIZATION: return "SERIALIZATION";
        case INA_NN_SERVER: return "SERVER";
        case INA_NN_SERVICE: return "SERVICE";
        case INA_NN_SIZE: return "SIZE";
        case INA_NN_SOFTWARE: return "SOFTWARE";
        case INA_NN_SOURCE: return "SOURCE";
        case INA_NN_SPACE: return "SPACE";
        case INA_NN_STACK: return "STACK";
        case INA_NN_STACKTRACE: return "STACKTRACE";
        case INA_NN_STREAM: return "STREAM";
        case INA_NN_STREAMING: return "STREAMING";
        case INA_NN_STRING: return "STRING";
        case INA_NN_STRUCT: return "STRUCT";
        case INA_NN_SUBSYSTEM: return "SUBSYSTEM";
        case INA_NN_SYSTEM: return "SYSTEM";
        case INA_NN_TEXT: return "TEXT";
        case INA_NN_TIME: return "TIME";
        case INA_NN_TRANSLATION: return "TRANSLATION";
        case INA_NN_TRANSPORT: return "TRANSPORT";
        case INA_NN_TRIGGER: return "TRIGGER";
        case INA_NN_TYPE: return "TYPE";
        case INA_NN_UPGRADE: return "UPGRADE";
        case INA_NN_UPLOAD: return "UPLOAD";
        case INA_NN_USER: return "USER";
        case INA_NN_USERNAME: return "USERNAME";
        case INA_NN_VALUE: return "VALUE";
        case INA_NN_VERSION: return "VERSION";
        case INA_NN_DECOMPRESSION: return "DECOMPRESSION";
        case INA_NM_STATE: return "STATE";
        case INA_NN_DUMP: return "DUMP";
        case INA_NN_CHAR: return "CHAR";
        case INA_NN_CONFIGURATION: return "CONFIGURATION";
        case INA_NN_SECTION: return "SECTION";
        case INA_NN_KEY: return "KEY";
        case INA_NN_ENUMERATION: return "ENUMERATION";
        case INA_NN_READ: return "READ";
        case INA_NN_WRITE: return "WRITE";
        case INA_NN_OPTION: return "OPTION";
        case INA_NN_BUFFER: return "BUFFER";
    }
}

INA_API(const char*) ina_err_strerror(ina_rc_t rc, char buf[INA_ERR_MSGLEN])
{
    const char *neg = "", *adj = "";
    char noun[256-48];

    if (INA_SUCCEED(rc)) {
        return (buf[0] = '\0', buf);
    }
    strcpy (noun, __ina_get_noun(rc & 0xFFFF));

    if (rc & ( 1LL << INA_RC_BIT_N )) {
        neg = "NOT";
    }

    switch (rc & ( 0xFFLL << INA_RC_BIT_A ) ) {
        default: break;
        case INA_ERR_A: adj = "A";break;
        case INA_ERR_ACK: adj = "ACK";break;
        case INA_ERR_ACTIVE: adj = "ACTIVE"; break;
        case INA_ERR_ALIGNED: adj = "ALIGNED"; break;
        case INA_ERR_ALLOWED: adj = "ALLOWED"; break;
        case INA_ERR_ASSIGNED: adj = "ASSIGNED";break;
        case INA_ERR_ATTACHED: adj = "ATTACHED"; break;
        case INA_ERR_ATTEMPTED: adj = "ATTEMPTED"; break;
        case INA_ERR_AUTHORIZED: adj = "AUTHORIZED"; break;
        case INA_ERR_AVAILABLE: adj = "AVAILABLE"; break;
        case INA_ERR_BAD: adj = "BAD"; break;
        case INA_ERR_BLOCKED: adj = "BLOCKED"; break;
        case INA_ERR_BROKEN: adj = "BROKEN"; break;
        case INA_ERR_BUILT: adj = "BUILT"; break;
        case INA_ERR_BUSY: adj = "BUSY"; break;
        case INA_ERR_CLOSED: adj = "CLOSED"; break;
        case INA_ERR_COMPILED: adj = "COMPILED"; break;
        case INA_ERR_COMPLETE: adj = "COMPLETE"; break;
        case INA_ERR_CONFLICTED: adj = "CONFLICTED"; break;
        case INA_ERR_CONNECTED: adj = "CONNECTED"; break;
        case INA_ERR_CONSTRUCTED: adj = "CONSTRUCTED"; break;
        case INA_ERR_CREATED: adj = "CREATED"; break;
        case INA_ERR_DEFINED: adj = "DEFINED"; break;
        case INA_ERR_DENIED: adj = "DENIED";break;
        case INA_ERR_DESTRUCTED: adj = "DESTRUCTED"; break;
        case INA_ERR_DETACHED: adj = "DETACHED"; break;
        case INA_ERR_DETECTED: adj = "DETECTED"; break;
        case INA_ERR_DOWN: adj = "DOWN"; break;
        case INA_ERR_DOWNLOADED: adj = "DOWNLOADED"; break;
        case INA_ERR_EMPTY: adj = "EMPTY"; break;
        case INA_ERR_ENHANCED: adj = "ENHANCED"; break;
        case INA_ERR_ENOUGH: adj = "ENOUGH"; break;
        case INA_ERR_EXCEEDED: adj = "EXCEEDED"; break;
        case INA_ERR_EXCHANGED: adj = "EXCHANGED"; break;
        case INA_ERR_EXECUTABLE: adj = "EXECUTABLE"; break;
        case INA_ERR_EXISTS: adj = "EXISTS"; break;
        case INA_ERR_EXPIRED: adj = "EXPIRED"; break;
        case INA_ERR_EXTENDED: adj = "EXTENDED"; break;
        case INA_ERR_FAILED: adj = "FAILED"; break;
        case INA_ERR_FALSE: adj = "FALSE"; break;
        case INA_ERR_FATAL: adj = "FATAL"; break;
        case INA_ERR_FORBIDDEN: adj = "FORBIDDEN"; break;
        case INA_ERR_FORMATTED: adj = "FORMATTED"; break;
        case INA_ERR_FOUND: adj = "FOUND"; break;
        case INA_ERR_FULL: adj = "FULL"; break;
        case INA_ERR_GONE: adj = "GONE"; break;
        case INA_ERR_GOOD: adj = "GOOD"; break;
        case INA_ERR_HALTED: adj = "HALTED"; break;
        case INA_ERR_HOLD: adj = "HOLD"; break;
        case INA_ERR_IDLE: adj = "IDLE"; break;
        case INA_ERR_ILLEGAL: adj = "ILLEGAL"; break;
        case INA_ERR_IMPLEMENTED: adj = "IMPLEMENTED"; break;
        case INA_ERR_IN_PROGRESS: adj = "IN PROGRESS"; break;
        case INA_ERR_IN_USE: adj = "IN USE"; break;
        case INA_ERR_INITIALIZED: adj = "INITIALIZED"; break;
        case INA_ERR_INSTALLED: adj = "INSTALLED"; break;
        case INA_ERR_INTERRUPTED: adj = "INTERRUPTED"; break;
        case INA_ERR_KNOWN: adj = "KNOWN"; break;
        case INA_ERR_LINKED: adj = "LINKED"; break;
        case INA_ERR_LOADED: adj = "LOADED"; break;
        case INA_ERR_LOCAL: adj = "LOCAL"; break;
        case INA_ERR_LOCKED: adj = "LOCKED"; break;
        case INA_ERR_LOOPED: adj = "LOOPED"; break;
        case INA_ERR_LOST: adj = "LOST"; break;
        case INA_ERR_MISSING: adj = "MISSING"; break;
        case INA_ERR_MOUNTED: adj = "MOUNTED"; break;
        case INA_ERR_NEEDED: adj = "NEEDED"; break;
        case INA_ERR_NO: adj = "NO"; break;
        case INA_ERR_NO_SUCH: adj = "NO SUCH"; break;
        case INA_ERR_OFF: adj = "OFF"; break;
        case INA_ERR_ON: adj = "ON"; break;
        case INA_ERR_ONLINE: adj = "ONLINE"; break;
        case INA_ERR_OPEN: adj = "OPEN"; break;
        case INA_ERR_ORDERED: adj = "ORDERED"; break;
        case INA_ERR_OUT_OF: adj = "OUT OF";break;
        case INA_ERR_OUT_OF_RANGE: adj = "OUT OF RANGE"; break;
        case INA_ERR_OVERFLOW: adj = "OVERFLOW"; break;
        case INA_ERR_PADDED: adj = "PADDED"; break;
        case INA_ERR_PERMITTED: adj = "PERMITTED"; break;
        case INA_ERR_PROCESSABLE: adj = "PROCESSABLE"; break;
        case INA_ERR_PROVIDED: adj = "PROVIDED"; break;
        case INA_ERR_REACHABLE: adj = "REACHABLE"; break;
        case INA_ERR_READABLE: adj = "READABLE"; break;
        case INA_ERR_RECEIVED: adj = "RECEIVED"; break;
        case INA_ERR_REFUSED: adj = "REFUSED"; break;
        case INA_ERR_REGISTERED: adj = "REGISTERED"; break;
        case INA_ERR_REJECTED: adj = "REJECTED"; break;
        case INA_ERR_RELEASED: adj = "RELEASED"; break;
        case INA_ERR_REMOTE: adj = "REMOTE"; break;
        case INA_ERR_RENDERABLE: adj = "RENDERABLE"; break;
        case INA_ERR_RESERVED: adj = "RESERVED"; break;
        case INA_ERR_RESET: adj = "RESET"; break;
        case INA_ERR_RESPONDING: adj = "RESPONDING"; break;
        case INA_ERR_RETRIED: adj = "RETRIED"; break;
        case INA_ERR_RIGHT: adj = "RIGHT"; break;
        case INA_ERR_RUNNING: adj = "RUNNING"; break;
        case INA_ERR_SENT: adj = "SENT"; break;
        case INA_ERR_SPECIFIED: adj = "SPECIFIED"; break;
        case INA_ERR_STALLED: adj = "STALLED"; break;
        case INA_ERR_STOPPED: adj = "STOPPED"; break;
        case INA_ERR_SUCEEDED: adj = "SUCEEDED"; break;
        case INA_ERR_SUITABLE: adj = "SUITABLE"; break;
        case INA_ERR_SUPPORTED: adj = "SUPPORTED"; break;
        case INA_ERR_SYNCHRONIZED: adj = "SYNCHRONIZED"; break;
        case INA_ERR_TERMINATED: adj = "TERMINATED"; break;
        case INA_ERR_THROWN: adj = "THROWN"; break;
        case INA_ERR_TIMED_OUT: adj = "TIMED OUT"; break;
        case INA_ERR_TOO_COMPLEX: adj = "TOO COMPLEX"; break;
        case INA_ERR_TOO_FEW: adj = "TOO FEW"; break;
        case INA_ERR_TOO_LARGE: adj = "TOO LARGE"; break;
        case INA_ERR_TOO_LONG: adj = "TOO LONG";break;
        case INA_ERR_TOO_MANY: adj = "TOO MANY"; break;
        case INA_ERR_TOO_MUCH: adj = "TOO MUCH"; break;
        case INA_ERR_TOO_SIMPLE: adj = "TOO SIMPLE"; break;
        case INA_ERR_TOO_SMALL: adj = "TOO SMALL"; break;
        case INA_ERR_TRIGGERED: adj = "TRIGGERED";break;
        case INA_ERR_TRUE: adj = "TRUE"; break;
        case INA_ERR_UNIQUE: adj = "UNIQUE"; break;
        case INA_ERR_UP: adj = "UP"; break;
        case INA_ERR_UPDATED: adj = "UPDATED"; break;
        case INA_ERR_UPGRADED: adj = "UPGRADED"; break;
        case INA_ERR_UPLOADED: adj = "UPLOADED"; break;
        case INA_ERR_USED: adj = "USED"; break;
        case INA_ERR_VALID: adj = "VALID"; break;
        case INA_ERR_WORKING: adj = "WORKING"; break;
        case INA_ERR_WRITABLE: adj = "WRITABLE"; break;
        case INA_ERR_WRONG: adj = "WRONG"; break;
        case INA_ERR_END_OF: adj = "END OF";
    };

    {
        const char *common[] = { noun, neg, adj };
        const char *special[] = { neg, adj, noun };
        const char **use = common;
        ina_rc_t type = rc & (0x1FFLL << INA_RC_BIT_A);

        if ((type == INA_ERR_A) || (type == INA_ERR_NOT_A) ||
            (type == INA_ERR_NO) || (type == INA_ERR_NO_SUCH) ||
            (type == INA_ERR_ENOUGH) || (type == INA_ERR_NOT_ENOUGH)) {
            use = special;
        }
        strcpy(buf, (use)[0]);
        strcat(buf, (use)[0][0] ? " " : "");
        strcat(buf, (use)[1]);
        strcat(buf, (use)[1][0] ? " " : "");
        strcat(buf, (use)[2]);
    }
    sprintf(buf, "%s ; error=%d,api=%d,rev=%d,line=%d,neg=%d,attr=%d,noun=%d",
            buf,
            INA_RC_E(rc),
            INA_RC_V(rc),
            INA_RC_R(rc),
            INA_RC_L(rc),
            INA_RC_N(rc),
            INA_RC_A(rc),
            INA_RC_U(rc));
    return (buf[255] = '\0', buf);
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
        return INA_ERROR(INA_NN_DUMP|INA_ERR_FAILED);
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


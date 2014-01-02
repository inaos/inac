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

struct ina_service_ctx_s {
    ina_service_mode_t mode;
    ina_service_descriptor_t *descriptor;
#ifdef INA_OS_WIN32
    HANDLE hmutex;
    HANDLE main_thread;
#else
#endif
};

static ina_service_ctx_t *__ina_service_context = NULL;

#ifdef INA_OS_WIN32
static SERVICE_STATUS __ina_service_service_status;
static SERVICE_STATUS_HANDLE __ina_service_status_handle = 0;
static HANDLE __ina_service_stop_service_event = 0;
static ina_service_descriptor_t *__ina_service_descriptor = NULL;

static DWORD __stdcall __ina_service_start_wrapper(LPVOID data)
{
    INA_ASSERT_NOTNULL(__ina_service_descriptor->run_func);
    return __ina_service_descriptor->run_func(__ina_service_descriptor->user_data);
}
static void WINAPI ServiceControlHandler( DWORD controlCode )
{
	switch (controlCode) {
		case SERVICE_CONTROL_INTERROGATE:
			break;
		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
			__ina_service_service_status.dwCurrentState = SERVICE_STOP_PENDING;
			SetServiceStatus(__ina_service_status_handle, &__ina_service_service_status);
			SetEvent(__ina_service_stop_service_event);
			return;
		case SERVICE_CONTROL_PAUSE:
			break;
		case SERVICE_CONTROL_CONTINUE:
			break;
		default:
			if (controlCode >= 128 && controlCode <= 255)
				/* user defined control code */
				break;
			else
				/* unrecognised control code */
				break;
	}
	SetServiceStatus(__ina_service_status_handle, &__ina_service_service_status);
}
static void WINAPI ServiceMain(DWORD argc, TCHAR* argv[])
{
    INA_ASSERT_NOTNULL(__ina_service_descriptor);

	/* initialise service status */
	__ina_service_service_status.dwServiceType = SERVICE_WIN32;
	__ina_service_service_status.dwCurrentState = SERVICE_STOPPED;
	__ina_service_service_status.dwControlsAccepted = 0;
	__ina_service_service_status.dwWin32ExitCode = NO_ERROR;
	__ina_service_service_status.dwServiceSpecificExitCode = NO_ERROR;
	__ina_service_service_status.dwCheckPoint = 0;
	__ina_service_service_status.dwWaitHint = 0;

    __ina_service_status_handle = RegisterServiceCtrlHandler(ina_str_cstr(__ina_service_descriptor->name), ServiceControlHandler);

	if (__ina_service_status_handle) {
        HANDLE thread_handle = INVALID_HANDLE_VALUE;
        DWORD thread_exit_code = 0;

		/* service is starting */
		__ina_service_service_status.dwCurrentState = SERVICE_START_PENDING;
		SetServiceStatus(__ina_service_status_handle, &__ina_service_service_status);

		/* do initialisation here */
        thread_handle = CreateThread(NULL, 0, __ina_service_start_wrapper, NULL, 0, NULL);
        INA_ASSERT_NOTEQUAL(INVALID_HANDLE_VALUE, thread_handle);
        __ina_service_stop_service_event = CreateEvent(0, FALSE, FALSE, 0);
        Sleep(1);
        GetExitCodeThread(thread_handle, &thread_exit_code);
        if (thread_exit_code != STILL_ACTIVE) {
            __ina_service_service_status.dwCurrentState = SERVICE_STOPPED;
            __ina_service_service_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
            __ina_service_service_status.dwServiceSpecificExitCode = thread_exit_code;
            SetServiceStatus(__ina_service_status_handle, &__ina_service_service_status);
            return;
        }

		/* running */
		__ina_service_service_status.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		__ina_service_service_status.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus(__ina_service_status_handle, &__ina_service_service_status);

        /* Wait for the stop-event to trigger */
        WaitForSingleObject(__ina_service_stop_service_event, INFINITE);

        /* we received a stop-event now execute the shutdown proc and join the thread */
        INA_ASSERT_NOTNULL(__ina_service_descriptor->shutdown_func);
        __ina_service_descriptor->shutdown_func(__ina_service_descriptor->user_data);
        WaitForSingleObject(thread_handle, INFINITE);

		/* service was stopped */
		__ina_service_service_status.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus( __ina_service_status_handle, &__ina_service_service_status);

		/* do cleanup here */
		CloseHandle(__ina_service_stop_service_event);
		__ina_service_stop_service_event = 0;

		/* service is now stopped */
		__ina_service_service_status.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		__ina_service_service_status.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(__ina_service_status_handle, &__ina_service_service_status);
	}
}
static ina_rc_t __ina_service_win_install(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    DWORD start_type;
    SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
    LPCSTR username = NULL;
    LPCSTR password = NULL;

    INA_ASSERT_NOTNULL(descriptor);

    if (ina_str_len(descriptor->name) > 255) {
        return INA_SERVICE_EMAXLEN;
    }
    if (ina_str_len(descriptor->display_name) > 255) {
        return INA_SERVICE_EMAXLEN;
    }
    if (descriptor->startup == INA_SERVICE_STARTUP_TYPE_AUTO) {
        start_type = SERVICE_AUTO_START;
    }
    else if (descriptor->startup == INA_SERVICE_STARTUP_TYPE_MANUAL) {
        start_type = SERVICE_DEMAND_START;
    }
    else {
        return INA_SERVICE_EUST;
    }
    if (descriptor->username != NULL) {
        username = ina_str_cstr(descriptor->username);
    }
    if (descriptor->username != NULL) {
        password = ina_str_cstr(descriptor->password);
    }

	if (serviceControlManager) {
        char path[_MAX_PATH + 1];
		if (GetModuleFileName(0, path, sizeof(path)/sizeof(path[0])) > 0 ) {
            SC_HANDLE service;
            if (descriptor->startup_args != NULL) {
                strcat(path, " ");
                strcat(path, ina_str_cstr(descriptor->startup_args));
            }
            service = CreateService(serviceControlManager,
                ina_str_cstr(descriptor->name), ina_str_cstr(descriptor->display_name),
                SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                start_type, SERVICE_ERROR_NORMAL, path,
                0, 0, 0, username, password);

			if (service) {
                SERVICE_DESCRIPTION svc_desc;
                svc_desc.lpDescription = (LPSTR)ina_str_cstr(descriptor->short_description);
                ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &svc_desc);
                CloseServiceHandle(service);
            }
            else {
                CloseServiceHandle(serviceControlManager);
                return INA_SERVICE_ECAPI;
            }
		}
		CloseServiceHandle(serviceControlManager);
	}
    else {
        return INA_SERVICE_ESCM;
    }

    return INA_SUCCESS;
}
static ina_rc_t __ina_service_win_uninstall(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);

    INA_ASSERT_NOTNULL(descriptor);

	if (serviceControlManager) {
        SC_HANDLE service = OpenService(serviceControlManager,
            ina_str_cstr(descriptor->name), SERVICE_QUERY_STATUS | DELETE);
		if (service) {
			SERVICE_STATUS serviceStatus;
			if (QueryServiceStatus(service, &serviceStatus)) {
				if (serviceStatus.dwCurrentState == SERVICE_STOPPED) {
					DeleteService(service);
                }
                else {
                    CloseServiceHandle(service);
                    return INA_SERVICE_ERUNNING;
                }
			}
            else {
                CloseServiceHandle(service);
                return INA_SERVICE_EQRYS;
            }
			CloseServiceHandle(service);
		}
        else {
            CloseServiceHandle(serviceControlManager);
            return INA_SERVICE_ESVCNF;
        }
		CloseServiceHandle(serviceControlManager);
	}
    else {
        return INA_SERVICE_ESCM;
    }
    return INA_SUCCESS;
}
static ina_rc_t __ina_service_win_setandcheck_mutex(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    ina_str_t mutex_name = ina_str_new_fromcstr("/ina_service_mutex_");
    ina_str_cat(mutex_name, descriptor->name);
    if (descriptor->exclusive_flag) {
        HANDLE hmutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, ina_str_cstr(mutex_name));
        if (hmutex != NULL) {
            CloseHandle(hmutex);
            return INA_SERVICE_EEXCL;
        }
        ctx->hmutex = CreateMutex(NULL, TRUE, ina_str_cstr(mutex_name));
        if (ctx->hmutex == INVALID_HANDLE_VALUE) {
            return INA_SERVICE_EMINIT;
        }
    }
    return INA_SUCCESS;
}
static ina_rc_t __ina_service_win_run(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    SERVICE_TABLE_ENTRY serviceTable[] = {
        { (LPSTR)ina_str_cstr(descriptor->name), ServiceMain},
		{ 0, 0 }
	};
    BOOL success;

    __ina_service_descriptor = descriptor;

    if (!INA_SUCCEED(__ina_service_win_setandcheck_mutex(ctx, descriptor))) {
        return INA_ERR_PUSH_LAST;
    }
    
    success = StartServiceCtrlDispatcher(serviceTable);
    if (!success) {
    	return INA_SERVICE_ESDIS;
    }

    return INA_SUCCESS;
}
static ina_rc_t __ina_service_win_console(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    if (!INA_SUCCEED(__ina_service_win_setandcheck_mutex(ctx, descriptor))) {
        return INA_ERR_PUSH_LAST;
    }

    if (!INA_SUCCEED(descriptor->run_func(descriptor->user_data))) {
        return INA_ERR_PUSH_LAST;
    }

    return INA_SUCCESS;
}
#else

#endif

static void __ina_service_signal_handler(ina_signal_t sig, ina_signal_behavior_t *sb, int *exitcode)
{
    if (sig == INA_SIGNAL_INT) {
        /* Ignore default signal handling */
        *sb = INA_SIGNAL_BEHAVIOR_IGNORE;
        __ina_service_context->descriptor->shutdown_func(__ina_service_context->descriptor->user_data);
#ifdef INA_OS_WIN32
        WaitForSingleObject(__ina_service_context->main_thread, INFINITE);
#endif
    }
}

INA_API(ina_rc_t) ina_service_init(ina_service_ctx_t **ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    *ctx = (ina_service_ctx_t*)ina_mem_alloc(sizeof(struct ina_service_ctx_s));
#ifdef INA_OS_WIN32
    (*ctx)->hmutex = INVALID_HANDLE_VALUE;
    (*ctx)->main_thread = GetCurrentThread();
#endif

    __ina_service_context = *ctx;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_service_destroy(ina_service_ctx_t **ctx)
{
#ifdef INA_OS_WIN32
    if ((*ctx)->hmutex != INVALID_HANDLE_VALUE) {
        ReleaseMutex((*ctx)->hmutex);
    }
#endif
    ina_mem_free(*ctx);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_service_install(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    ctx->descriptor = descriptor;
#ifdef INA_OS_WIN32
    return __ina_service_win_install(ctx, descriptor);
#else
    return INA_ERR_ENYI;
#endif
}

INA_API(ina_rc_t) ina_service_uninstall(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    ctx->descriptor = descriptor;
#ifdef INA_OS_WIN32
    return __ina_service_win_uninstall(ctx, descriptor);
#else
    return INA_ERR_ENYI;
#endif
}

INA_API(ina_rc_t) ina_service_run_service(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    ctx->descriptor = descriptor;
#ifdef INA_OS_WIN32
    return __ina_service_win_run(ctx, descriptor);
#else
    return INA_ERR_ENYI;
#endif
}

INA_API(ina_rc_t) ina_service_run_console(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    ctx->descriptor = descriptor;
    ina_register_signal_handler(INA_SIGNAL_INT, __ina_service_signal_handler);
#ifdef INA_OS_WIN32
    return __ina_service_win_console(ctx, descriptor);
#else
    return INA_ERR_ENYI;
#endif
}

INA_API(ina_rc_t) ina_service_mode(ina_service_ctx_t *ctx, ina_service_mode_t *mode)
{
    *mode = ctx->mode;
    return INA_SUCCESS;
}

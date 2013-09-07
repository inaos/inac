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
#ifdef INA_OS_WIN32
    HANDLE hmutex;
#else
#endif
};

#ifdef INA_OS_WIN32
static SERVICE_STATUS __ina_service_service_status;
static SERVICE_STATUS_HANDLE __ina_service_status_handle = 0;
static HANDLE __ina_service_stop_service_event = 0;

static void WINAPI ServiceControlHandler( DWORD controlCode )
{
	switch ( controlCode )
	{
		case SERVICE_CONTROL_INTERROGATE:
			break;

		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
			serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			SetServiceStatus( serviceStatusHandle, &serviceStatus );

			SetEvent( stopServiceEvent );
			return;

		case SERVICE_CONTROL_PAUSE:
			break;

		case SERVICE_CONTROL_CONTINUE:
			break;

		default:
			if ( controlCode >= 128 && controlCode <= 255 )
				// user defined control code
				break;
			else
				// unrecognised control code
				break;
	}

	SetServiceStatus( serviceStatusHandle, &serviceStatus );
}
static void WINAPI ServiceMain( DWORD /*argc*/, TCHAR* /*argv*/[] )
{
    

	// initialise service status
	serviceStatus.dwServiceType = SERVICE_WIN32;
	serviceStatus.dwCurrentState = SERVICE_STOPPED;
	serviceStatus.dwControlsAccepted = 0;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;

	serviceStatusHandle = RegisterServiceCtrlHandler( serviceName, ServiceControlHandler );

	if ( serviceStatusHandle )
	{
		// service is starting
		serviceStatus.dwCurrentState = SERVICE_START_PENDING;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );

		// do initialisation here
		stopServiceEvent = CreateEvent( 0, FALSE, FALSE, 0 );

		// running
		serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );

		do
		{
			Beep( 1000, 100 );

		}
		while ( WaitForSingleObject( stopServiceEvent, 1000 ) == WAIT_TIMEOUT );

		// service was stopped
		serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );

		// do cleanup here
		CloseHandle( stopServiceEvent );
		stopServiceEvent = 0;

		// service is now stopped
		serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );
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
            SC_HANDLE service = CreateService(serviceControlManager,
                ina_str_cstr(descriptor->name), ina_str_cstr(descriptor->display_name),
                SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                start_type, SERVICE_ERROR_NORMAL, path,
                0, 0, 0, username, password);

			if (service) {
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
    ina_str_t mutex_name = ina_str_fromcstr("/ina_service_mutex_");
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

    if (!INA_SUCCEED(__ina_service_win_setandcheck_mutex(ctx, descriptor))) {
        return INA_ERR_PUSH_LAST;
    }
    
    success = StartServiceCtrlDispatcher(serviceTable);
    if (!success) {
    	/* FIXME raise error */
    }

    return INA_SUCCESS;
}
static ina_rc_t __ina_service_win_console(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
    if (!INA_SUCCEED(__ina_service_win_setandcheck_mutex(ctx, descriptor))) {
        return INA_ERR_PUSH_LAST;
    }

    return INA_SUCCESS;
}
#else

#endif

INA_API(ina_rc_t) ina_service_init(ina_service_ctx_t **ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    *ctx = (ina_service_ctx_t*)ina_mem_alloc(sizeof(struct ina_service_ctx_s));
#ifdef INA_OS_WIN32
    (*ctx)->hmutex = INVALID_HANDLE_VALUE;
#endif
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
#ifdef INA_OS_WIN32
    return __ina_service_win_install(ctx, descriptor);
#else
    return INA_ERR_ENYI;
#endif
}

INA_API(ina_rc_t) ina_service_uninstall(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
#ifdef INA_OS_WIN32
    return __ina_service_win_uninstall(ctx, descriptor);
#else
    return INA_ERR_ENYI;
#endif
}

INA_API(ina_rc_t) ina_service_run_service(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
#ifdef INA_OS_WIN32
    return __ina_service_win_run(ctx, descriptor);
#else
    return INA_ERR_ENYI;
#endif
}

INA_API(ina_rc_t) ina_service_run_console(ina_service_ctx_t *ctx, ina_service_descriptor_t *descriptor)
{
#ifdef INA_OS_WIN32
    return __ina_service_win_console(ctx, descriptor);
#else
    return INA_ERR_ENYI;
#endif
}

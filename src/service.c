

/*
 * Copyright (c) 2013-2014, INAOS GmbH
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

#ifndef INA_OS_WIN32
#include <unistd.h>
#include <sys/stat.h>
#define INA_SERVICE_PID_FILE_FMT  "/var/run/%s.pid"
#define INA_SERVICE_LOCK_FILE_FMT "/var/lock/%s"
#define __INA_CHKCONFIG_DFT " -  20 80"
#define __INA_CHKCONFIG_CMD "chkconfig --%s %s"
#else
#define __INA_CHKCONFIG_DFT ""
#endif

struct ina_service_ctx_s {
    int is_deamon;
    ina_service_mode_t mode;
    ina_service_descriptor_t *descriptor;
    void *user_data;
#ifdef INA_OS_WIN32
    HANDLE hmutex;
    HANDLE main_thread;
    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE status_handle;
    HANDLE stop_service_event;  
#else
#endif
};

static ina_service_ctx_t *__ctx = NULL;

static ina_rc_t __ina_service_install(const ina_service_ctx_t*);
static ina_rc_t __ina_service_uninstall(const ina_service_ctx_t*);
static ina_rc_t __ina_service_run_service(const ina_service_ctx_t*);
static ina_rc_t __ina_service_run_console(const ina_service_ctx_t *ctx);
static ina_rc_t __ina_service_mgnt_status(const  char *name, ina_service_status_t *status);
static ina_rc_t __ina_service_mgnt_start(const char *name);
static ina_rc_t __ina_service_mgnt_stop(const char *name);

extern ina_service_descriptor_t __ina_service_section;

#ifdef INA_OS_WIN32

static DWORD __stdcall __ina_service_start_wrapper(LPVOID data)
{
    INA_ASSERT_NOTNULL(__ctx->descriptor);    
    INA_ASSERT_NOTNULL(__ctx->descriptor->service_fn);
    if (!INA_SUCCEED(__ctx->descriptor->service_fn(__ctx, INA_SERVICE_STATUS_START, __ctx->user_data))) {
        __ctx->descriptor->service_fn(__ctx, INA_SERVICE_STATUS_ERROR, __ctx->user_data);
        return INA_ERR_PUSH_LAST;
    }
    return __ctx->descriptor->service_fn(__ctx, INA_SERVICE_STATUS_RUN, __ctx->user_data);
}
static void WINAPI ServiceControlHandler( DWORD controlCode )
{
    switch (controlCode) {
        case SERVICE_CONTROL_INTERROGATE:
            break;
        case SERVICE_CONTROL_SHUTDOWN:
        case SERVICE_CONTROL_STOP:
            __ctx->status.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(__ctx->status_handle, &__ctx->status);
            SetEvent(__ctx->stop_service_event);
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
    SetServiceStatus(__ctx->status_handle, &__ctx->status);
}
static void WINAPI ServiceMain(DWORD argc, TCHAR* argv[])
{
    /* initialise service status */
    __ctx->status.dwServiceType = SERVICE_WIN32;
    __ctx->status.dwCurrentState = SERVICE_STOPPED;
    __ctx->status.dwControlsAccepted = 0;
    __ctx->status.dwWin32ExitCode = NO_ERROR;
    __ctx->status.dwServiceSpecificExitCode = NO_ERROR;
    __ctx->status.dwCheckPoint = 0;
    __ctx->status.dwWaitHint = 0;

    __ctx->status_handle = RegisterServiceCtrlHandler(
                    ina_str_cstr(__ctx->descriptor->name), 
                    ServiceControlHandler);

    if (__ctx->status_handle) {
        HANDLE thread_handle = INVALID_HANDLE_VALUE;
        DWORD thread_exit_code = 0;

        /* service is starting */
        __ctx->status.dwCurrentState = SERVICE_START_PENDING;
        SetServiceStatus(__ctx->status_handle, &__ctx->status);

        /* do initialisation here */
        thread_handle = CreateThread(NULL, 0, __ina_service_start_wrapper, NULL, 0, NULL);
        INA_ASSERT_NOTEQUAL(INVALID_HANDLE_VALUE, thread_handle);
        __ctx->stop_service_event = CreateEvent(0, FALSE, FALSE, 0);
        Sleep(1);
        GetExitCodeThread(thread_handle, &thread_exit_code);
        if (thread_exit_code != STILL_ACTIVE) {
            __ctx->status.dwCurrentState = SERVICE_STOPPED;
            __ctx->status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
            __ctx->status.dwServiceSpecificExitCode = thread_exit_code;
            SetServiceStatus(__ctx->status_handle, &__ctx->status);
            return;
        }

        /* running */
        __ctx->status.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
        __ctx->status.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(__ctx->status_handle, &__ctx->status);

        /* Wait for the stop-event to trigger */
        WaitForSingleObject(__ctx->stop_service_event, INFINITE);

        /* we received a stop-event now execute the shutdown proc and join the thread */
        __ctx->descriptor->service_fn(__ctx, INA_SERVICE_STATUS_SHUTDOWN, __ctx->user_data);
        WaitForSingleObject(thread_handle, INFINITE);

        /* service was stopped */
        __ctx->status.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(__ctx->status_handle, &__ctx->status);

        /* do cleanup here */
        CloseHandle(__ctx->stop_service_event);
        __ctx->stop_service_event = 0;

        /* service is now stopped */
        __ctx->status.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
        __ctx->status.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(__ctx->status_handle, &__ctx->status);
       __ctx->descriptor->service_fn(__ctx, INA_SERVICE_STATUS_STOP, (void*)__ctx->user_data);
    }
}
static ina_rc_t __ina_service_install(const ina_service_ctx_t *ctx)
{   
    DWORD start_type;
    SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
    LPCSTR username = NULL;
    LPCSTR password = NULL;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(ctx->descriptor);

    if (ina_str_len(ctx->descriptor->name) > 255) {
        return INA_SERVICE_EMAXLEN;
    }
    if (ina_str_len(ctx->descriptor->display_name) > 255) {
        return INA_SERVICE_EMAXLEN;
    }
    if (ctx->descriptor->startup == INA_SERVICE_STARTUP_TYPE_AUTO) {
        start_type = SERVICE_AUTO_START;
    }
    else if (ctx->descriptor->startup == INA_SERVICE_STARTUP_TYPE_MANUAL) {
        start_type = SERVICE_DEMAND_START;
    }
    else {
        return INA_SERVICE_EUST;
    }
    if (ctx->descriptor->username != NULL && strlen(ctx->descriptor->username)) {
        username = ina_str_cstr(ctx->descriptor->username);
    }
    if (ctx->descriptor->password != NULL && strlen(ctx->descriptor->password)) {
        password = ina_str_cstr(ctx->descriptor->password);
    }

    if (serviceControlManager) {
        char path[_MAX_PATH + 1];
            SC_HANDLE service;
            service = CreateService(serviceControlManager,
                ina_str_cstr(ctx->descriptor->name), ina_str_cstr(ctx->descriptor->display_name),
                SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                start_type, SERVICE_ERROR_NORMAL, ctx->descriptor->startup_args,
                0, 0, 0, username, password);
            
            if (service) {
                SERVICE_DESCRIPTION svc_desc;
                svc_desc.lpDescription = (LPSTR)ina_str_cstr(ctx->descriptor->description);
                ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &svc_desc);
                CloseServiceHandle(service);
            } else {
                CloseServiceHandle(serviceControlManager);
                return INA_SERVICE_ECAPI;
            }
        CloseServiceHandle(serviceControlManager);
    }
    else {
        return INA_SERVICE_ESCM;
    }

    return INA_SUCCESS;
}
static ina_rc_t __ina_service_uninstall(const ina_service_ctx_t *ctx)
{
    SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(ctx->descriptor);

    if (serviceControlManager) {
        SC_HANDLE service = OpenService(serviceControlManager,
            ina_str_cstr(ctx->descriptor->name), SERVICE_QUERY_STATUS | DELETE);
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
static ina_rc_t __ina_service_win_setandcheck_mutex(ina_service_ctx_t *ctx)
{
    ina_str_t mutex_name = ina_str_new_fromcstr("/ina_service_mutex_");
    ina_str_cat(mutex_name, ctx->descriptor->name);
    if (ctx->descriptor->exclusive_flag) {
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
static ina_rc_t __ina_service_run_service(const ina_service_ctx_t *ctx)
{
    SERVICE_TABLE_ENTRY serviceTable[] = {
        { (LPSTR)ina_str_cstr(ctx->descriptor->name), ServiceMain},
        { 0, 0 }
    };
    BOOL success;

    if (!INA_SUCCEED(__ina_service_win_setandcheck_mutex((ina_service_ctx_t*)ctx))) {
        return INA_ERR_PUSH_LAST;
    }
    
    success = StartServiceCtrlDispatcher(serviceTable);
    if (!success) {
        return INA_SERVICE_ESDIS;
    }

    return INA_SUCCESS;
}

static ina_rc_t __ina_service_run_console(const ina_service_ctx_t *ctx)
{
    if (!INA_SUCCEED(__ina_service_win_setandcheck_mutex((ina_service_ctx_t*)ctx))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_START, (void*)ctx->user_data))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_RUN, (void*)ctx->user_data))) {
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
}

static ina_rc_t __ina_service_mgnt_status(const char *name, ina_service_status_t *status)
{
    ina_str_t cmd;
    int retval;
    
	INA_ASSERT_NOTNULL(status);
    *status = INA_SERVICE_STATUS_STOP;

    cmd = ina_str_sprintf("sc query \"%s\" | find \"RUNNING\"", name);
    retval = system(ina_str_cstr(cmd));
    ina_str_free(cmd);
    if (retval == 0) {
		*status = INA_SERVICE_STATUS_RUN;
    }
    return INA_SUCCESS;
}

static ina_rc_t __ina_service_mgnt_start(const char *name)
{
    ina_str_t cmd;
    int retval;
    
    cmd = ina_str_sprintf("NET start %s", name);
    retval = system(ina_str_cstr(cmd));
    ina_str_free(cmd);
    if (retval == 0) {
        return INA_SUCCESS;
    }
	return INA_FAILURE;
}

static ina_rc_t __ina_service_mgnt_stop(const char *name)
{
    ina_str_t cmd;
    int retval;
    
    cmd = ina_str_sprintf("NET stop %s", name);
    retval = system(ina_str_cstr(cmd));
    ina_str_free(cmd);
    if (retval == 0) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

#else
extern char _binary____etc_template_init_script_tpl_start;
extern char _binary____etc_template_init_script_tpl_end;
/*
 * NOTES:
 *
 * 1. Is expected that the UNIX service creates two files upon startup
 *    - /var/run/${NAME}.pid (with the PID as only content)
 *    - /var/lock/subsys/${NAME}
 * 2. However the two files are removed by the service wrapper script
 *
 * 3. Install procedure:
 *    - read the init-script template from the section
 *    - process it via ina_template and write it to /etc/init.d/
 *    - also add it to chkconfig - if automated startup has been selected
 *
 * 4. Uninstall procedure:
 *    - remove from chkconfig - if automated startup has been selected
 *    - delete the service from /etc/init.d/
 *
 */
static ina_rc_t __ina_service_install(const ina_service_ctx_t *ctx)
{
    ina_template_ctx_t *tpl_ctx;
    ina_template_env_t *env;
    ina_str_t out;
    ina_str_t script_filepath;
    FILE *fp;
    
    if (!INA_SUCCEED(ina_template_init(&tpl_ctx))) {
        return INA_ERR_PUSH_LAST;
    }

    if (!INA_SUCCEED(ina_template_compile(tpl_ctx,
                     "init-script", 
                     &_binary____etc_template_init_script_tpl_start, 
                     &env))) {
        return INA_ERR_PUSH_LAST;
    }
    ina_template_set_at_as_expression_starter(tpl_ctx);

    ina_template_set_string(env, "service_name", ctx->descriptor->name);
    ina_template_set_string(env, "service_display_name",  ctx->descriptor->display_name);
    ina_template_set_string(env, "service_username", ctx->descriptor->username);
    ina_template_set_string(env, "service_startup", ctx->descriptor->startup_args);
    ina_template_set_string(env, "service_description", ctx->descriptor->description);
    ina_template_set_string(env, "service_chkconfig", ctx->descriptor->chkconfig);
 
    if (!INA_SUCCEED(ina_template_render(env, &out))) {
        return INA_ERR_PUSH_LAST;
    }
    
    ina_template_destroy(&tpl_ctx);

    /*
     * every distro seem to have a different way to manage autopstart for services 
     *
     * something for the portable header?
     * -> http://stackoverflow.com/questions/7824625/in-the-code-c-file-how-i-can-find-the-linux-distribution-name-version
     *
     * Redhat: https://access.redhat.com/site/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Deployment_Guide/s2-services-chkconfig.html
     * Debian/Ubuntu: http://www.debuntu.org/how-to-managing-services-with-update-rc-d/
     * Suse: chkconfig I guess
     */
    script_filepath = ina_str_new_fromcstr("/etc/init.d/");
    script_filepath = ina_str_catcstr(script_filepath, ctx->descriptor->name);
    fp = fopen(ina_str_cstr(script_filepath), "w");
    if (fp) {
        if (fwrite(ina_str_cstr(out), ina_str_len(out), 1, fp) <= 0) {
            /* FIXME: Error handling */
        }
        fclose(fp);
        chmod(ina_str_cstr(script_filepath), 0755);
    }
    ina_str_free(out);
    ina_str_free(script_filepath);

    return INA_SUCCESS;
}

static ina_rc_t __ina_service_uninstall(const ina_service_ctx_t *ctx)
{
    ina_str_t script_filepath;
    script_filepath = ina_str_new_fromcstr("/etc/init.d/");
    script_filepath = ina_str_catcstr(script_filepath, ctx->descriptor->name);
    unlink(script_filepath);
    return INA_SUCCESS;
}

static ina_rc_t __ina_service_run_service(const ina_service_ctx_t *ctx)
{
    pid_t pid;
    int fp,pfp;
    ina_str_t pid_str;
    ina_str_t pid_file_path;

    if (getppid() == 1) {
        return INA_SERVICE_EAID; /* already a daemon */
    }
    pid = fork();
    
    if (pid < 0) {
        return INA_SERVICE_EFERR; /* fork error */
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); /* parent exits */
    }

    if (setsid() < 0) { /* obtain a new process group */
        exit(EXIT_FAILURE);
    }
  
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0) {
        return INA_SERVICE_EFERR; /* fork error */
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); /* parent exits */
    }

    umask(0); /* protect files written by us */

    for (fp = getdtablesize(); fp >= 0; --fp) {
        close(fp); /* close all descriptors */
    }
    
    fp = open("/dev/null", O_RDWR); /* open stdin */
    fp = dup(fp); /* stdout */
    fp = dup(fp); /* stderr */

    /* set working-directory */
    if (chdir(ina_str_cstr(ctx->descriptor->working_directory)) < 0) {
        exit(EXIT_FAILURE);
    } 

    pid_file_path = ina_str_sprintf(INA_SERVICE_PID_FILE_FMT, 
                            ina_str_cstr(ctx->descriptor->name));
    pfp = open(ina_str_cstr(pid_file_path), O_RDWR | O_CREAT, 0640);
    if (pfp < 0) {
        return INA_SERVICE_ELCO; /* can not open */
    }
    pid_str = ina_str_sprintf("%d\n", getpid()); 
    if (ftruncate(pfp, 0) == 0) {
        if (write(pfp, ina_str_cstr(pid_str), ina_str_len(pid_str)+1)<=0) {
            close(pfp); 
            return INA_SERVICE_ELCO;
        }
        close(pfp); 
        return INA_SERVICE_ELCO;
    } 
    close(pfp);

    ina_str_free(pid_file_path);
    ina_str_free(pid_str);          close(pfp); 
            return INA_SERVICE_ELCO;
  

    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_START, (void*)ctx->user_data))) {
       return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_RUN, (void*)ctx->user_data))) {
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
}

static ina_rc_t __ina_service_run_console(const ina_service_ctx_t *ctx)
{
    int lfp;
    ina_str_t lock_file_path;

    lock_file_path = ina_str_sprintf(INA_SERVICE_PID_FILE_FMT, 
                                        ina_str_cstr(ctx->descriptor->name));

    lfp = open(ina_str_cstr(lock_file_path), O_RDWR | O_CREAT, 0640);
    if (lfp < 0) {
        return INA_SERVICE_ELCO; /* can not open */
    }
    if (lockf(lfp, F_TLOCK, 0) < 0) {
        return INA_SERVICE_ELOCK; /* can not lock */
    }
   if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_START, (void*)ctx->user_data))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_RUN, (void*)ctx->user_data))) {
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
}

static ina_rc_t __ina_service_mgnt_start(const char *name)
{
    ina_str_t cmd;
    int retval;
    
    cmd = ina_str_sprintf("/etc/init.d/%s start", name);
    retval = system(ina_str_cstr(cmd));
    ina_str_free(cmd);
    if (retval == 0) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

static ina_rc_t __ina_service_mgnt_stop(const char *name)
{
    ina_str_t cmd;
    int retval;
    
    cmd = ina_str_sprintf("/etc/init.d/%s stop", name);
    retval = system(ina_str_cstr(cmd));
    ina_str_free(cmd);
    if (retval == 0) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

static  ina_rc_t __ina_service_mgnt_status(const char *name, ina_service_status_t *status)
{
    int lfp;
    ina_str_t lock_file_path;

    INA_ASSERT_NOTNULL(status);

    *status = INA_SERVICE_STATUS_STOP;

    lock_file_path = ina_str_sprintf(INA_SERVICE_PID_FILE_FMT, name);
    lfp = open(ina_str_cstr(lock_file_path), O_RDONLY, 0640);
    if (lfp > 0) {
        *status = INA_SERVICE_STATUS_RUN;
        close(lfp);
    }
    ina_str_free(lock_file_path);
    return INA_SUCCESS;
}
#endif

static void __ina_service_signal_handler(ina_signal_t sig, 
                                         ina_signal_behavior_t *sb, 
                                         int *exitcode)
{
    if (sig == INA_SIGNAL_INT || sig == INA_SIGNAL_TERM) {
        if (INA_SUCCEED(__ctx->descriptor->service_fn(
            __ctx, INA_SERVICE_STATUS_SHUTDOWN, 
            (void*)__ctx->user_data))) {
            __ctx->descriptor->service_fn(
                __ctx, INA_SERVICE_STATUS_STOP,
                (void*)__ctx->user_data);
        } else {
            __ctx->descriptor->service_fn(__ctx,
                INA_SERVICE_STATUS_ERROR,
                (void*)__ctx->user_data);
        }
#ifdef INA_OS_WIN32
        WaitForSingleObject(__ctx->main_thread, INFINITE);
#endif
    }
}

INA_API(ina_rc_t) ina_service_init(ina_service_ctx_t **ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    if (__ctx != NULL) {
        *ctx = __ctx;
        return INA_SUCCESS;
    }

    __ctx = (ina_service_ctx_t*)ina_mem_alloc(sizeof(ina_service_ctx_t));
    if (__ctx == NULL) {
        return INA_ERR_PUSH_LAST;
    }

#ifdef INA_OS_WIN32
    __ctx->hmutex = INVALID_HANDLE_VALUE;
    __ctx->main_thread = GetCurrentThread();
#endif
    if (!INA_SUCCEED(ina_service_get_descriptor(__ctx, &__ctx->descriptor))) {
        return INA_ERR_PUSH_LAST;
    }
    if (__ctx->descriptor->service_fn) {
        if (!INA_SUCCEED(__ctx->descriptor->service_fn(__ctx, INA_SERVICE_STATUS_INIT, (void*)__ctx->user_data))) {
            return INA_ERR_PUSH_LAST;
        }
    }
    *ctx = __ctx;
    return INA_SUCCESS;
} 

INA_API(ina_rc_t) ina_service_destroy(ina_service_ctx_t **ctx)
{
    if (*ctx == NULL || __ctx == NULL) {
        return INA_SUCCESS;
    }
    if (__ctx != __ctx) {
        return INA_FAILURE;
    }

#ifdef INA_OS_WIN32
    if ((*ctx)->hmutex != INVALID_HANDLE_VALUE) {
        ReleaseMutex((*ctx)->hmutex);
    }
#endif
    if ((*ctx)->descriptor){
        ina_mem_free((*ctx)->descriptor);
        (*ctx)->descriptor = NULL;
    }
    ina_mem_free(ctx);
    __ctx = NULL;
    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_service_get_data(const ina_service_ctx_t *ctx, const void **user_data)
{
    INA_ASSERT_NOTNULL(ctx);
    *user_data = ctx->user_data;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_service_set_data(const ina_service_ctx_t *ctx, const void *user_data) 
{
    INA_ASSERT_NOTNULL(ctx);
    ((ina_service_ctx_t*)ctx)->user_data = (void*)user_data;
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_service_dispatch(const ina_service_ctx_t *ctx, const void *user_data)
{
    ina_str_t cmd;
    INA_ASSERT_NOTNULL(ctx);

    if (user_data != NULL) {
        ina_service_set_data(ctx, user_data);
    }

    /* Check if run in console mode */
    if (INA_SUCCEED(ina_opt_get_string(INA_SERVICE_OPT_NAME, &cmd)) &&
        INA_CSTR_CASECMP(ina_str_cstr(cmd), INA_SERVICE_CMD_CONSOLE) == 0) {
        return ina_service_run_service(ctx, INA_YES, user_data);
    }

    else if (INA_CSTR_CASECMP(ina_str_cstr(cmd), INA_SERVICE_CMD_INSTALL) == 0) {
        return ina_service_install(ctx);
    }

    else if (INA_CSTR_CASECMP(ina_str_cstr(cmd), INA_SERVICE_CMD_UNINSTALL) == 0) {
        return ina_service_uninstall(ctx);
    }

    else if (INA_CSTR_CASECMP(ina_str_cstr(cmd), INA_SERVICE_CMD_DEAMON) == 0) {
        return ina_service_run_service(ctx, INA_NO, user_data);
    }

    else if (INA_CSTR_CASECMP(ina_str_cstr(cmd), INA_SERVICE_CMD_REPORT) == 0) {
        ina_service_descriptor_t *ds = NULL;
        ina_service_get_descriptor(ctx, &ds);
        INA_ASSERT_NOTNULL(ds);
        return ds->service_fn(ctx, INA_SERVICE_STATUS_REPORT, (void*)user_data);
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_service_get_descriptor(const ina_service_ctx_t *ctx, 
                                    ina_service_descriptor_t **descriptor)
{
    ina_service_descriptor_t *ds;

    INA_ASSERT_NOTNULL(ctx);

    if (ctx->descriptor) {
        *descriptor = ctx->descriptor;
        return INA_SUCCESS;
    }

    ds = ina_mem_alloc(sizeof(ina_service_descriptor_t));
    if (ds == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    ina_mem_cpy(ds, &__ina_service_section, sizeof(ina_service_descriptor_t));
    ((ina_service_ctx_t*)ctx)->descriptor = ds;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_service_install(const ina_service_ctx_t *ctx)
{
    ina_service_descriptor_t *ds = NULL;
    
    INA_ASSERT_NOTNULL(ctx);

    ina_service_get_descriptor(ctx, &ds);
    INA_ASSERT_NOTNULL(ds);
  
    if (strlen(ds->chkconfig) == 0) {
        strcpy(ds->chkconfig, __INA_CHKCONFIG_DFT);
    }

    if (strlen(ds->startup_args) == 0) {
        int index = 0;
        ina_str_t key;
        ina_str_t value;
        ina_str_t startup_args = ina_str_new_fromcstr("");

        startup_args = ina_str_catcstr(startup_args, ina_app_get_path());
            
        while (INA_SUCCEED(ina_opt_get_key_value(index, &key, &value))) {
            if (INA_CSTR_CASECMP(ina_str_cstr(key), INA_SERVICE_OPT_NAME) != 0) {
                startup_args = ina_str_catcstr(startup_args, " --");
                startup_args = ina_str_cat(startup_args, key);
                startup_args = ina_str_catcstr(startup_args, "=");
                startup_args = ina_str_cat(startup_args, value);
            }
            index++;
        }
        startup_args = ina_str_catcstr(startup_args, " --");
        startup_args = ina_str_catcstr(startup_args, INA_SERVICE_OPT_NAME);
        startup_args = ina_str_catcstr(startup_args, "=");
        startup_args = ina_str_catcstr(startup_args, INA_SERVICE_CMD_DEAMON);
        strncpy(ds->startup_args, 
            ina_str_cstr(startup_args), 
            INA_SERVICE_STARTUP_ARGS_MAXLEN);
    }

    if (INA_SUCCEED(__ina_service_install(ctx))) {
        ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_INSTALL, ctx->user_data);
        return INA_SUCCESS;
    }
    ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_ERROR, ctx->user_data);
    return INA_ERR_PUSH_LAST; 
}

INA_API(ina_rc_t) ina_service_uninstall(const ina_service_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    if (INA_SUCCEED(__ina_service_uninstall(ctx))) {
        ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_UNINSTALL, ctx->user_data);
        return INA_SUCCESS;
    }
    ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_ERROR, ctx->user_data);
    return INA_ERR_PUSH_LAST; 
}

INA_API(ina_rc_t) ina_service_run_service(const ina_service_ctx_t *ctx, int console, const void *user_data)
{
    INA_ASSERT_NOTNULL(ctx);

    ina_register_signal_handler(INA_SIGNAL_TERM, __ina_service_signal_handler);
    ina_register_signal_handler(INA_SIGNAL_INT, __ina_service_signal_handler);
    
    if (user_data != NULL) {
        ina_service_set_data(ctx, user_data);
    } 
    if (!console) {
        return __ina_service_run_service(ctx);
    }
    ((ina_service_ctx_t*)ctx)->is_deamon = INA_YES;
    return __ina_service_run_console(ctx);
}

INA_API(ina_rc_t) ina_service_get_mode(const ina_service_ctx_t *ctx, 
                                       ina_service_mode_t *mode)
{
    *mode = ctx->mode;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_service_is_deamon(const ina_service_ctx_t *ctx) 
{
    INA_ASSERT_NOTNULL(ctx);
    if (ctx->is_deamon) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_service_mgnt_start(const char *name)
{
    ina_service_status_t status;
    INA_ASSERT_NOTNULL(name);
    INA_ASSERT_TRUE(strlen(name));

    if (!INA_SUCCEED(ina_service_mgnt_status(name, &status))) {
        return INA_ERR_PUSH_LAST;
    }

    if (status == INA_SERVICE_STATUS_RUN) {
        return INA_SUCCESS;
    }

    if (status == INA_SERVICE_STATUS_STOP) {
        return __ina_service_mgnt_start(name);
    }
    /* TODO: specific error */
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_service_mgnt_stop(const char *name)
{
    ina_service_status_t status;

    INA_ASSERT_NOTNULL(name);
    INA_ASSERT_TRUE(strlen(name));

    if (!INA_SUCCEED(ina_service_mgnt_status(name, &status))) {
        return INA_ERR_PUSH_LAST;
    }

    if (status == INA_SERVICE_STATUS_STOP) {
        return INA_SUCCESS;
    }

    if (status == INA_SERVICE_STATUS_RUN) {
        return __ina_service_mgnt_stop(name);
    }
    /* TODO: specific error */
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_service_mgnt_status(const char *name, ina_service_status_t *status)
{
    INA_ASSERT_NOTNULL(name);
    INA_ASSERT_TRUE(strlen(name));
    INA_ASSERT_NOTNULL(status);
    return __ina_service_mgnt_status(name, status);
}

INA_API(ina_rc_t) ina_service_mgnt_install(const char *bin_path, const char *startup_args)
{
    ina_str_t cmd;
    int retval;

    INA_ASSERT_NOTNULL(bin_path);
    
    
    if (startup_args != NULL) {
        cmd = ina_str_sprintf("%s --service=install %s", bin_path, startup_args);
    } else {
        cmd = ina_str_sprintf("%s --service=install", bin_path);        
    }
    retval = system(cmd);

    ina_str_free(cmd);
    
    if (retval != 0) {
        /* TODO: Specific error */
        return INA_FAILURE;
    }
    return INA_SUCCESS;

}

INA_API(ina_rc_t) ina_service_mgnt_uninstall(const char *bin_path)
{
    ina_str_t cmd;
    int retval;

    INA_ASSERT_NOTNULL(bin_path);

    cmd = ina_str_sprintf("%s --service=uninstall", bin_path);
    retval = system(cmd);

    ina_str_free(cmd);

    if (retval != 0) {
        /* TODO: Specific error */
        return INA_FAILURE;
    }
    return INA_SUCCESS;
}
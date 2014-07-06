

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
#define INA_SERVICE_LOCK_FILE_FMT "/var/lock/subsys/%s"
#endif

struct ina_service_ctx_s {
    ina_service_mode_t mode;
    ina_service_descriptor_t *descriptor;
#ifdef INA_OS_WIN32
    HANDLE hmutex;
    HANDLE main_thread;
    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE status_handle;
    HANDLE stop_service_event;  
#else
#endif
};

static ina_service_ctx_t __ina_service_ctx;

static ina_rc_t __ina_service_install(const ina_service_ctx_t*);
static ina_rc_t __ina_service_uninstall(const ina_service_ctx_t*);
static ina_rc_t __ina_service_run_service(const ina_service_ctx_t*);
static ina_rc_t __ina_service_run_console(const ina_service_ctx_t *ctx);

extern ina_service_descriptor_t __ina_service_section;

#ifdef INA_OS_WIN32

static DWORD __stdcall __ina_service_start_wrapper(LPVOID data)
{
    INA_ASSERT_NOTNULL(__ina_service_ctx.descriptor);    
    INA_ASSERT_NOTNULL(__ina_service_ctx.descriptor->service_fn);
    if (!INA_SUCCEED(__ina_service_ctx.service_fn(&__ina_service_ctx, INA_SERVICE_STATUS_STARTUP))) {
        __ina_service_ctx.service_fn(&__ina_service_ctx, INA_SERVICE_STATUS_ERROR);
        return INA_ERR_PUSH_LAST;
    }
    return __ina_service_ctx.descriptor->service_fn(__ina_service_ctx, INA_SERVICE_STATUS_RUNNING);
}
static void WINAPI ServiceControlHandler( DWORD controlCode )
{
    switch (controlCode) {
        case SERVICE_CONTROL_INTERROGATE:
            break;
        case SERVICE_CONTROL_SHUTDOWN:
        case SERVICE_CONTROL_STOP:
            __ina_service_ctx.status.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(__ina_service_ctx.status_handle, &__ina_service_ctx.status);
            SetEvent(__ina_service_ctx.stop_service_event);
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
    SetServiceStatus(__ina_service_ctx.status_handle, &__ina_service_ctx.status);
}
static void WINAPI ServiceMain(DWORD argc, TCHAR* argv[])
{
    /* initialise service status */
    __ina_service_ctx.status.dwServiceType = SERVICE_WIN32;
    __ina_service_ctx.status.dwCurrentState = SERVICE_STOPPED;
    __ina_service_ctx.status.dwControlsAccepted = 0;
    __ina_service_ctx.status.dwWin32ExitCode = NO_ERROR;
    __ina_service_ctx.status.dwServiceSpecificExitCode = NO_ERROR;
    __ina_service_ctx.status.dwCheckPoint = 0;
    __ina_service_ctx.status.dwWaitHint = 0;

    __ina_service_ctx.status_handle = RegisterServiceCtrlHandler(
                    ina_str_cstr(__ina_service_ctx.descriptor->name), 
                    ServiceControlHandler);

    if (__ina_service_ctx.status_handle) {
        HANDLE thread_handle = INVALID_HANDLE_VALUE;
        DWORD thread_exit_code = 0;

        /* service is starting */
        __ina_service_ctx.status.dwCurrentState = SERVICE_START_PENDING;
        SetServiceStatus(__ina_service_ctx.status_handle, &__ina_service_ctx.status);

        /* do initialisation here */
        thread_handle = CreateThread(NULL, 0, __ina_service_start_wrapper, NULL, 0, NULL);
        INA_ASSERT_NOTEQUAL(INVALID_HANDLE_VALUE, thread_handle);
        __ina_service_ctx.stop_service_event = CreateEvent(0, FALSE, FALSE, 0);
        Sleep(1);
        GetExitCodeThread(thread_handle, &thread_exit_code);
        if (thread_exit_code != STILL_ACTIVE) {
            __ina_service_ctx.status.dwCurrentState = SERVICE_STOPPED;
            __ina_service_ctx.status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
            __ina_service_ctx.status.dwServiceSpecificExitCode = thread_exit_code;
            SetServiceStatus(__ina_service_ctx.status_handle, &__ina_service_ctx.status);
            return;
        }

        /* running */
        __ina_service_ctx.status.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
        __ina_service_ctx.status.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(__ina_service_ctx.status_handle, &__ina_service_ctx.status);

        /* Wait for the stop-event to trigger */
        WaitForSingleObject(__ina_service_ctx.stop_service_event, INFINITE);

        /* we received a stop-event now execute the shutdown proc and join the thread */
        __ina_service_ctx.descriptor->service_fn(&__ina_service_ctx, INA_SERVICE_STATUS_SHUTDOWN);
        WaitForSingleObject(thread_handle, INFINITE);

        /* service was stopped */
        __ina_service_ctx.status.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(__ina_service_ctx.status_handle, &__ina_service_ctx.status);

        /* do cleanup here */
        CloseHandle(__ina_service_ctx.stop_service_event);
        __ina_service_ctx.stop_service_event = 0;

        /* service is now stopped */
        __ina_service_ctx.status.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
        __ina_service_ctx.status.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(__ina_service_ctx.status_handle, &__ina_service_ctx.status);
       __ina_service_ctx.descriptor->service_fn(&__ina_service_ctx, INA_SERVICE_STATUS_STOPPED);
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
    if (ctx->descriptor->username != NULL) {
        username = ina_str_cstr(ctx->descriptor->username);
    }
    if (ctx->descriptor->username != NULL) {
        password = ina_str_cstr(ctx->descriptor->password);
    }

    if (serviceControlManager) {
        char path[_MAX_PATH + 1];
        if (GetModuleFileName(0, path, sizeof(path)/sizeof(path[0])) > 0 ) {
            SC_HANDLE service;
            if (ctx->descriptor->startup_args != NULL) {
                strcat(path, " ");
                strcat(path, ina_str_cstr(ctx->descriptor->startup_args));
            }
            service = CreateService(serviceControlManager,
                ina_str_cstr(ctx->descriptor->name), ina_str_cstr(ctx->descriptor->display_name),
                SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                start_type, SERVICE_ERROR_NORMAL, path,
                0, 0, 0, username, password);

            if (service) {
                SERVICE_DESCRIPTION svc_desc;
                svc_desc.lpDescription = (LPSTR)ina_str_cstr(ctx->descriptor->short_description);
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
    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_STARTUP))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_RUNNING))) {
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
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
    ina_template_set_string(env, "service_short_description", ctx->descriptor->long_description);
    ina_template_set_string(env, "service_long_description", ctx->descriptor->short_description);

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
    int fp,lfp,pfp;
    ina_str_t pid_str;
    ina_str_t pid_file_path;
    ina_str_t lock_file_path;

    if (getppid() == 1) {
        return INA_SERVICE_EAID; /* already a daemon */
    }
    pid = fork();
    
    if (pid < 0) {
        return INA_SERVICE_EFERR; /* fork error */
    }
    if (pid > 0) {
        exit(0); /* parent exits */
    }

    setsid(); /* obtain a new process group */

    for (fp = getdtablesize(); fp >= 0; --fp) {
        close(fp); /* close all descriptors */
    }

    fp = open("/dev/null", O_RDWR); /* open stdin */
    dup(fp); /* stdout */
    dup(fp); /* stderr */

    umask(027); /* protect files written by us */

    /* set working-directory */
    chdir(ina_str_cstr(ctx->descriptor->working_directory)); 

    pid_file_path = ina_str_sprintf(INA_SERVICE_PID_FILE_FMT, 
                            ina_str_cstr(ctx->descriptor->name));
    lock_file_path = ina_str_sprintf(INA_SERVICE_PID_FILE_FMT, 
                            ina_str_cstr(ctx->descriptor->name));

    
    lfp = open(ina_str_cstr(lock_file_path), O_RDWR | O_CREAT, 0640);
    if (lfp < 0) {
        return INA_SERVICE_ELCO; /* can not open */
    }
    if (lockf(lfp, F_TLOCK, 0) < 0) {
        return INA_SERVICE_ELOCK; /* can not lock */
    }
    
    pid_str = ina_str_sprintf("%d\n", getpid());

    pfp = open(ina_str_cstr(pid_file_path), O_RDWR | O_CREAT, 0640);
    write(pfp, ina_str_cstr(pid_str), ina_str_len(pid_str)); /* record pid to pid-file */
    close(pfp);

    ina_str_free(pid_file_path);
    ina_str_free(lock_file_path);
    ina_str_free(pid_str);

    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_STARTUP))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_RUNNING))) {
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
   if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_STARTUP))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_RUNNING))) {
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
}

#endif

static void __ina_service_signal_handler(ina_signal_t sig, 
                                         ina_signal_behavior_t *sb, 
                                         int *exitcode)
{
    if (sig == INA_SIGNAL_INT || sig == INA_SIGNAL_TERM) {
        __ina_service_ctx.descriptor->service_fn(
            &__ina_service_ctx, INA_SERVICE_STATUS_SHUTDOWN);
#ifdef INA_OS_WIN32
        WaitForSingleObject(__ina_service_ctx.main_thread, INFINITE);
#endif
    }
}

INA_API(ina_rc_t) ina_service_init(ina_service_ctx_t **ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    *ctx = &__ina_service_ctx;

#ifdef INA_OS_WIN32
    (*ctx)->hmutex = INVALID_HANDLE_VALUE;
    (*ctx)->main_thread = GetCurrentThread();
#endif
    return ina_service_get_descriptor(*ctx, &(*ctx)->descriptor);
}

INA_API(ina_rc_t) ina_service_destroy(ina_service_ctx_t **ctx)
{
#ifdef INA_OS_WIN32
    if ((*ctx)->hmutex != INVALID_HANDLE_VALUE) {
        ReleaseMutex((*ctx)->hmutex);
    }
#endif
    if ((*ctx)->descriptor){
        ina_mem_free((*ctx)->descriptor);
        (*ctx)->descriptor = NULL;
    }
    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_service_dispatch(const ina_service_ctx_t *ctx)
{
    ina_str_t cmd;
    INA_ASSERT_NOTNULL(ctx);

    /* Check if run in console mode */
    if (INA_SUCCEED(ina_opt_get_string(INA_SERVICE_OPT_NAME, &cmd)) &&
        strcasecmp(ina_str_cstr(cmd), INA_SERVICE_CMD_CONSOLE) == 0) {
        return ina_service_run_service(ctx, INA_YES);
    }

    if (strcasecmp(ina_str_cstr(cmd), INA_SERVICE_CMD_INSTALL) == 0) {
        ina_service_descriptor_t *ds = NULL;
        ina_service_get_descriptor(ctx, &ds);
        INA_ASSERT_NOTNULL(ds);
  
        if (strlen(ds->startup_args) == 0) {
            int index = 0;
            ina_str_t key;
            ina_str_t value;
            ina_str_t startup_args = ina_str_new_fromcstr("");

            startup_args = ina_str_catcstr(startup_args, ina_app_get_path());
            startup_args = ina_str_catcstr(startup_args, " ");
            
            while (INA_SUCCEED(ina_opt_get_key_value(index, &key, &value))) {
                if (strcasecmp(ina_str_cstr(key), INA_SERVICE_OPT_NAME) != 0) {
                    startup_args = ina_str_catcstr(startup_args, " --");
                    startup_args = ina_str_cat(startup_args, key);
                    startup_args = ina_str_catcstr(startup_args, " ");
                    startup_args = ina_str_cat(startup_args, value);
                }
                index++;
            }
            startup_args = ina_str_catcstr(startup_args, " --");
            startup_args = ina_str_catcstr(startup_args, INA_SERVICE_OPT_NAME);
            startup_args = ina_str_catcstr(startup_args, " ");
            startup_args = ina_str_catcstr(startup_args, INA_SERVICE_CMD_DEAMON);
            strncpy(ds->startup_args, 
                ina_str_cstr(startup_args), 
                INA_SERVICE_STARTUP_ARGS_MAXLEN);
        }
        return ina_service_install(ctx);
    }

    if (strcasecmp(ina_str_cstr(cmd), INA_SERVICE_CMD_UNINSTALL) == 0) {
        return ina_service_uninstall(ctx);
    }

    if (strcasecmp(ina_str_cstr(cmd), INA_SERVICE_CMD_DEAMON) == 0) {
        return ina_service_run_service(ctx, INA_NO);
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_service_get_descriptor(const ina_service_ctx_t *ctx, 
                                    ina_service_descriptor_t **descriptor)
{
    ina_service_descriptor_t *ds;

    if (ctx->descriptor) {
        *descriptor = ctx->descriptor;
        return INA_SUCCESS;
    }

    ds = ina_mem_alloc(sizeof(ina_service_descriptor_t));
    ina_mem_cpy(ds, &__ina_service_section, sizeof(ina_service_descriptor_t));
    ((ina_service_ctx_t*)ctx)->descriptor = ds;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_service_install(const ina_service_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_INSTALL))) {
        return INA_ERR_PUSH_LAST;
    } 
    return __ina_service_install(ctx);
}

INA_API(ina_rc_t) ina_service_uninstall(const ina_service_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    if (!INA_SUCCEED(ctx->descriptor->service_fn(ctx, INA_SERVICE_STATUS_UNINSTALL))) {
        return INA_ERR_PUSH_LAST;
    }     
    return __ina_service_uninstall(ctx);
}

INA_API(ina_rc_t) ina_service_run_service(const ina_service_ctx_t *ctx, int console)
{
    ina_register_signal_handler(INA_SIGNAL_TERM, __ina_service_signal_handler);
    
    if (!console) {
        return __ina_service_run_service(ctx);
    }
    ina_register_signal_handler(INA_SIGNAL_INT, __ina_service_signal_handler);
    return __ina_service_run_console(ctx);
}

INA_API(ina_rc_t) ina_service_get_mode(const ina_service_ctx_t *ctx, 
                                       ina_service_mode_t *mode)
{
    *mode = ctx->mode;
    return INA_SUCCESS;
}
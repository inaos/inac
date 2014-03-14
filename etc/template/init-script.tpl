#!/bin/bash
#
# @{service_name}      This shell script takes care of starting and stopping @{service_display_name}
#
# chkconfig: - 80 20
#
### BEGIN INIT INFO
# Provides: @{service_name}
# Required-Start: $network $local_fs
# Required-Stop: $network $local_fs
# Default-Start:
# Default-Stop:
# Description: @{service_long_description}
# Short-Description: @{service_short_description}
### END INIT INFO

# Copyright (c) 2014, INAOS GmbH
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the INAOS GmbH nor the names of its contributors
#       may be used to endorse or promote products derived from this software 
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
# ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.

## Source function library.
#. /etc/rc.d/init.d/functions
# Source LSB function library.
if [ -r /lib/lsb/init-functions ]; then
    . /lib/lsb/init-functions
else
    exit 1
fi

NAME="$(basename $0)"

# For SELinux we need to use 'runuser' not 'su'
if [ -x "/sbin/runuser" ]; then
    SU="/sbin/runuser"
else
    SU="/bin/su"
fi
    
# Define username and password
INA_SERVICE_USER=@{service_username}

# Define service-start command-line
INA_SERVICE_STARTUP=@{service_startup}

RETVAL="0"

# start-service
function start() {

    echo -n "Starting @{service_name}: "
    if [ -f "/var/lock/subsys/${NAME}" ] ; then
		read mypid < /var/run/${NAME}.pid
		if [ -d "/proc/${mypid}" ]; then
			log_success_msg
			return 0
		fi
    fi
    
    $SU - $INA_SERVICE_USER -c "${INA_SERVICE_STARTUP}"
    
    RETVAL="$?"
    if [ "$RETVAL" -eq 0 ]; then 
        log_success_msg
    else
        log_failure_msg
    fi
    
    return $RETVAL
}

# stop-service
function stop() {
    RETVAL="0"
    echo -n "Stopping @{service_name}: "
    if [ -f "/var/lock/subsys/${NAME}" ]; then
        
		# read pid and send SIGINT
		read mypid < /var/run/${NAME}.pid
		kill -SIGINT $mypid
		
        RETVAL="$?"
        if [ "$RETVAL" -eq "0" ]; then
            count="0"
			until [ "$(ps --pid $mypid | grep -c $mypid)" -eq "0" ] || \
				  [ "$count" -gt "$SHUTDOWN_WAIT" ]; do
				if [ "$SHUTDOWN_VERBOSE" = "true" ]; then
					echo "waiting for processes $mypid to exit"
				fi
				sleep 1
				let count="${count}+1"
			done
			if [ "$count" -gt "$SHUTDOWN_WAIT" ]; then
				if [ "$SHUTDOWN_VERBOSE" = "true" ]; then
					echo "killing processes which didn't stop after $SHUTDOWN_WAIT seconds"
				fi
				kill -9 $mypid
			fi
			log_success_msg            
            rm -f /var/lock/subsys/${NAME} /var/run/${NAME}.pid
        else
            log_failure_msg
        fi
    else
        log_success_msg
    fi
    
    return $RETVAL
}

# See how we were called.
case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        stop
        start
        ;;
    condrestart|try-restart)
        if [ -f "/var/run/${NAME}.pid" ]; then
            stop
            start
        fi
        ;;
    reload)
        RETVAL="3"
        ;;
    force-reload)
        if [ -f "/var/run/${NAME}.pid" ]; then
            stop
            start
        fi
        ;;
    status)
        if [ -f "/var/run/${NAME}.pid" ]; then
            read mypid < /var/run/${NAME}.pid
            if [ -d "/proc/${mypid}" ]; then
                echo "${NAME} (pid ${mypid}) is running..."
                RETVAL="0"
            fi
        else
            pid=`$(/usr/bin/pgrep -d , "${INA_SERVICE_STARTUP}")`
            if [ -z "$pid" ]; then
                echo "${NAME} is stopped"
                RETVAL="3"
            else
                echo "${NAME} (pid $pid) is running..."
                RETVAL="0"
            fi
        fi
        ;;
    version)
        RETVAL="3"
        ;;
    *)
        echo "Usage: $0 {start|stop|restart|condrestart|try-restart|reload|force-reload|status|version}"
        RETVAL="2"
esac

exit $RETVAL

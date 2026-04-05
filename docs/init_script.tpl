#!/bin/sh
#
# Simple init.d script conceived to work on Linux systems
# as it does use of the /proc filesystem.

# This is arbitrary. Change it to whatever you'd like.
SERVICE_NAME="frequent_service"

# Frequent cron must exist at this path. Optionally, you can change it to a path where it does exist.
EXEC=/usr/bin/frequent-cron

# Change this to the path of your choice.
PIDFILE=/var/run/${SERVICE_NAME}.pid

# Point this to the shell (or program) that you'd like to run. /tmp/myshell.sh is just an example.
COMMAND=/tmp/myshell.sh

# Frequency is in milliseconds. The command will be invoked every frequency interval.
FREQUENCY=1000

case "$1" in
    start)
        if [ -f $PIDFILE ]
        then
                echo "$PIDFILE exists; process is already running or crashed."
        else
                echo "Starting $SERVICE_NAME frequent cron..."
                FULL_COMMAND="$EXEC --frequency $FREQUENCY --command $COMMAND --pid-file $PIDFILE"
                $FULL_COMMAND
                echo "${SERVICE_NAME} started."
        fi
        ;;
    stop)
        if [ ! -f $PIDFILE ]
        then
                echo "$PIDFILE does not exist; process is not running."
        else
                echo "Stopping..."
                kill $(cat $PIDFILE)
                rm $PIDFILE
                echo "${SERVICE_NAME} stopped."
        fi
        ;;
    *)
        echo "Please use start or stop as first argument."
        ;;
esac

# FreeBSD Installation and Setup

## Dependencies

```bash
pkg install cmake boost-all sqlite3
```

## Build

```bash
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake .
make -j$(sysctl -n hw.ncpu)
make test
make install
```

## Running Directly

```bash
frequent-cron run --frequency=1000 --command="/path/to/your/script.sh" --pid-file=/var/run/frequent-cron.pid
```

## Managing Services

```bash
# Register a service (also creates an rc.d script)
frequent-cron install myservice --frequency=1000 --command="/path/to/script.sh"

# This creates: /usr/local/etc/rc.d/frequent_cron_myservice
# And enables it in rc.conf via sysrc

# Start/stop/status
frequent-cron start myservice
frequent-cron status
frequent-cron stop myservice

# View logs
frequent-cron logs myservice

# Remove (also removes the rc.d script)
frequent-cron remove myservice
```

## Manual rc.d Setup

If you prefer to manage the rc.d script yourself:

1. Create `/usr/local/etc/rc.d/frequent_cron_myservice`:

   ```sh
   #!/bin/sh
   #
   # PROVIDE: frequent_cron_myservice
   # REQUIRE: DAEMON
   # KEYWORD: shutdown

   . /etc/rc.subr

   name="frequent_cron_myservice"
   rcvar="frequent_cron_myservice_enable"
   pidfile="/var/run/frequent-cron-myservice.pid"
   command="/usr/local/bin/frequent-cron"
   command_args="run --frequency=1000 --command='/path/to/script.sh' --pid-file=/var/run/frequent-cron-myservice.pid"

   load_rc_config $name
   run_rc_command "$1"
   ```

2. Enable and start:
   ```bash
   chmod +x /usr/local/etc/rc.d/frequent_cron_myservice
   sysrc frequent_cron_myservice_enable=YES
   service frequent_cron_myservice start
   ```

3. To stop:
   ```bash
   service frequent_cron_myservice stop
   ```

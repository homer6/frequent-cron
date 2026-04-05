# Ubuntu/Debian Installation and Setup

## Dependencies

```bash
sudo apt-get install libboost-system-dev libboost-program-options-dev libsqlite3-dev cmake build-essential
```

## Build

```bash
git clone https://github.com/homer6/frequent-cron.git
cd frequent-cron
cmake .
make
make test
sudo make install
```

## Running Directly

```bash
frequent-cron run --frequency=1000 --command="/path/to/your/script.sh" --pid-file=/var/run/frequent-cron.pid
```

## Managing Services

```bash
# Register a service (also creates a systemd unit file)
sudo frequent-cron install myservice --frequency=1000 --command="/path/to/script.sh"

# This creates: /etc/systemd/system/frequent-cron-myservice.service
# For non-root: ~/.config/systemd/user/frequent-cron-myservice.service

# Start/stop/status
frequent-cron start myservice
frequent-cron status
frequent-cron stop myservice

# View logs
frequent-cron logs myservice

# Remove (also removes the systemd unit)
frequent-cron remove myservice
```

## Manual systemd Setup

If you prefer to manage the unit file yourself:

1. Create a unit file at `/etc/systemd/system/frequent-cron.service`:

   ```ini
   [Unit]
   Description=frequent-cron
   After=network.target

   [Service]
   Type=forking
   ExecStart=/usr/local/bin/frequent-cron run --frequency=1000 --command=/path/to/your/script.sh --pid-file=/var/run/frequent-cron.pid
   PIDFile=/var/run/frequent-cron.pid
   Restart=on-failure

   [Install]
   WantedBy=multi-user.target
   ```

2. Enable and start:
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable frequent-cron
   sudo systemctl start frequent-cron
   ```

3. To stop:
   ```bash
   sudo systemctl stop frequent-cron
   ```

## Legacy init.d Setup

An init.d script template is available at `init_script.tpl` in the repository root. Copy and configure it:

```bash
sudo cp init_script.tpl /etc/init.d/frequent_service
# Edit COMMAND, FREQUENCY, and PIDFILE
sudo chmod +x /etc/init.d/frequent_service
sudo /etc/init.d/frequent_service start
```

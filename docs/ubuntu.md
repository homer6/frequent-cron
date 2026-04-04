# Ubuntu/Debian Installation and Setup

## Dependencies

```bash
sudo apt-get install libboost-system-dev libboost-program-options-dev cmake build-essential
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
./frequent-cron --frequency=1000 --command="/path/to/your/script.sh" --pid-file=/var/run/frequent-cron.pid
```

## Running as an init.d Service

1. Copy the template:
   ```bash
   sudo cp init_script.tpl /etc/init.d/frequent_service
   ```

2. Edit `/etc/init.d/frequent_service` and set `COMMAND`, `FREQUENCY`, and `PIDFILE` (use absolute paths).

3. Enable and start:
   ```bash
   sudo chmod +x /etc/init.d/frequent_service
   sudo /etc/init.d/frequent_service start
   ```

4. Optionally, enable auto-start on boot:
   ```bash
   sudo update-rc.d frequent_service defaults
   ```

5. To stop:
   ```bash
   sudo /etc/init.d/frequent_service stop
   ```

## Running as a systemd Service

1. Create a unit file at `/etc/systemd/system/frequent-cron.service`:

   ```ini
   [Unit]
   Description=frequent-cron
   After=network.target

   [Service]
   Type=forking
   ExecStart=/usr/local/bin/frequent-cron --frequency=1000 --command=/path/to/your/script.sh --pid-file=/var/run/frequent-cron.pid
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

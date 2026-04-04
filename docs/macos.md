# macOS Installation and Setup

## Dependencies

```bash
brew install boost cmake
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
./frequent-cron --frequency=1000 --command="/path/to/your/script.sh" --pid-file=/tmp/frequent-cron.pid
```

## Running as a launchd Service

1. Create a plist file at `~/Library/LaunchAgents/com.frequent-cron.myjob.plist`:

   ```xml
   <?xml version="1.0" encoding="UTF-8"?>
   <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
   <plist version="1.0">
   <dict>
       <key>Label</key>
       <string>com.frequent-cron.myjob</string>
       <key>ProgramArguments</key>
       <array>
           <string>/usr/local/bin/frequent-cron</string>
           <string>--frequency=1000</string>
           <string>--command=/path/to/your/script.sh</string>
           <string>--pid-file=/tmp/frequent-cron-myjob.pid</string>
       </array>
       <key>RunAtLoad</key>
       <true/>
       <key>KeepAlive</key>
       <true/>
   </dict>
   </plist>
   ```

2. Load and start:
   ```bash
   launchctl load ~/Library/LaunchAgents/com.frequent-cron.myjob.plist
   ```

3. To stop and unload:
   ```bash
   launchctl unload ~/Library/LaunchAgents/com.frequent-cron.myjob.plist
   ```

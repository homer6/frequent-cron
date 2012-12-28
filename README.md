Overview
------------
frequent-cron is a linux daemon under the MIT License. It is designed to run crons by millisecond in linux.
Calls to the script or commands block, meaning that if you have a 500ms frequent-cron and your script
runs for 3 minutes, your script will run once every 3 minutes.


Dependencies
------------

  - Boost 1.37 (apt-get install libboost-all-dev)
  - cmake 2.8.2 (apt-get install cmake)


Installation
------------

  - git clone https://github.com/homer6/frequent-cron.git
  - cd frequent-cron
  - cmake .
  - make


Starting the Service (Simple)
-----------------------------
  - ./frequent-cron --frequency=1000 --command="/usr/local/bin/php /home/ssperandeo/dev/homer6/frequent-cron/test.php"


Stopping the Service (Simple)
-----------------------------
  - ps aux | grep frequent
  - kill 3423



Starting the Service (Using init.d)
-----------------------------------
  - sudo cp init_script.tpl /etc/init.d/frequent_service
  - edit "command", "frequency" and "pid-file" (make sure both the "command" and the "pid-file" are absolute paths)
  - sudo chmod ugo+x /etc/init.d/frequent_service
  - sudo update-rc.d frequent_service defaults  (optional; will automatically restart this service on system restart)
  - sudo /etc/init.d/frequent_service start


Stopping the Service (Using init.d)
----------------------------------
  - sudo /etc/init.d/frequent_service stop





Overview
------------
frequent-cron is a linux daemon under the MIT License. It is designed to run crons by millisecond in linux.
Calls to the script or commands are synchronous, meaning that if you have a 500ms frequent-cron and your script
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


Starting the Service
--------------------
  - ./frequent-cron --frequency=1000 --command="/usr/local/bin/php /home/ssperandeo/dev/homer6/frequent-cron/test.php"


Stopping the Service
--------------------
  - ps aux | grep frequent
  - kill 3423



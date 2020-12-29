#!/bin/sh
if test -f tmp/save_pid.txt; then
   # with this call the gpio pins get released
   curl http://localhost:8080/stop
   kill -9 `cat tmp/save_pid.txt`
   rm tmp/save_pid.txt
else
   nohup python3 webservice.py 8080 start > tmp/webservice.log 2>&1 &
   echo $! > tmp/save_pid.txt
fi


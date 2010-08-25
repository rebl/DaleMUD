#!/bin/sh
while [ 1 -eq 1 ]
 do
 ps -x >check
 if [ `grep -c "tf 7777" check` = 0 ]
   then
   date >>CRASH.LOG
   tail -40 sys_log >> CRASH.LOG
	if [ -f output.log.8 ]; then mv -f output.log.8 output.log.9; fi
	if [ -f output.log.7 ]; then mv -f output.log.7 output.log.8; fi
	if [ -f output.log.6 ]; then mv -f output.log.6 output.log.7; fi
	if [ -f output.log.5 ]; then mv -f output.log.5 output.log.6; fi
	if [ -f output.log.4 ]; then mv -f output.log.4 output.log.5; fi
	if [ -f output.log.3 ]; then mv -f output.log.3 output.log.4; fi
	if [ -f output.log.2 ]; then mv -f output.log.2 output.log.3; fi
	if [ -f output.log.1 ]; then mv -f output.log.1 output.log.2; fi
	if [ -f lib/output.log ]; then mv -f lib/output.log output.log.1; fi

   tail -n 50 CRASH.LOG|mail -s "REAL SD Crash Log" wilsonm
   cp sys_log syslog.bak
   if [ -f dmserver ]
     then
     echo "--- SWITCHING TO NEW TF VERSION ----" >> sys_log
     echo "--- SWITCHING TO NEW TF VERSION ----" >> CRASH.LOG
     mv tf tf.old
     mv dmserver tf
     chmod 770 tf
   fi
   gdb tf < gdb_command_list &
   sleep 500
 else
   sleep 30
 fi
done

RT POSIX 1003.1b features:

X	queueing signals
	priority scheduling hooks
X	interval timers
	clocks
X	message queues
X	semaphores
	asynchronous I/O
	synchronous I/O
	memory (protection, locking, mapping, shared)

POSIX 1003.1 threads features:

X	basic thread create/destroy
	condition variable synchronization


Examples and which POSIX 1003.1b feature(s) they use:

simple_sat_downlink.c => queueing signals, semaphores
posix_sw_wd.c => interval timers
posix_combined_demo.c, posix_vxw_demo.c, posix_solaris_demo.c => 
  queueing signals, semaphores
posix_mq.c => message queues


Examples and which POSIX 1003.1 threads feature(s) they use:

posix_solaris_demo.c => basic thread creat/destroy


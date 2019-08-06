-- Scenario:
-- We should never wait for syncrep while handling a USR1 signal. This is because waiting for syncrep may involve
-- waiting on a latch that may depend on a USR1 for unblocking. We can end up waiting for syncrep if we receive a TERM
-- while we are handling a USR1 and we have a transaction on backend exit. We can ensure that we have such a transaction
-- if we have temporary tables to be cleaned up.

1: CREATE TEMP TABLE tmp_syncrep_sigusr1(i int);
-- And we have recorded the backend pid
1: CREATE TABLE syncrep_sigusr1_backend_pid AS SELECT pg_backend_pid();
1: \a
1: \t
1: \o syncrep_sigusr1_pid.out
1: SELECT pg_backend_pid();

-- And we arrange to suspend backend 1's SIGUSR1 handler upon entry. ("infinite_loop" fault type does
-- not block signals).
2: SELECT gp_inject_fault2('procsignal_sigusr1_handler_start', 'infinite_loop', dbid, hostname, port)
    FROM gp_segment_configuration WHERE role='p' AND content=-1;

-- When we send a SIGUSR1 to backend 1
2: \! cat ./syncrep_sigusr1_pid.out | xargs kill -SIGUSR1
-- And we follow it up with a SIGTERM in order to
2: \! cat ./syncrep_sigusr1_pid.out | xargs kill -SIGTERM

2: SELECT gp_inject_fault2('procsignal_sigusr1_handler_start', 'reset', dbid, hostname, port)
   FROM gp_segment_configuration WHERE role='p' AND content=-1;

2: SELECT (count(*) == 0) AS backend_dead
    FROM pg_stat_activity WHERE procpid == (SELECT * FROM syncrep_sigusr1_backend_pid);


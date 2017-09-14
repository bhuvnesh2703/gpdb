-- Test the built-in gp_dump_query function.
SELECT gp_dump_query_oids('SELECT 123');
SELECT gp_dump_query_oids('SELECT * FROM pg_proc');
SELECT gp_dump_query_oids('SELECT length(proname) FROM pg_proc');

-- with EXPLAIN
SELECT gp_dump_query_oids('explain SELECT length(proname) FROM pg_proc');

-- with a multi-query statement
SELECT gp_dump_query_oids('SELECT length(proname) FROM pg_proc; SELECT abs(relpages) FROM pg_class');

-- Test error reporting on an invalid query.
SELECT gp_dump_query_oids('SELECT * FROM nonexistent_table');
SELECT gp_dump_query_oids('SELECT with syntax error');

-- Test partition and inherited tables
CREATE TABLE minirepro_partition_test (id int, info json);
CREATE TABLE foo (id int, year int, a int, b int, c int, d int, region text)
 DISTRIBUTED BY (id)
 PARTITION BY RANGE (year)
     SUBPARTITION BY RANGE (a)
        SUBPARTITION TEMPLATE (
         START (1) END (2) EVERY (1),
         DEFAULT SUBPARTITION other_a )
            SUBPARTITION BY RANGE (b)
               SUBPARTITION TEMPLATE (
               START (1) END (2) EVERY (1),
               DEFAULT SUBPARTITION other_b )
( START (2002) END (2003) EVERY (1),
   DEFAULT PARTITION outlying_years );
CREATE TABLE ptable (c1 text, c2 float);
CREATE TABLE ctable (c3 char(2)) INHERITS (ptable);
CREATE TABLE cctable (c4 char(2)) INHERITS (ctable);

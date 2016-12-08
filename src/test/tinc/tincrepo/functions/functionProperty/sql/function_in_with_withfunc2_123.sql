-- @executemode ORCA_PLANNER_DIFF
-- @description function_in_with_withfunc2_123.sql
-- @db_name functionproperty
-- @author tungs1
-- @modified 2013-04-03 12:00:00
-- @created 2013-04-03 12:00:00
-- @tags functionProperties 
WITH v(a, b) AS (SELECT func1_read_setint_sql_stb(func2_sql_int_vol(a)), b FROM foo WHERE b < 5) SELECT v1.a, v2.b FROM v AS v1, v AS v2 WHERE v1.a < v2.a order by v1.a, v2.b;  

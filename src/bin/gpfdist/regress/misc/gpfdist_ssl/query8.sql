DROP TABLE IF EXISTS tbl_on_heap;
CREATE TABLE tbl_on_heap (
            s1 text, s2 text, s3 text, dt timestamp,
            n1 smallint, n2 integer, n3 bigint, n4 decimal,
            n5 numeric, n6 real, n7 double precision);
DROP EXTERNAL TABLE IF EXISTS tbl_2;
CREATE EXTERNAL TABLE tbl_2 (s1 text, s2 text, s3 text, dt timestamp,n1 smallint, n2 integer, n3 bigint, n4 decimal, n5 numeric, n6 real, n7 double precision)
LOCATION ('gpfdists://127.0.0.1:8080/gpfdist_ssl/data/tbl1.tbl')
FORMAT 'TEXT' (DELIMITER '|' );
INSERT INTO tbl_on_heap SELECT * FROM tbl_2;
SELECT * FROM tbl_on_heap ORDER BY s1;

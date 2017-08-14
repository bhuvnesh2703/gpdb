-- Install a helper function to inject faults, using the fault injection
-- mechanism built into the server.
CREATE FUNCTION gp_inject_fault(
  faultname text,
  type text,
  ddl text,
  database text,
  tablename text,
  numoccurrences int4,
  sleeptime int4,
  db_id int4)
RETURNS boolean
AS '$libdir/gp_inject_fault'
LANGUAGE C VOLATILE STRICT NO SQL;

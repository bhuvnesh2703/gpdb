select oid, conname,connamespace, contype, condeferrable, condeferred, conrelid, contypid, confrelid, confupdtype, confdeltype, confmatchtype, conkey, confkey from pg_constraint where oid<10000 order by oid;
select conname, contype, condeferrable, condeferred, conrelid, confrelid, confupdtype, confdeltype, confmatchtype, conkey, confkey from pg_constraint order by conname;

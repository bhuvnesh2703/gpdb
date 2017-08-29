--
-- xml.sql
--
-- This file is based on src/test/regress/sql/xml.sql from the PostgreSQL 9.1
-- distributon.  The primary differences between this test and the original are
-- as follows:
--
-- 1. tests for xmlconcat, xmlelement, xmlattributes, xmlpi, xmlroot, 
--    xmlparse, xmlserialize, IS DOCUMENT, IS CONTENT, SET XML OPTION
--    were removed since only XPath function support was added to GPDB.
--
-- 2. DROP TABLE commands were added in places to make the test more hygienic.
--
DROP TABLE IF EXISTS xmltest;
CREATE TABLE xmltest (
    id int,
    data xml
);

-- Test copy with gp_strict_xml_parse GUC
drop TABLE IF EXISTS xmla;
create TABLE xmla as select (xpath('//foo/text()'::text, '<foo>bar</foo>'::xml))[1] as cx1 distributed randomly ;
insert into xmla values ('<?xml version="1.0"?><note><to>Tove</to><body>hello</body></note>' ) ;
insert into xmla values ('<?xml version="1.0"?><!DOCTYPE note SYSTEM "note.dtd"><note><to>Tove</to><body>hello</body></note>' ) ;

drop TABLE IF EXISTS xmlb;
create TABLE xmlb (like xmla) distributed randomly;
copy xmla to '/tmp/xmla.xml';
copy xmlb from '/tmp/xmla.xml';

SET gp_strict_xml_parse TO FALSE;
set gp_vmem_idle_resource_timeout=1;
copy xmlb from '/tmp/xmla.xml';
select * from xmlb;
copy xmlb from '/tmp/xmla.xml';
set gp_vmem_idle_resource_timeout=600000;
SET gp_strict_xml_parse TO TRUE;

-- Test normal insert with gp_strict_xml_parse GUC
INSERT INTO xmltest VALUES (0, 'content_only');
SET gp_strict_xml_parse TO FALSE;
INSERT INTO xmltest VALUES (0, 'content_only');
SELECT * FROM xmltest;
TRUNCATE TABLE xmltest;
SET gp_strict_xml_parse TO TRUE;

INSERT INTO xmltest VALUES (1, '<value>one</value>');
INSERT INTO xmltest VALUES (2, '<value>two</value>');
INSERT INTO xmltest VALUES (3, '<wrong');
SELECT * FROM xmltest;

SELECT xmlcomment('test');
SELECT xmlcomment('-test');
SELECT xmlcomment('test-');
SELECT xmlcomment('--test');
SELECT xmlcomment('te st');

SELECT xmlconcat2(xmlcomment('hello'),
                 xmlcomment('world'));

SELECT xmlconcat2('hello', 'you');
SELECT xmlconcat2(1, 2);
SELECT xmlconcat2('bad', '<syntax');
SELECT xmlconcat2('<foo/>', '<?xml version="1.1" standalone="no"?><bar/>');
SELECT xmlconcat2('<?xml version="1.1"?><foo/>', '<?xml version="1.1" standalone="no"?><bar/>');
SELECT xmlconcat2('<?xml version="1.1"?><foo/>', NULL);
SELECT xmlconcat2(NULL, NULL);

SELECT xml_is_well_formed_document('<foo>bar</foo><bar>foo</bar>');
SELECT xml_is_well_formed_document('<abc/>');
SELECT xml_is_well_formed_document('abc');
SELECT xml_is_well_formed_document('<>');

-- order by to make input to aggregate deterministic
SELECT xmlagg(data order by text(data)) from xmltest;

SELECT xmlagg(data) FROM xmltest WHERE id > 10;


-- Text XPath expressions evaluation

SELECT xpath('/value', data) FROM xmltest;
SELECT xpath(NULL, NULL) IS NULL FROM xmltest;
SELECT xpath('', '<!-- error -->');
SELECT xpath('//text()', '<local:data xmlns:local="http://127.0.0.1"><local:piece id="1">number one</local:piece><local:piece id="2" /></local:data>');
SELECT xpath('//loc:piece/@id', '<local:data xmlns:local="http://127.0.0.1"><local:piece id="1">number one</local:piece><local:piece id="2" /></local:data>', ARRAY[ARRAY['loc', 'http://127.0.0.1']]);
SELECT xpath('//b', '<a>one <b>two</b> three <b>etc</b></a>');

-- Test xpath_exists
SELECT xpath_exists('//town[text() = ''Toronto'']','<towns><town>Bidford-on-Avon</town><town>Cwmbran</town><town>Bristol</town></towns>'::xml);
SELECT xpath_exists('//town[text() = ''Cwmbran'']','<towns><town>Bidford-on-Avon</town><town>Cwmbran</town><town>Bristol</town></towns>'::xml);

INSERT INTO xmltest VALUES (4, '<menu><beers><name>Budvar</name><cost>free</cost><name>Carling</name><cost>lots</cost></beers></menu>'::xml);
INSERT INTO xmltest VALUES (5, '<menu><beers><name>Molson</name><cost>free</cost><name>Carling</name><cost>lots</cost></beers></menu>'::xml);
INSERT INTO xmltest VALUES (6, '<myns:menu xmlns:myns="http://myns.com"><myns:beers><myns:name>Budvar</myns:name><myns:cost>free</myns:cost><myns:name>Carling</myns:name><myns:cost>lots</myns:cost></myns:beers></myns:menu>'::xml);
INSERT INTO xmltest VALUES (7, '<myns:menu xmlns:myns="http://myns.com"><myns:beers><myns:name>Molson</myns:name><myns:cost>free</myns:cost><myns:name>Carling</myns:name><myns:cost>lots</myns:cost></myns:beers></myns:menu>'::xml);

SELECT COUNT(id) FROM xmltest WHERE xpath_exists('/menu/beer',data);
SELECT COUNT(id) FROM xmltest WHERE xpath_exists('/menu/beers',data);
SELECT COUNT(id) FROM xmltest WHERE xpath_exists('/menu/beers/name[text() = ''Molson'']',data);
SELECT COUNT(id) FROM xmltest WHERE xpath_exists('/myns:menu/myns:beer',data,ARRAY[ARRAY['myns','http://myns.com']]);
SELECT COUNT(id) FROM xmltest WHERE xpath_exists('/myns:menu/myns:beers',data,ARRAY[ARRAY['myns','http://myns.com']]);
SELECT COUNT(id) FROM xmltest WHERE xpath_exists('/myns:menu/myns:beers/myns:name[text() = ''Molson'']',data,ARRAY[ARRAY['myns','http://myns.com']]);

DROP TABLE IF EXISTS query;
CREATE TABLE query ( expr TEXT );

INSERT INTO query VALUES ('/menu/beers/cost[text() = ''lots'']');
SELECT COUNT(id) FROM xmltest, query WHERE xpath_exists(expr, data);

-- Test xml_is_well_formed and variants

SELECT xml_is_well_formed_document('<foo>bar</foo>');
SELECT xml_is_well_formed_document('abc');
SELECT xml_is_well_formed_content('<foo>bar</foo>');
SELECT xml_is_well_formed_content('abc');

SELECT xml_is_well_formed('abc');
SELECT xml_is_well_formed('<>');
SELECT xml_is_well_formed('<abc/>');
SELECT xml_is_well_formed('<foo>bar</foo>');
SELECT xml_is_well_formed('<foo>bar</foo');
SELECT xml_is_well_formed('<foo><bar>baz</foo>');
SELECT xml_is_well_formed('<local:data xmlns:local="http://127.0.0.1"><local:piece id="1">number one</local:piece><local:piece id="2" /></local:data>');
SELECT xml_is_well_formed('<pg:foo xmlns:pg="http://postgresql.org/stuff">bar</my:foo>');
SELECT xml_is_well_formed('<pg:foo xmlns:pg="http://postgresql.org/stuff">bar</pg:foo>');

DROP TABLE query;
DROP TABLE xmltest;

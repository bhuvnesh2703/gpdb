select
    linear_interpolate('2010-01-05T20:40:00'::timestamp, '2010-01-03T02:00:00'::timestamp, 2050::int4, '2010-01-02T09:20:00'::timestamp, 2000::int4),
     2250::int4 as answer,
     2250::int4 = linear_interpolate('2010-01-05T20:40:00'::timestamp, '2010-01-03T02:00:00'::timestamp, 2050::int4, '2010-01-02T09:20:00'::timestamp, 2000::int4) as match ;

select
    linear_interpolate(600::numeric, 100::numeric, 2000::int4, 200::numeric, 2050::int4),
     2250::int4 as answer,
     2250::int4 = linear_interpolate(600::numeric, 100::numeric, 2000::int4, 200::numeric, 2050::int4) as match ;

select
    linear_interpolate(200::float8, 100::float8, 2000::int4, 600::float8, 2250::int4),
     2050::int4 as answer,
     2050::int4 = linear_interpolate(200::float8, 100::float8, 2000::int4, 600::float8, 2250::int4) as match ;

select
    linear_interpolate('85 minutes'::interval, '65 minutes'::interval, 2000::int4, '165 minutes'::interval, 2250::int4),
     2050::int4 as answer,
     2050::int4 = linear_interpolate('85 minutes'::interval, '65 minutes'::interval, 2000::int4, '165 minutes'::interval, 2250::int4) as match ;

select
    linear_interpolate('2010-01-05T20:40:00'::timestamptz, '2010-01-03T02:00:00'::timestamptz, 2050::int4, '2010-01-02T09:20:00'::timestamptz, 2000::int4),
     2250::int4 as answer,
     2250::int4 = linear_interpolate('2010-01-05T20:40:00'::timestamptz, '2010-01-03T02:00:00'::timestamptz, 2050::int4, '2010-01-02T09:20:00'::timestamptz, 2000::int4) as match ;

select
    linear_interpolate('10:45:00'::time, '10:25:00'::time, 2050::int4, '10:20:00'::time, 2000::int4),
     2250::int4 as answer,
     2250::int4 = linear_interpolate('10:45:00'::time, '10:25:00'::time, 2050::int4, '10:20:00'::time, 2000::int4) as match ;

select
    linear_interpolate(600::int4, 200::int4, '2012-06-05T04:00:00'::timestamp, 100::int4, '2012-06-01T16:40:00'::timestamp),
     '2012-06-19T01:20:00'::timestamp as answer,
     '2012-06-19T01:20:00'::timestamp = linear_interpolate(600::int4, 200::int4, '2012-06-05T04:00:00'::timestamp, 100::int4, '2012-06-01T16:40:00'::timestamp) as match ;

select
    linear_interpolate(600::int4, 200::int4, 2050::float4, 100::int4, 2000::float4),
     2250::float4 as answer,
     2250::float4 = linear_interpolate(600::int4, 200::int4, 2050::float4, 100::int4, 2000::float4) as match ;

select
    linear_interpolate(600::int4, 100::int4, '2010-01-21'::date, 200::int4, '2010-01-31'::date),
     '2010-03-12'::date as answer,
     '2010-03-12'::date = linear_interpolate(600::int4, 100::int4, '2010-01-21'::date, 200::int4, '2010-01-31'::date) as match ;


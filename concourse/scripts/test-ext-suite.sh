#!/bin/bash
set -eo pipefail

GPDB_ENV_CMD="source ~/.bash_profile && source /usr/local/gpdb/greenplum_path.sh"

start_ssh() {
    echo "Start ssh service.."
    service ssh start
}

untar_gpdb_src() {
    echo "Untar gpdb_src..."
    mkdir -p gpdb_src && tar -xf gpdb_tarball/gpdb_src.tar.gz -C gpdb_src --strip 1
    echo "Change ownnership of gpdb_src to gpadmin"
    chown -R gpadmin:gpadmin gpdb_src
}

copy_ms_suite() {
    echo "Copy ms-suite to src/test directory.."
    cp -r gporca-misc/ms-suite gpdb_src/src/test/
    echo "Change ownnership of gpdb_src to gpadmin"
    chown -R gpadmin:gpadmin gpdb_src/src/test/$TEST_SUITE
}
configure_gpdb() {
    pushd gpdb_src
    ./configure --with-libxml --with-python --without-zstd --with-libs=/usr/local/gpdb/lib --with-includes=/usr/local/gpdb/include --prefix=/usr/local/gpdb
    chown -R gpadmin:gpadmin .
    popd
}

remove_old_gpdb_binary() {
    echo "Remove old gpdb binaries from /usr/local/gpdb"
	rm -rf /usr/local/gpdb/*
}

untar_new_gpdb_binary() {
    echo "Place new gpdb binaries to /usr/local/gpdb"
	tar -xvzf bin_gpdb/bin_gpdb_with_orca_centos6.tar.gz -C /usr/local/gpdb
	chown -R gpadmin:gpadmin /usr/local/gpdb
}

start_gpdb() {
	su gpadmin -c "$GPDB_ENV_CMD && gpstart -a"
}

make_regress() {
    pushd gpdb_src/src/test/regress
    su gpadmin -c "make"
    popd
}

start_test_suite() {
        pushd gpdb_src/src/test/$TEST_SUITE
        su gpadmin -c "$GPDB_ENV_CMD && env PGOPTIONS=\"$PGOPTIONS\" ../regress/pg_regress --use-existing --init-file=init_file --schedule=./$SCHEDULE_FILE --dbname=mrvdb || true"
        popd
}


_main() {
    start_ssh
    untar_gpdb_src
    copy_ms_suite
    remove_old_gpdb_binary
    untar_new_gpdb_binary
    configure_gpdb
    start_gpdb
    make_regress
    start_test_suite
    if [ -f "gpdb_src/src/test/$TEST_SUITE/regression.diffs" ]; then
        cp gpdb_src/src/test/$TEST_SUITE/regression.diffs icg_output/
        cp gpdb_src/src/test/$TEST_SUITE/regression.out icg_output/
        cat gpdb_src/src/test/$TEST_SUITE/regression.diffs
        exit 1
    fi
}

_main "$@"


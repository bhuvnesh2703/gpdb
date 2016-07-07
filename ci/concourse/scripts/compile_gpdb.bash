#!/bin/bash -l

set -exo pipefail

CWDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "${CWDIR}/common.bash"

function prep_env_for_centos() {
  source /opt/gcc_env.sh
  ln -s "$(pwd)/gpdb_src/gpAux/ext/rhel5_x86_64/python-2.6.2" /opt
  export JAVA_HOME=/usr/lib/jvm/java-1.6.0-openjdk-1.6.0.39.x86_64
  export PATH=${JAVA_HOME}/bin:${PATH}
}

function prep_env_for_sles() {
  source /opt/gcc_env.sh
  ln -s "$(pwd)/gpdb_src/gpAux/ext/suse11_x86_64/python-2.6.2" /opt
  export JAVA_HOME=/usr/lib64/jvm/java-1.6.0-openjdk-1.6.0
  export PATH=${JAVA_HOME}/bin:${PATH}
}

function make_sync_tools() {
  pushd gpdb_src/gpAux
    make sync_tools
    tar -czf ../../sync_tools_gpdb/sync_tools_gpdb.tar.gz ext
  popd
}

function build_gpdb() {
  pushd gpdb_src/gpAux
    make GPROOT=/usr/local dist
  popd
}

function unittest_check_gpdb() {
  pushd gpdb_src/gpAux
    make GPROOT=/usr/local unittest-check
  popd
}

function export_gpdb() {
  TARBALL=$(pwd)/bin_gpdb/bin_gpdb.tar.gz
  pushd /usr/local/greenplum-db-devel
    source greenplum_path.sh
    python -m compileall -x test .
    chmod -R 755 .
    tar -czf "${TARBALL}" ./*
  popd
}

function _main() {
  case "$TARGET_OS" in
    centos)
      prep_env_for_centos
      ;;
    sles)
      prep_env_for_sles
      ;;
    *)
      echo "only centos and sles are supported TARGET_OS'es"
      false
      ;;
  esac

  make_sync_tools
  build_gpdb
  unittest_check_gpdb
  export_gpdb
}

_main "$@"

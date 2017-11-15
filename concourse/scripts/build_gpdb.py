#!/usr/bin/python2

import optparse
import os
import subprocess
import sys
import shutil
import stat

from builds.GpBuild import GpBuild

CODEGEN_MODE = 'codegen'
ORCA_CODEGEN_DEFAULT_MODE = "orca_codegen"
ORCA_MODE = 'orca'
PLANNER_MODE = 'planner'
INSTALL_DIR = "/usr/local/gpdb"


def copy_installed(output_dir):
    if os.path.normpath(INSTALL_DIR) != os.path.normpath(output_dir):
        status = subprocess.call("mkdir -p " + output_dir, shell=True)
        if status:
            return status
        return subprocess.call("cp -r %s/*  %s" % (INSTALL_DIR, output_dir), shell=True)
    return


def print_compiler_version():
    status = subprocess.call(["g++", "--version"])
    if status:
        return status
    return subprocess.call(["gcc", "--version"])

def create_gpadmin_user():
    status = subprocess.call("gpdb_src/concourse/scripts/setup_gpadmin_user.bash")
    os.chmod('/bin/ping', os.stat('/bin/ping').st_mode | stat.S_ISUID)
    if status:
        return status


def copy_output():
    for dirpath, dirs, diff_files in os.walk('gpdb_src/'):
        if 'regression.diffs' in diff_files:
            diff_file = dirpath + '/' + 'regression.diffs'
            print(  "======================================================================\n" +
                    "DIFF FILE: " + diff_file+"\n" +
                    "----------------------------------------------------------------------")
            with open(diff_file, 'r') as fin:
                print fin.read()
    shutil.copyfile("gpdb_src/src/test/regress/regression.diffs", "icg_output/regression.diffs")
    shutil.copyfile("gpdb_src/src/test/regress/regression.out", "icg_output/regression.out")


def main():
    parser = optparse.OptionParser()
    parser.add_option("--build_type", dest="build_type", default="RELEASE")
    parser.add_option("--mode", choices=['orca', 'planner'])
    parser.add_option("--compiler", dest="compiler")
    parser.add_option("--cxxflags", dest="cxxflags")
    parser.add_option("--output_dir", dest="output_dir", default=INSTALL_DIR)
    parser.add_option("--configure-option", dest="configure_option", action="append", help="Configure flags, \
                                                                                            ex --configure_option=--disable-orca --configure_option=--disable-gpcloud")
    parser.add_option("--gcc-env-file", dest="gcc_env_file", help="GCC env file to be sourced")
    parser.add_option("--install-orca-in-gpdb-location", dest="install_orca_in_gpdb_location", action="store_true", help="Instal ORCA header and library files in GPDB install directory")
    parser.add_option("--required-action", choices=['build', 'test'], help="Build GPDB or Run Install Check")
    parser.add_option("--gpdb_name", dest="gpdb_name")
    (options, args) = parser.parse_args()



    ci_common = GpBuild(options.mode)
    status = print_compiler_version()
    if status:
        return status

    ci_common.set_gcc_env_file(options.gcc_env_file)

    if options.required-action == 'build':
        install_dir = INSTALL_DIR if options.install_orca_in_gpdb_location else "/usr/local"
        for dependency in args:
            status = ci_common.install_dependency(dependency, install_dir)
            if status:
                return status
    else if options.required-action == 'test':
        status = ci_common.install_dependency(options.gpdb_name, INSTALL_DIR)
        if status:
            return status

    configure_option = []
    if options.configure_option:
        configure_option.extend(options.configure_option)

    if options.install_orca_in_gpdb_location:
        configure_option.append("--with-libs={0}".format(os.path.join(install_dir, "lib")))
        configure_option.append("--with-includes={0}".format(os.path.join(install_dir, "include")))
    ci_common.append_configure_options(configure_option)

    status = ci_common.configure()
    if status:
        return status

    if options.required-action == 'build':
        status = ci_common.make()
        if status:
            return status

        status = ci_common.unittest()
        if status:
            return status

        status = ci_common.make_install()
        if status:
            return status

        status = copy_installed(options.output_dir)
        if status:
            return status
    else if options.required-action == 'test':
        status = create_gpadmin_user()
        if status:
            return status
        if os.getenv("TEST_SUITE", "icg") == 'icw':
          status = ciCommon.install_check('world')
        else:
          status = ciCommon.install_check()
        if status:
            copy_output()
        return status
        
    return 0


if __name__ == "__main__":
    sys.exit(main())

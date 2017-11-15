#!/usr/bin/python2

import optparse
import os
import subprocess
import sys

from builds.GpBuild import GpBuild

CODEGEN_MODE = 'codegen'
ORCA_CODEGEN_DEFAULT_MODE = "orca_codegen"
ORCA_MODE = 'orca'
PLANNER_MODE = 'planner'
INSTALL_DIR = "/usr/local/gpdb"


def make(num_cpus, gcc_env_file):
    return subprocess.call(["source", gcc_env_file, "&&", "make",
                            "-j" + str(num_cpus),
                            "-l" + str(2 * num_cpus),
                            ],
                           cwd="gpdb_src")


def install():
    return subprocess.call("make install", cwd="gpdb_src", shell=True)


def copy_installed(output_dir):
    if os.path.normpath(INSTALL_DIR) != os.path.normpath(output_dir):
        status = subprocess.call("mkdir -p " + output_dir, shell=True)
        if status:
            return status
        return subprocess.call("cp -r %s/*  %s" % (INSTALL_DIR, output_dir), shell=True)
    return


def unittest():
    return subprocess.call("make -s unittest-check", cwd="gpdb_src/src/backend", shell=True)


def print_compiler_version():
    status = subprocess.call(["g++", "--version"])
    if status:
        return status
    return subprocess.call(["gcc", "--version"])


def main():
    parser = optparse.OptionParser()
    parser.add_option("--build_type", dest="build_type", default="RELEASE")
    parser.add_option("--mode", choices=[ORCA_MODE, CODEGEN_MODE, ORCA_CODEGEN_DEFAULT_MODE, PLANNER_MODE], default=ORCA_CODEGEN_DEFAULT_MODE)
    parser.add_option("--compiler", dest="compiler")
    parser.add_option("--cxxflags", dest="cxxflags")
    parser.add_option("--output_dir", dest="output_dir", default=INSTALL_DIR)
    parser.add_option("--configure_options", dest="configure_options", help="Comma separated configure flags, \
                                                                            ex: '--disable-gpcloud, --disable-orca'")
    parser.add_option("--gcc_env_file", dest="gcc_env_file", help="GCC env file to be sourced")
    (options, args) = parser.parse_args()
    ci_common = GpBuild(ORCA_CODEGEN_DEFAULT_MODE)
    if options.mode == ORCA_MODE:
        ci_common = GpBuild(options.mode)
    elif options.mode == PLANNER_MODE:
        ci_common = GpBuild(options.mode)

    for dependency in args:
        status = ci_common.install_dependency(dependency)
        if status:
            return status
    status = print_compiler_version()
    if status:
        return status

   
    ci_common.set_gcc_env_file(options.gcc_env_file)
    configure_options = []
    if options.configure_options:
        configure_options = [option.strip() for option in options.configure_options.split(',')]
    ci_common.append_configure_options(configure_options)
    status = ci_common.configure()
    if status:
        return status
    status = ci_common.make()
    if status:
        return status
    status = unittest()
    if status:
        return status
    status = install()
    if status:
        return status
    status = copy_installed(options.output_dir)
    if status:
        return status
    return 0


if __name__ == "__main__":
    sys.exit(main())

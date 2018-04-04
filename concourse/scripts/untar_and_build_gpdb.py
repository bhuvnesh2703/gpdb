#!/usr/bin/env python
import subprocess

subprocess.call(["mkdir -p gpdb_src && tar -xf gpdb_tarball/gpdb_src.tar.gz -C gpdb_src --strip 1"], shell=True)
subprocess.call(["gpdb/concourse/scripts/build_gpdb.py --mode=orca --output_dir=bin_gpdb/install --action=build --configure-option=--disable-gpcloud --orca-in-gpdb-install-location bin_orca bin_xerces"], shell=True)
subprocess.call(["env src_root=bin_gpdb/install dst_tarball=package_tarball/bin_gpdb.tar.gz gpdb/concourse/scripts/package_tarball.bash"], shell=True)

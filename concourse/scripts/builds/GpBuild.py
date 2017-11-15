import subprocess
from GpdbBuildBase import GpdbBuildBase

INSTALL_DIR="/usr/local/gpdb"


class GpBuild(GpdbBuildBase):
    def __init__(self, mode="orca"):
        GpdbBuildBase.__init__(self)
        self.mode = 'on' if mode == 'orca' else 'off'
        self.configure_options =  [
                                    "--enable-mapreduce",
                                    "--with-gssapi",
                                    "--with-perl",
                                    "--with-libxml",
                                    "--with-python",
                                    "--prefix={0}".format(INSTALL_DIR)
                                  ]
        self.gcc_env_file = ''

    def configure(self):
        if self.mode == 'off':
            self.configure_options.append("--disable-orca")
        cmd_with_options = ["./configure"]
        cmd_with_options.extend(self.configure_options)
        source_cmd = ''
        if self.gcc_env_file:
            source_cmd = "source {0} && ".format(self.gcc_env_file)
        cmd = source_cmd + " ".join(cmd_with_options)
        return subprocess.call(cmd, shell=True, cwd="gpdb_src")

    @staticmethod
    def create_demo_cluster():
        return subprocess.call([
            "runuser gpadmin -c \"source %s/greenplum_path.sh \
            && make create-demo-cluster DEFAULT_QD_MAX_CONNECT=150\"" % INSTALL_DIR],
            cwd="gpdb_src/gpAux/gpdemo", shell=True)

    def install_check(self, option='good'):
        status = self.create_demo_cluster()
        if status:
            return status
        if option == 'world':
            return self.run_install_check_with_command('make installcheck-world')
        else:
            return self.run_install_check_with_command('make -C src/test installcheck-good')

    def run_install_check_with_command(self, make_command):
        return subprocess.call([
            "runuser gpadmin -c \"source {0}/greenplum_path.sh \
            && source gpAux/gpdemo/gpdemo-env.sh && PGOPTIONS='-c optimizer={1}' \
            {2} \"".format(INSTALL_DIR, self.mode, make_command)], cwd="gpdb_src", shell=True)

    def append_configure_options(self, configure_options):
        if len(configure_options) > 0:
            self.configure_options.extend(configure_options)

    def set_gcc_env_file(self, gcc_env_file):
        self.gcc_env_file = gcc_env_file

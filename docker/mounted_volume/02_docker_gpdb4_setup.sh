#!/bin/sh

# this file should be run as "gpadmin" user

set -e


mkdir -p ~/.ssh

rm -rf $HOME/.ssh/id_rsa
ssh-keygen -t rsa -N '' -f ~/.ssh/id_rsa

cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys

#the below echo uses multiple lines
echo 'host *
  UserKnownHostsFile /dev/null
  StrictHostKeyChecking no
' > /home/gpadmin/.ssh/config

chmod 700 /home/gpadmin/.ssh
chmod og-wx ~/.ssh/authorized_keys
chmod 600 ~/.ssh/config

#Configure environment and pull libraries
#the below echo uses multiple lines
echo '
#for gpdb4 compilation
source /opt/gcc_env.sh
export JAVA_HOME=/usr/lib/jvm/java-1.6.0-openjdk.x86_64
export PATH=${JAVA_HOME}/bin:${PATH}
export IVYREPO_HOST=repo.pivotal.io
export IVYREPO_REALM="Artifactory Realm"
export IVYREPO_USER=build_readonly
export IVYREPO_PASSWD="zlIU5Ys&U0WBKN&O"
export PYTHONPATH=/opt/python-2.6.2/lib/python2.6:$PYTHONPATH
export PYTHONHOME=/opt/python-2.6.2
export PATH=/opt/python-2.6.2/bin:$PATH

' >> ~/.bashrc

source ~/.bashrc
echo '
source ~/built-gpdb4/greenplum-db-devel/greenplum_path.sh
source ~/gpdemo/gpdemo-env.sh
' >> ~/.bashrc
cd ~/gpdb4_mount/gpAux
rm -f /opt/python-2.6.2
ln -s "/home/gpadmin/gpdb4_mount/gpAux/ext/rhel5_x86_64/python-2.6.2" /opt
make sync_tools

#=========================================================
# Give parent script a chance to save container
exit

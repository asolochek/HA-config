#!/bin/sh

#env
#whoami
#pwd
#chmod 600 /root/.ssh/id_rsa
#ls -la /root/.ssh
ssh -oStrictHostKeyChecking=no 10.3.1.2 ifconfig eth0 down
#ssh -i -oStrictHostKeyChecking=no 10.3.1.2 ifconfig

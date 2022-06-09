#!/bin/bash

relative_path="msr-driver"

if [ "$(whoami)" != "root" ] ; then
        echo -e "\n\tYou must be root to run this script.\n"
        exit 1
fi
mknod /dev/msrdrv_util c 223 0
chmod 666 /dev/msrdrv_util
insmod ${relative_path}/kernel/msrdrv_util.ko

sleep 0.5

ioctl_infofile_name="inctl.ini"
${relative_path}/usr_enable.out > ${ioctl_infofile_name}

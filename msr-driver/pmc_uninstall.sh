#!/bin/bash

relative_path="msr-driver"

if [ "$(whoami)" != "root" ] ; then
        echo -e "\n\tYou must be root to run this script.\n"
        exit 1
fi

${relative_path}/usr_disable.out

sleep 0.5

rmmod msrdrv_util
rm -f /dev/msrdrv_util

ioctl_infofile_name="inctl.ini"
rm -f ${ioctl_infofile_name}

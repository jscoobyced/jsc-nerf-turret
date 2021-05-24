#!/bin/bash

if [ $(id -u) -ne 0 ];
then
	echo "Must be run as root."
	exit -1
fi

UB=$1
CHROOTDIR="chroot-dir"
MNTDIR="mnt"

apt install -y qemu-user-static

if [ "$UB" == "" ];
then
	echo "Usage:"
	echo "sudo $0 path_to_ubuntu.img"
	exit -1
fi

if [[ "${UB}" == *".xz" ]];
then
	echo "Archive found."
	cp ${UB} /tmp/
	UB=$(basename ${UB})
	pushd /tmp
	xz --decompress ${UB}
	UB="/tmp/${UB:0:-3}"
	popd
fi

if [ -d ${CHROOTDIR} ];
then
	echo "Deleting ${CHROOTDIR}"
	rm -Rf ${CHROOTDIR}
fi

if [ -d ${MNTDIR} ];
then
	echo "Deleting ${MNTDIR}"
	rm -Rf ${MNTDIR}
fi

THEDEVICE=$(losetup -f --show -P ${UB})
THEDEVICEP2="${THEDEVICE}p2"
THEMOUNTP2=$(basename ${THEDEVICEP2})

if [ -e ${THEDEVICEP2} ];
then
	mkdir -p ${MNTDIR}/${THEMOUNTP2}
	mount ${THEDEVICEP2} ${MNTDIR}/${THEMOUNTP2}
	mkdir ${CHROOTDIR}
	cp -rp ${MNTDIR}/${THEMOUNTP2}/* ${CHROOTDIR}
	pushd ${CHROOTDIR}
	cp /usr/bin/qemu-arm-static usr/bin/
	rm etc/resolv.conf
	cp /etc/resolv.conf etc/
	cp /etc/hosts etc/
	cp ../dev-tools.sh /tmp/
	popd
	sleep 1
	umount ${MNTDIR}/${THEMOUNTP2}
fi

losetup -d ${THEDEVICE}

rm -Rf mnt

# Setup locales
chroot chroot-dir locale-gen en_US.UTF-8
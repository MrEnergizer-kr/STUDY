inherit cmake ylicense ycommon 
DESCRIPTION = "usb82642 health check thought SCSI"
LICENSE          = "YURA-Corpo-Inc-Proprietary"
LIC_FILES_CHKSUM = "file://${YMETA_PATH}/files/yura-licenses/\
${LICENSE};md5=a9ec3f5e63697d687b9706b8e774a466"


SECTION = 'dev'
#SRCREV = "${AUTOREV}"
#PV = "1.0+git${SRCPV}"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

#SRC_URI = 'git://gitlab-server/BLTN_CAM_G2/BSP/bsp-utils/sd-health-read-tools.git;user=git;protocol=ssh;destsuffix=${PN}-${PV}'
#SRC_URI = 'git://gitlab-server/BLTN_CAM_G2/BSP/bsp-utils/sd-health-read-tools.git;user=git;protocol=ssh;tag=0.3.2;destsuffix=${PN}-${PV}'
SRC_URI += 'file://sd-health-read-tools'

S = "${WORKDIR}/sd-health-read-tools" 
#S = "${WORKDIR}/sd-health-read-tools-${PV}"
OECMAKE_SOURCEPATH = "${S}/build"
OECMAKE_BUILDPATH = "${WORKDIR}/build"


FILES_${PN} += " \
   ${bindir}/* \
   ${libdir}/* \
"
FILES_${PN}-dev = " \
   ${libdir}/* \
   ${includedir}/* \
"

PACKAGES = "${PN} ${PN}-dev"

 
DEPENDS += "sg3-utils udev"
#CFLAGS += "-lpthread -lsgutils2 -ludev"



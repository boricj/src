#	$NetBSD: Makefile,v 1.26 2015/05/28 09:51:31 martin Exp $
CDBASE=		sparc64cd		# gives ${CDBASE}.iso
CDRELEASE=	true			# include $RELEASEDIR/$MACHINE
CDKERNELS=	${RELEASEDIR}/${MACHINE}/binary/kernel/netbsd-GENERIC.gz	netbsd
CD_SETS=	base etc modules

# for PRINTOBJDIR
.include <bsd.own.mk>

SYSINSTDIR!= cd ${.CURDIR}/../../../../usr.sbin/sysinst/arch/${MACHINE} && ${PRINTOBJDIR}

CDRELEASE_NOISOS=	true
CDBOOTIMAGEDIR!= cd ${NETBSDSRCDIR}/distrib/sparc64/bootfs && ${PRINTOBJDIR}
CDBOOTIMAGE=    ${CDBOOTIMAGEDIR}/boot.fs
SUN_BOOT_ARGS:=  - - - - ${CDBOOTIMAGE}

image_md_pre:
	${RM} -f cdrom/etc/gettytab cdrom/etc/ttys cdrom/etc/rc
	${HOST_LN} -fs /tmp/gettytab cdrom/etc/gettytab
	${INSTALL} ${COPY} ${.CURDIR}/etc.ttys cdrom/etc/ttys
	${INSTALL} ${COPY} ${.CURDIR}/etc.rc cdrom/etc/rc
	${INSTALL} ${COPY} -m 0555 ${.CURDIR}/install.sh cdrom/install.sh
	${MKDIR} cdrom/mnt2 cdrom/targetroot
	${INSTALL} ${COPY} ${SYSINSTDIR}/sysinstmsgs.?? cdrom/usr/share/sysinst/catalog/

image_md_post:
	SUNLABEL=${TOOL_SUNLABEL:Q} \
		${HOST_SH} ${NETBSDSRCDIR}/distrib/common/sunbootcd.sh \
		${CDIMAGE} ${SUN_BOOT_ARGS}

.include "${.CURDIR}/../../../common/Makefile.bootcd"


################################################################################
#
# hello-world
#
################################################################################

HELLO_WORLD_VERSION = 1.0
# Build from source vendored in this repo instead of a download URL.
HELLO_WORLD_SITE_METHOD = local
HELLO_WORLD_SITE = $(BR2_EXTERNAL_EXAMPLE_PATH)/src/hello-world
HELLO_WORLD_LICENSE = MIT

# sysrepo and libyang are what the daemon links against.  confd is here
# for ordering: it wipes and re-populates the build's sysrepo datastore,
# so our model has to be loaded after it has had its turn.
HELLO_WORLD_DEPENDENCIES = sysrepo libyang confd

HELLO_WORLD_YANG_REV = 2026-08-26
HELLO_WORLD_YANG_SRC = $(BR2_EXTERNAL_EXAMPLE_PATH)/yang
HELLO_WORLD_YANG_DIR = $(TARGET_DIR)/usr/share/yang/modules/example

define HELLO_WORLD_BUILD_CMDS
	$(MAKE) -C $(@D) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define HELLO_WORLD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/hello-world \
		$(TARGET_DIR)/usr/sbin/hello-world
	$(INSTALL) -D -m 0644 \
		$(HELLO_WORLD_YANG_SRC)/example-hello@$(HELLO_WORLD_YANG_REV).yang \
		$(HELLO_WORLD_YANG_DIR)/example-hello@$(HELLO_WORLD_YANG_REV).yang
endef

# Load the model into sysrepo at build time, the way Infix loads its own.
# infix/package/confd-test-mode/ is the pattern this follows: a package
# outside confd, adding one model of its own.  The SHM prefix has to be
# confd's, so srload attaches to the datastore confd just set up.
define HELLO_WORLD_INSTALL_YANG_MODULES
	SYSREPO_SHM_PREFIX=$(CONFD_SYSREPO_SHM_PREFIX) \
	SYSREPOCTL_EXECUTABLE="$(HOST_DIR)/bin/sysrepoctl" \
	SYSREPOCFG_EXECUTABLE="$(HOST_DIR)/bin/sysrepocfg" \
	SEARCH_PATH="$(HELLO_WORLD_YANG_DIR)/" \
	$(BR2_EXTERNAL_INFIX_PATH)/utils/srload \
		$(HELLO_WORLD_YANG_SRC)/example-hello.inc
endef
HELLO_WORLD_POST_INSTALL_TARGET_HOOKS += HELLO_WORLD_INSTALL_YANG_MODULES

# sysrepo keeps each model's datastore under here, group readable so the
# CLI can get at it as the logged in user.
define HELLO_WORLD_PERMISSIONS
	/etc/sysrepo/data/ r 660 root sysrepo - - - - -
	/etc/sysrepo/data  d 770 root sysrepo - - - - -
endef

$(eval $(generic-package))

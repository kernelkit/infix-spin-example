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

define HELLO_WORLD_BUILD_CMDS
	$(MAKE) -C $(@D) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define HELLO_WORLD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/hello-world \
		$(TARGET_DIR)/usr/sbin/hello-world
endef

$(eval $(generic-package))

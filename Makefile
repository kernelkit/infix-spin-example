# Build the spin by handing over to Buildroot with two br2-externals
# stacked: Infix first, then this repo (branding, overlay, packages).
export BR2_EXTERNAL := $(CURDIR)/infix:$(CURDIR)

# Run our tests instead of Infix's, using Infix's test harness, see test/
export TESTS             := $(CURDIR)/test/case/all.yaml
export NINEPM_PROJ_CONFIG := $(CURDIR)/test/9pm-proj.yaml

O      ?= $(CURDIR)/output
config := $(O)/.config
bmake   = $(MAKE) -C infix/buildroot O=$(O) $1

all: $(config) infix/buildroot/Makefile
	@+$(call bmake,$@)

$(config):
	@+$(call bmake,list-defconfigs)
	@echo "ERROR: No configuration selected."
	@echo "Please choose a configuration from the list above by running"
	@echo "'make <board>_defconfig' before building an image."
	@exit 1

%_defconfig: configs/%_defconfig
	@+$(call bmake,$@)

# Boot the image in QEMU and run $(TESTS) against it.  Needs Docker or
# Podman; the harness pulls a container with QEMU and the test tooling.
# Explicit rule, since the catch-all below would find the test/ directory
# and think there is nothing to do.
test: | infix/buildroot/Makefile
	@+$(call bmake,$@)

# Any other target (menuconfig, clean, ...) is passed to Buildroot.
%: | infix/buildroot/Makefile
	@+$(call bmake,$@)

infix/buildroot/Makefile:
	@git submodule update --init --recursive

# Regenerate configs/ from Infix's defconfigs + our snippets.  Run this
# after editing anything in config-snippets/, and commit the result.
defconfigs-generate: infix/buildroot/Makefile
	./infix/utils/generate-defconfig.sh \
		-b infix/configs/x86_64_defconfig \
		-c config-snippets/common.conf \
		-c config-snippets/x86_64.conf \
		-o configs/x86_64_defconfig

.PHONY: all test defconfigs-generate

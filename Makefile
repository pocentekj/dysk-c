CC := gcc
BUILDDIR := build

.PHONY: all setup dev check clean

all: dev

$(BUILDDIR)/build.ninja:
	CC="$(CC)" meson setup "$(BUILDDIR)"

setup:
	CC="$(CC)" meson setup --reconfigure "$(BUILDDIR)"

dev: $(BUILDDIR)/build.ninja
	meson compile -C "$(BUILDDIR)"

check: $(BUILDDIR)/build.ninja
	meson test -C "$(BUILDDIR)"

clean:
	rm -rf "$(BUILDDIR)"

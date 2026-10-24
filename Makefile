CC := gcc
BUILDDIR := build

.PHONY: all setup format dev check clean

all: dev

$(BUILDDIR)/build.ninja:
	CC="$(CC)" meson setup "$(BUILDDIR)"

setup:
	CC="$(CC)" meson setup --reconfigure "$(BUILDDIR)"

format:
	find . -type f \( -name '*.c' -o -name '*.h' \) -print0 | xargs -0 clang-format -i

dev: check
	meson compile -C "$(BUILDDIR)"

check: $(BUILDDIR)/build.ninja
	meson test -C "$(BUILDDIR)"

clean:
	rm -rf "$(BUILDDIR)" .cache

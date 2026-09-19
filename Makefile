
clean:
	rm -f bin/* && rm -f $(shell find . -name "*.o")

# The engine targets -- `make engine`, `make engine-version`, `make engine-update`
# -- belong to the repo root, and the root has this Makefile, so the rule has to
# be reachable from here. A fresh clone found the gap: README and CLAUDE.md both
# said to run `make engine`, and the root answered "No rule to make target
# 'engine'". engine.mk is included AFTER clean so the default target stays clean.
include engine.mk


# ── engine.mk — the PINNED Storm Engine v2, built from the submodule ─────────
#
# THIS REPO BUILDS AGAINST ITS SUBMODULE, ALWAYS. The engine is pinned at
# external/storm-engine-v2 so the examples name the revision they were written
# against, and so a `git clone` of this repo needs no system install at all.
#
# ⚠️ WHY THAT MATTERS HERE, measured: CI used to download the newest engine
# release with a .deb and extract it into /usr/local, and this machine had 2.3.0
# installed while the newest release was 2.3.1. A pattern could therefore compile
# locally and fail in CI, or the reverse, with nothing in either place saying
# which engine was in play. Building the pinned revision removes that whole class
# of surprise.
#
# Requires ONE nested submodule: the engine compiles tinyxml2 IN rather than
# linking it (`TINYXML2_DIR = vendor/android/tinyxml2` in its Makefile.debian), so
# a from-source build fails without it. It is not fetched by initialising this
# repo's own submodules, which is why the recipe below names the path.

ROOT_DIR    = $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
ENGINE_DIR  = $(ROOT_DIR)/external/storm-engine-v2
ENGINE_BIN  = $(ENGINE_DIR)/bin
ENGINE_LIB  = $(ENGINE_BIN)/libstormenginev2.so
# The engine's headers live in `common/` and are included as <stormengine2/...>,
# so the include path must be a directory CONTAINING a `stormengine2`. Hence a
# one-entry shim, built under build/ so nothing inside the submodule is written
# to for this.
ENGINE_INC  = $(ROOT_DIR)/build/engine-include

# ⚠️ THE ENGINE BUILDS IN-TREE (its makefile writes objects beside its sources),
# so the submodule reads dirty afterwards. `.gitmodules` sets `ignore = dirty` for
# that reason -- without it every `git status` in this repo shows the engine as
# modified, which trains you to ignore the one line that matters.
$(ENGINE_LIB):
	@echo "==> Storm Engine v2: the pinned submodule"
	git -C $(ROOT_DIR) submodule update --init --depth 1 external/storm-engine-v2
	@echo "==> tinyxml2 (the engine compiles it in, so a from-source build needs it)"
	# ⚠️ A NESTED submodule can only be addressed from INSIDE its parent
	# repository. `git -C $(ROOT_DIR) submodule update ... <nested path>` fails with
	# "pathspec ... did not match any file(s) known to git", because the path is
	# not in THIS repo's index -- it is in the engine's.
	git -C $(ENGINE_DIR) submodule update --init --depth 1 vendor/android/tinyxml2
	@echo "==> headers: $(ENGINE_INC)/stormengine2 -> the engine's common/"
	mkdir -p $(ENGINE_INC)
	ln -sfn $(ENGINE_DIR)/common $(ENGINE_INC)/stormengine2
	@echo "==> building $(ENGINE_LIB)"
	$(MAKE) -C $(ENGINE_DIR) -f Makefile.debian
	@ls -la $(ENGINE_LIB)

.PHONY: engine
engine: $(ENGINE_LIB)
	@echo "engine: up to date ($(ENGINE_LIB))"

.PHONY: engine-version
engine-version:
	@printf 'engine: '; git -C $(ENGINE_DIR) describe --tags --always 2>/dev/null \
		|| echo "submodule not initialised -- run: make engine"

# ── make engine-update -- advance the pin to the engine's newest main ────────
#
# "Build on the latest engine, always" is a MAINTENANCE RULE, and this is it as
# one command: it moves the submodule to the tip of the engine's default branch
# and leaves the gitlink change for review. The pin stays a COMMIT rather than a
# branch, so the build is reproducible until someone runs this on purpose -- an
# automatic "always newest" would mean a red build traceable to a commit in
# another repository instead of one here.
.PHONY: engine-update
engine-update:
	git -C $(ROOT_DIR) submodule update --remote --depth 1 external/storm-engine-v2
	@$(MAKE) -f $(ROOT_DIR)/engine.mk engine
	@echo
	@echo "The pin moved. Review, then commit the new revision:"
	@git -C $(ROOT_DIR) diff --submodule=log -- external/storm-engine-v2

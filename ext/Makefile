
.PHONY: all unit clean debug release

all: release

export BASEP := $(shell pwd)
export CCACHE_BASEDIR := $(BASEP)
export CCACHE_DIR = $(CCACHE_BASEDIR)/_ccache/
#export CMAKE_BUILD_TYPE = $(BUILD_TYPE)


ifeq ($(OS),Windows_NT)
	export uname_s := Windows
        export uname_m := $(MSYSTEM_CARCH)
        export CMAKE_GENERATOR := MSYS Makefiles
else
	export uname_s := $(shell uname -s)
	export uname_m := $(shell uname -m)
	export CMAKE_GENERATOR := Unix Makefiles
endif

export TARGETDIR := $(BASEP)/$(uname_s)_$(uname_m)

clean:
	ls -l $(TARGETDIR)

release: makedirs
	export CMAKE_BUILD_TYPE="Release"; \
	$(MAKE) make_exts

debug: makedirs
	export CMAKE_BUILD_TYPE="Debug"; \
	$(MAKE) make_exts

makedirs: $(CCACHE_DIR) $(TARGETDIR)

$(CCACHE_DIR) $(TARGETDIR):
	mkdir -vp $@

make_exts: make_gitalk make_osmetrics make_rapidcheck make_elykseer_crypto
	echo "all done."

fset_gitalk := $(TARGETDIR)/bin/html2text
make_gitalk: $(fset_gitalk)
$(fset_gitalk):
	echo "making gitalk"
	cd ./gitalk; \
	git submodule update --init; \
	$(MAKE) -C ext

fset_os_metrics := $(TARGETDIR)/include/os-metrics.h $(TARGETDIR)/lib/libos-metrics_$(CMAKE_BUILD_TYPE)_1_0_0.a
make_osmetrics: $(fset_os_metrics)
$(fset_os_metrics):
	echo "making os-metrics"
	cd ./os-metrics; \
	if [ -e build ]; then rm -rf build; fi; \
	mkdir -vp build; \
	cd build; \
	cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} --install-prefix "${TARGETDIR}" -G "${CMAKE_GENERATOR}" .. ; \
	$(MAKE); \
	$(MAKE) install

fset_rapidcheck := $(TARGETDIR)/include/rapidcheck.h $(TARGETDIR)/lib/librapidcheck.a
make_rapidcheck: $(fset_rapidcheck)
$(fset_rapidcheck):
	echo "making rapidcheck"
	patch -N -d rapidcheck -p 1 < rapidcheck-cstdint.patch; \
	cd ./rapidcheck; \
	if [ -e build ]; then rm -rf build; fi; \
	mkdir -vp build; \
	cd build; \
	cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} --install-prefix "${TARGETDIR}" -G "${CMAKE_GENERATOR}" .. ; \
	$(MAKE) \
	$(MAKE) install

ifeq (${CMAKE_BUILD_TYPE},Debug)
fset_elykseer_crypto := $(TARGETDIR)/../elykseer-crypto/build/src/libelykseer-crypto_d.a
else
fset_elykseer_crypto := $(TARGETDIR)/../elykseer-crypto/build/src/libelykseer-crypto.a
endif
make_elykseer_crypto: $(fset_elykseer_crypto)
$(fset_elykseer_crypto):
	echo "making elykseer_crypto"
	cd ./elykseer-crypto; \
	git submodule update --init; \
	$(MAKE) -C ext; \
	cd build; \
	cmake --fresh -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -G "${CMAKE_GENERATOR}" . ; \
	$(MAKE)


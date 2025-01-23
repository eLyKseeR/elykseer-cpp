
.PHONY: all unit clean debug release xWin64

all: release

export BASEP := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
export CCACHE_BASEDIR := $(BASEP)
export CCACHE_DIR = $(CCACHE_BASEDIR)/_ccache/

ifeq ($(OS),Windows_NT)
	export uname_s := Windows
	export uname_m := $(MSYSTEM_CARCH)
	export CMAKE_GENERATOR := MSYS Makefiles
else
	export uname_s := $(shell uname -s)
	export uname_m := $(shell uname -m)
	export CMAKE_GENERATOR := Unix Makefiles
endif

ifndef (HOST_TARGETDIR)
        export HOST_TARGETDIR := $(BASEP)/$(uname_s)_$(uname_m)
endif
ifndef (WIN64_TARGETDIR)
        export WIN64_TARGETDIR := $(BASEP)/xWindows_x86_64
endif

clean:
	ls -l $(HOST_TARGETDIR)

release:
	export TARGETDIR=$(HOST_TARGETDIR); \
	export HOST_SELECTOR=""; \
	export CMAKE_BUILD_TYPE="Release"; \
	export PKG_CONFIG_PATH="${HOST_TARGETDIR}/lib/pkgconfig:${PKG_CONFIG_PATH}"; \
	$(MAKE) make_exts

debug:
	export TARGETDIR=$(HOST_TARGETDIR); \
	export HOST_SELECTOR=""; \
	export CMAKE_BUILD_TYPE="Debug"; \
	export PKG_CONFIG_PATH="${HOST_TARGETDIR}/lib/pkgconfig:${PKG_CONFIG_PATH}"; \
	$(MAKE) make_exts

xWin64: check_cross_compilers
	export CROSS_COMPILATION=1; \
	export TARGETDIR=$(WIN64_TARGETDIR); \
	export HOST_SELECTOR="--host=x86_64-w64-mingw32"; \
	export XCOMPILE_TOOLCHAIN="$(BASEP)/../build/Toolchain_Darwin_to_Windows.cmake"; \
	export CMAKE_BUILD_TYPE="Release"; \
	export PKG_CONFIG_PATH="$(WIN64_TARGETDIR)/lib/pkgconfig"; \
	$(MAKE) make_exts

makedirs: $(CCACHE_DIR) $(HOST_TARGETDIR) $(WIN64_TARGETDIR)

$(CCACHE_DIR) $(HOST_TARGETDIR) $(WIN64_TARGETDIR):
	mkdir -vp $@

check_cross_compilers: check_cc check_cxx check_ar check_ranlib
check_ar:
	@[ -n "$(AR)" ] && type $(AR) 2> /dev/null

check_cc:
	@[ -n "$(CC)" ] && type $(CC) 2> /dev/null

check_cxx:
	@[ -n "$(CXX)" ] && type $(CXX) 2> /dev/null

check_ranlib:
	@[ -n "$(RANLIB)" ] && type $(RANLIB) 2> /dev/null


make_exts: makedirs make_gitalk make_osmetrics make_rapidcheck make_elykseer_crypto
	echo "all done."

fset_gitalk := $(TARGETDIR)/bin/html2text
make_gitalk: $(fset_gitalk)
$(fset_gitalk):
	if [ -z "$(CROSS_COMPILATION)" ]; then \
	echo "making gitalk"; \
	cd ./gitalk; \
	git submodule update --init; \
	$(MAKE) -C ext; \
	fi

fset_os_metrics := $(TARGETDIR)/include/os-metrics.h $(TARGETDIR)/lib/libos-metrics_$(CMAKE_BUILD_TYPE)_1_0_0.a
make_osmetrics: $(fset_os_metrics)
$(fset_os_metrics):
	echo "making os-metrics"
	cd ./os-metrics; \
	if [ -e build ]; then rm -rf build; fi; \
	mkdir -vp build; \
	cd build; \
	cmake --fresh -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_TOOLCHAIN_FILE=${XCOMPILE_TOOLCHAIN} --install-prefix "${TARGETDIR}" -G "${CMAKE_GENERATOR}" .. ; \
	$(MAKE); \
	$(MAKE) install

fset_rapidcheck := $(TARGETDIR)/include/rapidcheck.h $(TARGETDIR)/lib/librapidcheck.a
make_rapidcheck: $(fset_rapidcheck)
$(fset_rapidcheck):
	if [ -z "$(CROSS_COMPILATION)" ]; then \
	echo "making rapidcheck"; \
	patch -N -d rapidcheck -p 1 < rapidcheck-cstdint.patch; \
	cd ./rapidcheck; \
	if [ -e build ]; then rm -rf build; fi; \
	mkdir -vp build; \
	cd build; \
	cmake --fresh -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} --install-prefix "${TARGETDIR}" -G "${CMAKE_GENERATOR}" .. ; \
	$(MAKE); \
	$(MAKE) install; \
	fi

ifeq (${CMAKE_BUILD_TYPE},Debug)
fset_elykseer_crypto := $(TARGETDIR)/lib/libelykseer-crypto_d.a
else
fset_elykseer_crypto := $(TARGETDIR)/lib/libelykseer-crypto.a
endif
make_elykseer_crypto: $(fset_elykseer_crypto)
$(fset_elykseer_crypto):
	echo "making elykseer_crypto"
	cd ./elykseer-crypto; \
	git submodule update --init; \
	if [ -z "$(CROSS_COMPILATION)" ]; then \
	$(MAKE) -C ext; \
	mkdir -vp build; cd build; \
	cmake --fresh -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} --install-prefix "${TARGETDIR}" -G Ninja .. ; \
	else \
	$(MAKE) -C ext xWin64; \
	cd build; \
	cmake --fresh -DCMAKE_TOOLCHAIN_FILE=${XCOMPILE_TOOLCHAIN} -DCMAKE_BUILD_TYPE=Release --install-prefix "${TARGETDIR}" -G "${CMAKE_GENERATOR}" . ; \
	fi; \
	ninja; \
	ninja install;


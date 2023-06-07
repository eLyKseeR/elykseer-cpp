
.PHONY: all unit clean debug release

all: release

export BASEP := $(shell pwd)
export CCACHE_BASEDIR := $(BASEP)
export CCACHE_DIR = $(CCACHE_BASEDIR)/_ccache/
#export CMAKE_BUILD_TYPE = $(BUILD_TYPE)


ifeq ($(OS),Windows_NT)
	uname_s := Windows
	uname_m := $(ARCHITECTURE)
	export CMAKE_GENERATOR := MinGW Makefiles
else
	uname_s := $(shell uname -s)
	uname_m := $(shell uname -m)
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

make_exts: make_osmetrics make_rapidcheck make_elykseer_crypto
	echo "all done."

fset_os_metrics := $(TARGETDIR)/include/os-metrics.h $(TARGETDIR)/lib/libos-metrics_$(CMAKE_BUILD_TYPE)_1_0_0.a

make_osmetrics: $(fset_os_metrics)
$(fset_os_metrics):
	echo "making os-metrics"
	cd ./os-metrics; \
	if [ -e build ]; then rm -rf build; fi; \
	mkdir -vp build; \
	cd build; \
	cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} --install-prefix "${TARGETDIR}" -G "${CMAKE_GENERATOR}" ..; \
	$(MAKE); \
	$(MAKE) install

fset_rapidcheck := $(TARGETDIR)/../rapidcheck/build/librapidcheck.a
make_rapidcheck: $(fset_rapidcheck)
$(fset_rapidcheck):
	echo "making rapidcheck"
	cd ./rapidcheck; \
	if [ -e build ]; then rm -rf build; fi; \
	mkdir -vp build; \
	cd build; \
	cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} --install-prefix "${TARGETDIR}" -G "${CMAKE_GENERATOR}" ..; \
	$(MAKE)

fset_elykseer_crypto := $(TARGETDIR)/include/yaml-cpp/yaml.h $(TARGETDIR)/lib/libyaml-cpp.a
make_elykseer_crypto: $(fset_elykseer_crypto)
$(fset_elykseer_crypto):
	echo "making elykseer_crypto"
	cd ./elykseer-crypto; \
	git submodule update --init; \
	make -C ext; \
	cd build; \
	cmake --fresh -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} --install-prefix "${TARGETDIR}" -G "${CMAKE_GENERATOR}" .; \
	$(MAKE)



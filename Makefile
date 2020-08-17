-include config.mk
-include installdeps.mk

CC       = gcc
CXX      = g++
CFLAGS   += -g -rdynamic -Wall -O2 -std=c++11
LDFLAGS  += $(LIBS) -lpthread -lz -lrt
BUILD_DIR = build

BINARY = slow5tools
OBJ = $(BUILD_DIR)/main.o \
      $(BUILD_DIR)/fastt_main.o \
	  $(BUILD_DIR)/ftidx.o \
	  $(BUILD_DIR)/kstring.o \
      $(BUILD_DIR)/nanopolish_fast5_io.o \
	  src/htslib/libhts.a


PREFIX = /usr/local
VERSION = `git describe --tags`

.PHONY: clean distclean format test install uninstall

$(BINARY): src/config.h $(HDF5_LIB) $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) $(LDFLAGS) -o $@

$(BUILD_DIR)/main.o: src/main.c src/slow5misc.h src/error.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

$(BUILD_DIR)/fastt_main.o: src/fastt_main.c src/slow5.h src/fast5lite.h src/slow5misc.h src/error.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

$(BUILD_DIR)/ftidx.o: src/ftidx.c src/ftidx.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

$(BUILD_DIR)/nanopolish_fast5_io.o: src/nanopolish_fast5_io.c src/fast5lite.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

$(BUILD_DIR)/kstring.o: src/kstring.c src/kstring.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) $< -c -o $@


src/config.h:
	echo "/* Default config.h generated by Makefile */" >> $@
	echo "#define HAVE_HDF5_H 1" >> $@

$(BUILD_DIR)/lib/libhdf5.a:
	if command -v curl; then \
		curl -o $(BUILD_DIR)/hdf5.tar.bz2 https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-$(HDF5_MAJOR_MINOR)/hdf5-$(HDF5_VERSION)/src/hdf5-$(HDF5_VERSION).tar.bz2; \
	else \
		wget -O $(BUILD_DIR)/hdf5.tar.bz2 https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-$(HDF5_MAJOR_MINOR)/hdf5-$(HDF5_VERSION)/src/hdf5-$(HDF5_VERSION).tar.bz2; \
	fi
	tar -xf $(BUILD_DIR)/hdf5.tar.bz2 -C $(BUILD_DIR)
	mv $(BUILD_DIR)/hdf5-$(HDF5_VERSION) $(BUILD_DIR)/hdf5
	rm -f $(BUILD_DIR)/hdf5.tar.bz2
	cd $(BUILD_DIR)/hdf5 && \
	./configure --prefix=`pwd`/../ && \
	make -j8 && \
	make install

clean:
	rm -rf $(BINARY) $(BUILD_DIR)/*.o

# Delete all gitignored files (but not directories)
distclean: clean
	git clean -f -X
	rm -rf $(BUILD_DIR)/* autom4te.cache

dist: distclean
	mkdir -p slow5tools-$(VERSION)
	autoreconf
	cp -r README.md LICENSE Dockerfile Makefile configure.ac config.mk.in \
		installdeps.mk src docs build .dockerignore configure slow5tools-$(VERSION)
	mkdir -p slow5tools-$(VERSION)/scripts
	cp scripts/install-hdf5.sh scripts/install-hts.sh scripts/test.sh scripts/common.sh scripts/test.awk slow5tools-$(VERSION)/scripts
	tar -zcf slow5tools-$(VERSION)-release.tar.gz slow5tools-$(VERSION)
	rm -rf slow5tools-$(VERSION)

binary:
	mkdir -p slow5tools-$(VERSION)
	make clean
	make cuda=1 && mv slow5tools slow5tools-$(VERSION)/slow5tools_x86_64_linux_cuda && make clean
	make && mv slow5tools slow5tools-$(VERSION)/slow5tools_x86_64_linux
	cp -r README.md LICENSE docs slow5tools-$(VERSION)/
	mkdir -p slow5tools-$(VERSION)/scripts
	cp scripts/test.sh scripts/common.sh scripts/test.awk slow5tools-$(VERSION)/scripts
	tar -zcf slow5tools-$(VERSION)-binaries.tar.gz slow5tools-$(VERSION)
	rm -rf slow5tools-$(VERSION)

install: $(BINARY)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	cp -f $(BINARY) $(DESTDIR)$(PREFIX)/bin
	gzip < docs/slow5tools.1 > $(DESTDIR)$(PREFIX)/share/man/man1/slow5tools.1.gz

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BINARY) \
		$(DESTDIR)$(PREFIX)/share/man/man1/slow5tools.1.gz

test: $(BINARY)
	./scripts/test.sh

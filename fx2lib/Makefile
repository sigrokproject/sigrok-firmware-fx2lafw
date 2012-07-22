

VER=devel
TAG=HEAD

.PHONY: all docs

all:
	$(MAKE) -C lib

docs:
	doxygen docs/docs.conf
	$(MAKE) -C docs/intro

dist: all docs
	mkdir -p build	
	git archive --prefix=fx2lib/ $(TAG) > build/fx2-$(VER).tar
	tar -C .. -rf build/fx2-$(VER).tar \
		fx2lib/docs/html \
		fx2lib/lib/fx2.lib \
		fx2lib/docs/intro/intro.pdf
	cat build/fx2-$(VER).tar | gzip > build/fx2-$(VER).tgz
	


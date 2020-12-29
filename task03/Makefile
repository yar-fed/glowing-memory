directory=$(CURDIR)/bin

default: all
all: pak unpak $(directory)

bin/pak: src/pak.c
	cc $< -o $@

bin/unpak: src/unpak.c
	cc $< -o $@

pak: bin/pak
unpak: bin/unpak

.PHONY: clean install uninstall test

$(directory):
	mkdir $@

install: pak unpak test deb
	dpkg -i fancy-pak.deb

uninstall:
	dpkg -r fancy-pak

test: pak unpak
	bin/pak <bin/pak >bin/pak1.pak
	bin/unpak <bin/pak1.pak >bin/pak1
	chmod +x bin/pak1
	bin/pak1 <bin/pak >bin/pak2.pak
	bin/unpak <bin/pak2.pak >bin/pak2
	diff bin/pak bin/pak2

deb: pak unpak
	mkdir -p fancy-pak/opt/fancy_pak
	cp bin/pak fancy-pak/opt/fancy_pak/
	cp bin/unpak fancy-pak/opt/fancy_pak/
	dpkg-deb --build fancy-pak

clean:
	rm $(directory)/* fancy-pak/opt/fancy_pak/*


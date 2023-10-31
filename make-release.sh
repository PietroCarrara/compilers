#!/bin/sh
shopt -s globstar
OUT=etapa1

main() {
	mkdir -p release/
	rm -rf release/*
	cp src/**/*.c src/**/*.h src/**/*.l release/
	cp build/_deps/datatype99-src/datatype99.h release/
	cp -r build/_deps/metalang99-src/include release/

	# Flatten the structure, because the professor said so (???)
	for i in ./release/include/**/*.h; do
		fname=$(echo "${i#./release/include/}" | tr \/ _)
		mv "$i" "release/$fname"
		fix_includes "release/$fname"
	done
	rm -rf release/include

	cat <<EOF >release/Makefile
default: build

build:
	flex lex.l
	gcc -I. *.c -o $OUT

clean:
	rm $OUT
	rm lex.yy.c
EOF

	cd release
	tar cvzf ../$OUT.tgz .
	cd ..
}

fix_includes() {
	sed -ri 's/#include <(.*)>/echo "#include <$(echo \1 | tr \/ _)>"/e' "$1"
}

main

#!/bin/sh
shopt -s globstar

main() {
	mkdir -p release/
	rm -rf release/*
	cp src/**/*.c src/**/*.h src/**/*.l release/
	cp build/_deps/datatype99-src/datatype99.h release/
	cp -r build/_deps/metalang99-src/include release/

	# Flatten the structure, because the professor said so
	for i in ./release/include/**/*.h; do
		fname=$(echo "${i#./release/include/}" | tr \/ _)
		mv "$i" "release/$fname"
		fix_includes "release/$fname"
	done

	cat <<EOF >release/Makefile
default: build

build:
	flex lex.l
	gcc -I. *.c -o etapa1

clean:
	rm etapa1
	rm lex.yy.c
EOF
}

fix_includes() {
	sed -ri 's/#include <(.*)>/echo "#include <$(echo \1 | tr \/ _)>"/e' "$1"
}

main
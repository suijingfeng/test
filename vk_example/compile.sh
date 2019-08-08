#!/bin/bash

# Set your VULKAN_SDK location before running
VULKAN_SDK=~/vulkan/1.1.82.0/x86_64/
GLSL_CC=$(VULKAN_SDK)/bin/glslangValidator;

if [[ ! -x "./bintoc" ]]
then
	gcc bintoc.c -o bintoc
fi

find -type f -name "*.vert" | \
	while read f; do glslangValidator -V ${f} -o "${f%.*}.vspv"; done

find -type f -name "*.frag" | \
	while read f; do glslangValidator -V ${f} -o "${f%.*}.fspv"; done



find -type f -name "*.vspv" | \
	while read f; do ./bintoc ${f} `basename ${f%.*}`_vert_spv > ${f%.*}_vert.c; done

find -type f -name "*.fspv" | \
	while read f; do ./bintoc ${f} `basename ${f%.*}`_frag_spv > ${f%.*}_frag.c; done

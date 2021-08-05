cmake_minimum_required(VERSION 3.5)
project(swipl-binformat)

include("../cmake/PrologPackage.cmake")

configure_file(config.h.cmake config.h)

swipl_plugin(
    MODULE binformat
    C_SOURCES pl-binformat.c
    PL_LIBS binformat.pl)

pkg_doc(binformat
	SECTION
            binformat.pl)

cmake_minimum_required(VERSION 3.5)
project(swipl-binformat)

include("../cmake/PrologPackage.cmake")

configure_file(config.h.cmake config.h)

swipl_plugin(
    binformat
    MODULE binformat4pl
    C_SOURCES binformat4pl.c
    PL_LIBS binformat.pl)

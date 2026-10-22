# Windows compatibility layer for the original Makefile.
# GNU Make prefers this file and then loads the project rules unchanged.
ifeq ($(OS),Windows_NT)
SHELL := /usr/bin/sh.exe
export PATH := /usr/bin:/ucrt64/bin:$(PATH)
endif

include Makefile

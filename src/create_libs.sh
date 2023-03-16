#!/bin/bash

zig build -Dtarget=x86_64-windows-gnu
zig build -Dtarget=x86_64-macos-none
zig build

mv zig-out/lib/ ../game/
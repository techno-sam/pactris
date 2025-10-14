#!/bin/env python3

# Maakt een volle pacman veld van een kwart veld

import sys

if len(sys.argv) != 3:
    print(f"gebruik: {sys.argv[0]} <kwart> <vol>")

lines: list[str] = []

with open(sys.argv[1], "r") as f_in:
    for line in f_in:
        line = line[:-1] # strip trailing newline
        line = line + "".join(reversed(line))
        lines.append(line)

with open(sys.argv[2], "w") as f_out:
    for line in lines:
        f_out.write(line + "\n")

    for line in reversed(lines):
        f_out.write(line + "\n")

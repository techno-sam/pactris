#!/bin/env python3

# Maakt een volle pacman veld van een kwart veld

import sys

if len(sys.argv) != 3:
    print(f"gebruik: {sys.argv[0]} pacman_kwart.txt pacman.txt")
    exit(1)

lines: list[str] = []

with open(sys.argv[1], "r") as f_in:
    for line in f_in:
        line = line[:-1] # strip trailing newline
        line = line + "".join(reversed(line[:-1]))
        lines.append(line)

lines += reversed(lines[:-1])

width = len(lines[0])
height = len(lines)

def s(x: int, y: int, c: str, relative: bool = True):
    assert len(c) == 1
    if relative:
        x += width // 2
        y += height // 2
    line = lines[y]
    line = line[:x] + c + line[(x+1):]
    lines[y] = line

def set_multi(
    x: int,
    y: int,
    data: str,
    relative: bool = True,
    ignore: str | None = None
):
    x0 = x
    for c in data:
        if c == '\n':
            x = x0
            y += 1
        elif c == ignore:
            x += 1
        else:
            s(x, y, c, relative=relative)
            x += 1

# HOME

s(0, -2, 'B') # blinky
s(-1, 0, 'I') # inky
s(0, 0, 'P') # pinky
s(1, 0, 'C') # clyde

set_multi(-2, -1, """
1-+-2
[~~~]
3___4
""".strip(), ignore='~')

# SPAWN
s(1, 1, '!', relative=False)

with open(sys.argv[2], "w") as f_out:
    for line in lines:
        f_out.write(line + "\n")

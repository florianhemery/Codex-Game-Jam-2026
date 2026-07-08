#!/usr/bin/env python3
"""Extract line ranges from a .cpp into a new translation unit."""

from __future__ import annotations

import argparse
import os
import sys

EPITECH = """\
/*
** EPITECH PROJECT, 2026
** racer
** File description:
** {desc}
*/

"""


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source")
    parser.add_argument("dest")
    parser.add_argument("start", type=int, help="1-based inclusive")
    parser.add_argument("end", type=int, help="1-based inclusive, 0 = EOF")
    parser.add_argument("--desc", required=True)
    parser.add_argument("--includes", nargs="*", default=[])
    parser.add_argument("--remove-from-source", action="store_true")
    args = parser.parse_args()

    with open(args.source, encoding="utf-8") as f:
        lines = f.readlines()

    end = len(lines) if args.end == 0 else args.end
    chunk = lines[args.start - 1 : end]

    body = "".join(f'#include "{inc}"\n' for inc in args.includes)
    if body:
        body += "\n"
    body += "namespace racer {\n\n"
    body += "".join(chunk)
    if not body.endswith("\n"):
        body += "\n"
    body += "\n} // namespace racer\n"

    os.makedirs(os.path.dirname(args.dest) or ".", exist_ok=True)
    with open(args.dest, "w", encoding="utf-8", newline="\n") as f:
        f.write(EPITECH.format(desc=args.desc))
        f.write(body)

    if args.remove_from_source:
        kept = lines[: args.start - 1] + lines[end:]
        with open(args.source, "w", encoding="utf-8", newline="\n") as f:
            f.writelines(kept)

    print(f"wrote {args.dest} ({end - args.start + 1} lines)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

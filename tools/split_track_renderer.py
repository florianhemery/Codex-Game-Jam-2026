#!/usr/bin/env python3
"""Split TrackRenderer.cpp into translation units under src/Render/Track/."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TRACK = ROOT / "src" / "Render" / "Track"
OLD = ROOT / "src" / "Render" / "TrackRenderer.cpp"

PREAMBLE = """/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track rendering module
*/

#include "Render/Track/TrackRendererInternal.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

#include "rlgl.h"

namespace racer {

"""

PREAMBLE_CORE = """/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track renderer facade
*/

#include "Render/Track/TrackRenderer.hpp"
#include "Render/Track/TrackRendererInternal.hpp"

#include <cmath>
#include <vector>

namespace racer {

"""


def write_cpp(path: Path, head: str, lines: list[str]) -> None:
    path.write_text(head + "".join(lines) + "\n} // namespace racer\n", encoding="utf-8")
    print(f"{path.name}: {len(lines)} lines")


def main() -> None:
    TRACK.mkdir(parents=True, exist_ok=True)
    lines = OLD.read_text(encoding="utf-8").splitlines(keepends=True)

    internal_head = """/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track renderer internal helpers
*/

#ifndef TRACK_RENDERER_INTERNAL_HPP_
#define TRACK_RENDERER_INTERNAL_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "raylib.h"
#include "Render/Track/TrackRenderer.hpp"
#include "Track/Track.hpp"

namespace racer {

constexpr int kSkidTextureSize = 2048;

"""
    internal_body = "".join(lines[24:122]) + "\n" + "".join(lines[784:972])
    internal_tail = "\n} // namespace racer\n\n#endif /* !TRACK_RENDERER_INTERNAL_HPP_ */\n"
    (TRACK / "TrackRendererInternal.hpp").write_text(
        internal_head + internal_body + internal_tail, encoding="utf-8"
    )

    splits = [
        ("TrackMeshDetailA.cpp", PREAMBLE, 124, 350),
        ("TrackMeshDetailB.cpp", PREAMBLE, 351, 550),
        ("TrackMeshDetailC.cpp", PREAMBLE, 551, 781),
        ("TrackDecorBuilderA.cpp", PREAMBLE, 974, 1200),
        ("TrackDecorBuilderB.cpp", PREAMBLE, 1201, 1450),
        ("TrackDecorBuilderC.cpp", PREAMBLE, 1451, 1750),
        ("TrackDecorBuilderD.cpp", PREAMBLE, 2236, 2262),
        ("TrackDrawPassA.cpp", PREAMBLE, 1751, 1950),
        ("TrackDrawPassB.cpp", PREAMBLE, 1951, 2217),
        ("TrackDrawPassC.cpp", PREAMBLE, 2372, 2403),
        ("TrackRendererCore.cpp", PREAMBLE_CORE, 2219, 2234),
        ("TrackRendererCore.cpp", PREAMBLE_CORE, 2264, 2371),
    ]

    core_chunks: list[str] = []
    for fname, head, start, end in splits:
        chunk = lines[start - 1 : end]
        if fname == "TrackRendererCore.cpp":
            core_chunks.extend(chunk)
            continue
        write_cpp(TRACK / fname, head, chunk)

    write_cpp(TRACK / "TrackRendererCore.cpp", PREAMBLE_CORE, core_chunks)
    print("Done.")


if __name__ == "__main__":
    main()

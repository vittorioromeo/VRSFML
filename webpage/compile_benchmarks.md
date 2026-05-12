# VRSFML vs upstream SFML -- compilation benchmarks

Wall-clock and CPU-time measurements for rebuilding VRSFML and upstream SFML
from source under a typical day-to-day development configuration.

## Setup

- **Tool**: `hyperfine` 1.20, 1 warmup run + 3 timed runs.
- **Build system**: `ninja`, `-j 16`.
- **Compiler**: `clang++` (system default), C++ standard set per project.
- **Build type**: `Debug`.
- **Linker**: `lld` (`-fuse-ld=lld`).
- **Linkage**: static (`BUILD_SHARED_LIBS=OFF`).
- **Dependencies**: bundled (`SFML_USE_SYSTEM_DEPS=OFF`).
- **Caching**: `ccache` disabled (`CMAKE_CXX_COMPILER_LAUNCHER=`).
- **PCH**: off (`SFML_ENABLE_PCH=OFF`).
- **Hardware**: 32-thread x86-64 Linux workstation, NVMe SSD.
- **VRSFML**: `bubble_idle` example excluded for fairness (commercial-game
  source tree, much larger than upstream's example pool).

### Selective clean

Between hyperfine iterations the `--prepare` step removes only SFML-owned
build outputs, not third-party dependencies -- that matches what a developer
actually does when iterating on their own code (you don't recompile
`freetype` or `harfbuzz` on every keystroke).

```bash
# Removed between runs
find <build>/src -path '*/CMakeFiles/sfml-*.dir/*' \( -name '*.o' -o -name '*.o.d' \) -delete
find <build>/lib -name 'libsfml-*.a' -delete
find <build>/examples -name '*.o' -delete                       # scenarios 2, 3
find <build>/bin -maxdepth 1 -type f ! -name 'test-*' -delete   # scenarios 2, 3
find <build>/test -name '*.o' -delete                           # scenario 3
find <build>/bin -maxdepth 1 -name 'test-*' -delete             # scenario 3

# Preserved between runs
_deps/**                  # all CPM/FetchContent source trees
libfreetyped.a            # freetype
libharfbuzzd.a            # upstream only
libimgui.a / libSDL3.a    # VRSFML only
libvorbisd.a / liboggd.a / libFLACd.a   # both
libmbed*.a                # both (SFTP example)
src/SFML/ImGui/CMakeFiles/imgui.dir/**/*.o    # VRSFML's bundled imgui code
```

## Scenarios

| # | Name                              | What it builds                                                          |
|---|-----------------------------------|-------------------------------------------------------------------------|
| 1 | Library only                      | All SFML modules (no examples, no tests)                                |
| 2 | Library + examples                | All SFML modules and every example app                                  |
| 3 | Library + examples + tests        | All SFML modules, every example, the full test suite                    |
| 4 | Six user TUs from scratch         | `island`, `shader`, `stencil`, `tennis`, `keyboard`, `joystick` only    |

For scenario 4 the SFML library is fully built once up-front; the prepare
step deletes only the six example object files and binaries.

## Results

### Scenario 1 -- Library only

| Command       | Mean        | Min     | Max     | Relative      |
|---------------|------------:|--------:|--------:|--------------:|
| `VRSFML s1`   | 1.678 s ± 0.016 | 1.660 s | 1.692 s | **1.00**      |
| `Upstream s1` | 3.692 s ± 0.027 | 3.671 s | 3.722 s | 2.20 ± 0.03   |

User CPU time: VRSFML 13.5 s, Upstream 39.6 s.

### Scenario 2 -- Library + examples

| Command       | Mean        | Min     | Max     | Relative      |
|---------------|------------:|--------:|--------:|--------------:|
| `VRSFML s2`   | 2.233 s ± 0.057 | 2.169 s | 2.279 s | **1.00**      |
| `Upstream s2` | 4.548 s ± 0.225 | 4.304 s | 4.748 s | 2.04 ± 0.11   |

User CPU time: VRSFML 24.5 s, Upstream 58.8 s.

### Scenario 3 -- Library + examples + tests

| Command       | Mean        | Min     | Max     | Relative      |
|---------------|------------:|--------:|--------:|--------------:|
| `VRSFML s3`   | 3.673 s ± 0.008 | 3.664 s | 3.680 s | **1.00**      |
| `Upstream s3` | 7.434 s ± 0.061 | 7.389 s | 7.504 s | 2.02 ± 0.02   |

User CPU time: VRSFML 45.1 s, Upstream 103.9 s.

### Scenario 4 -- Six user TUs from scratch (library prebuilt)

| Command       | Mean         | Min      | Max      | Relative      |
|---------------|-------------:|---------:|---------:|--------------:|
| `VRSFML s4`   | 287.8 ms ± 22.8 | 262.6 ms | 306.9 ms | **1.00**      |
| `Upstream s4` | 726.3 ms ± 33.0 | 694.7 ms | 760.6 ms | 2.52 ± 0.23   |

User CPU time: VRSFML 1.36 s, Upstream 3.67 s.

## Translation-unit counts and per-TU cost

The TU count is the number of `.o` files that the selective-clean step
removes -- i.e. the work each scenario actually performs.

| Scenario                       | VRSFML TUs           | Upstream TUs       | VRSFML CPU/TU | Upstream CPU/TU |
|--------------------------------|----------------------|--------------------|--------------:|----------------:|
| S1 Library only                | 129                  | 102                | 105 ms        | 388 ms          |
| S2 Library + examples          | 209  (129 + 80)      | 128  (102 + 26)    | 117 ms        | 459 ms          |
| S3 Library + examples + tests  | 350  (129 + 80 + 141)| 221  (102 + 26 + 93) | 129 ms      | 470 ms          |
| S4 Six user TUs                | 6                    | 6                  | 226 ms        | 612 ms          |

`CPU/TU` is `User-time / TU count`. It is the most direct measure of how
much work clang has to do to compile one translation unit on each side.

## Headline takeaways

- VRSFML rebuilds **2.0-2.5x faster** in wall time across every scenario.
- VRSFML actually compiles **more** TUs in scenarios 1-3 -- extra
  `SFML::ImGui` module, more bundled examples, larger test suite -- and is
  still ~2x faster. That means per-TU compiler work is roughly 3-4x lower.
- Scenario 4, where both projects compile the same six user-side TUs that
  include `<SFML/Graphics.hpp>` / `<SFML/Audio.hpp>`, isolates the header
  weight cleanly: ~2.7x less CPU per file.
- Removing third-party dep compilation from the loop (selective clean)
  roughly halved the previous "full ninja clean" wall times -- confirming
  that most of a naive `ninja clean && ninja` cycle was time spent
  rebuilding bundled C libraries that no real workflow re-runs.

## What is still not strictly comparable

- **`SFML::ImGui` module**. VRSFML ships an in-tree ImGui integration as a
  separate library; upstream does not. Scenarios 1-3 charge VRSFML for that
  extra surface. Scenario 4 is unaffected (none of the chosen examples use
  ImGui).
- **Example pool**. Even with `bubble_idle` excluded, VRSFML has more
  example apps than upstream, so scenario 2 (and 3) build more example
  binaries on the VRSFML side.
- **Test suite size**. VRSFML's test suite has more cases. Scenario 3
  reflects that.

A future "strictly comparable" run would add `SFML_BUILD_IMGUI=OFF` to
VRSFML's configure and reduce the example/test set to the intersection of
both projects.

## Reproducing

The scripts used to produce these numbers live in `/tmp/sfml_bench/` on the
machine that ran them; the relevant pieces are:

- `configure_all.sh` -- configures three build dirs per project (lib only,
  lib+examples, lib+examples+tests) with the flags listed above.
- `run_benchmarks_selective.sh` -- drives `hyperfine` for all four scenarios
  using the selective-clean prepare step.
- `count_tus2.sh` -- counts the `.o` files that each scenario's selective
  clean would delete, after a full build.

Both source trees were checked out fresh:

```bash
git clone https://github.com/vittorioromeo/VRSFML       ~/OHW/VRSFML
git clone --depth 1 https://github.com/SFML/SFML        ~/OHW/SFMLUpstreamForBenchmark
```

The `bubble_idle` exclusion was implemented by commenting out
`add_subdirectory(bubble_idle)` in `examples/CMakeLists.txt` for the
duration of the run, then restoring it.

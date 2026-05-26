# Zancle rebrand tooling

Mechanical-rename scripts developed for migrating VRSFML to its new name
(**Zancle**, namespaces `za::` / `zb::`). Iterated on a small sandbox corpus
before being committed here.

## What these scripts do

| Script | Role |
|---|---|
| `migrate.sh` | The actual rename pipeline. Moves files, rewrites namespaces, macros, includes, CMake targets, GLSL identifiers. |
| `verify.sh` | Greps the result for missed renames and surfaces "needs human review" hits (string literals, doc prose, asset paths). |
| `inventory.sh` | Counts old vs. new patterns before/after -- sanity-check that the totals balance. |
| `reset.sh` | Sandbox-only helper: restores `sandbox/` from `baseline/`. Not needed for the real repo migration (use `git restore` / branch). |

## Sandbox workflow (how the scripts were developed)

```
<root>/
├── baseline/   pristine source corpus
├── sandbox/    mutable working copy
└── scripts/    these scripts

scripts/reset.sh             # blank slate
scripts/inventory.sh sandbox # pre-flight census
scripts/migrate.sh sandbox   # do the rename
scripts/inventory.sh sandbox # post-flight census
scripts/verify.sh            # surface anything suspicious
```

## Real-repo workflow (when you're ready)

These scripts assume a `baseline/`/`sandbox/` layout. To run against the real
repo, do *one* of:

1. **Adapt in-place**: edit `migrate.sh` so `TARGET` resolves to the repo
   root (e.g. drop the `cd "$ROOT/$TARGET"` line, or pass `.` as the arg
   from inside the repo). Or just lift the `perl -i -pe` substitution table
   out of `migrate.sh` and run it directly against the repo from a branch.

2. **Use the sandbox as a dress rehearsal**: copy the whole repo into
   `baseline/`, run the pipeline, inspect, and only then replay the same
   substitutions on the real branch with `git mv` in place of `mv` for
   Phase 1.

Either way, the substitution rules in `migrate.sh` are the asset. The
wrapper is just plumbing.

## What the rules cover

In rough order of application (order is load-bearing -- see comments in
`migrate.sh`):

1. **File moves** -- `include/SFML/Base/` → `include/ZancleBase/`,
   `include/SFML/` → `include/Zancle/`, same for `src/`.
2. **Bare-namespace declarations** -- `namespace sf`, `namespace sf::base`,
   `using namespace sf`, closing-comment forms.
3. **Trailing-`::` namespaces** -- `sf::base::` → `zb::`, then `sf::` → `za::`.
4. **GLSL identifiers** -- `sf_u_*`, `sf_a_*`, `sf_v_*`, `sf_fragColor` →
   `za_*` equivalents. Applies in both C++ raw-string-literal shaders and
   standalone `.frag`/`.vert`/`.geom` files.
5. **CamelCase identifiers** -- `sfml<Upper>` → `zancle<Upper>` (catches
   `sfmlInternalAbiCheck*` ABI symbols, `sfmlLogoTexture` variables, etc.).
6. **Macros** -- `-DSFML_BASE_` and bare `SFML_BASE_` → `ZB_`,
   `-DSFML_` and bare `SFML_` → `ZA_`.
7. **Include paths** -- `<SFML/Base/X>` → `<ZancleBase/X>`,
   `<SFML/X>` → `<Zancle/X>`, same for `"..."` form.
8. **CMake** -- `SFML::X` → `Zancle::X` targets, `sfml-foo` → `zancle-foo`
   target names, `sfml_foo` → `zancle_foo` helper macros (CMake files
   only), bare `SFML` → `Zancle` (CMake files only).

## What the rules deliberately leave alone

These are NOT mechanical migration targets. `verify.sh` prints them as a
review checklist:

- **`sfex::` namespace and `SFEX_*` macros** -- example-only stackless
  coroutine utilities. Their own identity. Untouched on purpose.
- **`"sfml_logo.png"` and other lowercase `sfml_` substrings inside C++
  string literals** -- asset filenames. Renaming the reference without
  renaming the file breaks the example; this is a `git mv` decision.
- **Bare `SFML` in C++ string literals, comments, and Doxygen prose** --
  `"SFML Tennis"`, `// the SFML logo`, `unlike upstream SFML, ...`, license
  URLs. Judgment calls; sweep with an agent or by hand.

## Known follow-ups for the real migration

Anything in `migrate.sh` is what the test corpus exercised. Real repo will
likely surface more. Likely additions:

- License-URL sed: `s{vittorioromeo/VRSFML/blob/master/license.md}{vittorioromeo/Zancle/blob/master/license.md}g`.
- Doxygen prose sweep: `s/\bVRSFML\b/Zancle/g` inside Doxygen blocks.
- Asset file renames: `git mv examples/tennis/resources/sfml_logo.png examples/tennis/resources/zancle_logo.png` + ref updates.
- Header-guard normalization audit (mostly covered by the `\bSFML_` macro
  pass, but worth a spot-check that `#endif // ZA_..._HPP` comments are
  coherent).

Drop more files into the sandbox corpus and re-iterate the script if you
hit a pattern these rules don't cover.

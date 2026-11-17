# libImpeller glyph bounds repro

Minimal repro for [flutter/flutter#196552](https://github.com/flutter/flutter/pull/196552).

`ImpellerGlyphInfoGetGraphemeClusterBounds` returns an `ImpellerRect` with its
`x` and `y` fields swapped. [demo.c](demo.c) lays out the paragraph
`"Hello\nWorld"` and checks that:

1. The second glyph (`e`) is to the right of the first glyph (`H`).
2. The first glyph on the second line (`W`) is below the first line's baseline.

## Build & run

```sh
cmake -B build
cmake --build build
./build/impeller_demo
```

No window or GPU context is needed.

## Output (SDK `97f358fd0a7e06be8e36c634671f1a228ccf42c8`, macOS arm64)

```
'H' (line 0, index 0):       x=   0.45 y=   0.00 w= 14.84 h= 23.55
'e' (line 0, index 1):       x=   0.45 y=  14.84 w= 11.43 h= 23.55
'W' (line 1, first glyph):   x=  24.45 y=   0.00 w= 18.38 h= 23.55
Line 0 baseline: 19.78

FAIL: 'e' x (0.45) should be >= 'H' right edge (15.29)
FAIL: 'W' y (0.00) should be > line 0 baseline (19.78)

Bug reproduced: glyph bounds x/y are swapped.
```

The `x` for `e` and `W` sits in the `y` field and vice versa. The program exits
with `1` while the bug exists and `0` once it's fixed.

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Minimal repro for https://github.com/flutter/flutter/pull/196552
//
// `ImpellerGlyphInfoGetGraphemeClusterBounds` returns a rect whose `x` and `y`
// fields are swapped.
//
// This lays out a two-line paragraph and checks the glyph bounds:
//  1. The second glyph on the first line should be to the right of the first.
//  2. The first glyph on the second line should be below the first baseline.

#include <stdio.h>
#include <string.h>

#include "impeller.h"

static void PrintBounds(const char* label, ImpellerRect r) {
  printf("%-28s x=%7.2f y=%7.2f w=%6.2f h=%6.2f\n", label, r.x, r.y, r.width,
         r.height);
}

int main(int argc, char const* argv[]) {
  ImpellerTypographyContext context = ImpellerTypographyContextNew();
  ImpellerParagraphStyle style = ImpellerParagraphStyleNew();
  ImpellerParagraphStyleSetFontSize(style, 20);

  ImpellerParagraphBuilder builder = ImpellerParagraphBuilderNew(context);
  ImpellerParagraphBuilderPushStyle(builder, style);
  const char* text = "Hello\nWorld";
  ImpellerParagraphBuilderAddText(builder, (const uint8_t*)text,
                                  (uint32_t)strlen(text));
  ImpellerParagraph paragraph =
      ImpellerParagraphBuilderBuildParagraphNew(builder, 500);
  if (!paragraph) {
    fprintf(stderr, "Failed to build paragraph.\n");
    return 1;
  }

  ImpellerLineMetrics metrics = ImpellerParagraphGetLineMetrics(paragraph);
  double baseline0 = ImpellerLineMetricsGetBaseline(metrics, 0);
  size_t line1_start = ImpellerLineMetricsGetCodeUnitStartIndex(metrics, 1);

  ImpellerGlyphInfo first =
      ImpellerParagraphCreateGlyphInfoAtCodeUnitIndexNew(paragraph, 0);
  ImpellerGlyphInfo second =
      ImpellerParagraphCreateGlyphInfoAtCodeUnitIndexNew(paragraph, 1);
  ImpellerGlyphInfo next_line =
      ImpellerParagraphCreateGlyphInfoAtCodeUnitIndexNew(paragraph,
                                                         line1_start);

  ImpellerRect first_bounds = ImpellerGlyphInfoGetGraphemeClusterBounds(first);
  ImpellerRect second_bounds =
      ImpellerGlyphInfoGetGraphemeClusterBounds(second);
  ImpellerRect next_line_bounds =
      ImpellerGlyphInfoGetGraphemeClusterBounds(next_line);

  PrintBounds("'H' (line 0, index 0):", first_bounds);
  PrintBounds("'e' (line 0, index 1):", second_bounds);
  PrintBounds("'W' (line 1, first glyph):", next_line_bounds);
  printf("Line 0 baseline: %.2f\n\n", baseline0);

  int failures = 0;
  if (second_bounds.x < first_bounds.x + first_bounds.width) {
    printf("FAIL: 'e' x (%.2f) should be >= 'H' right edge (%.2f)\n",
           second_bounds.x, first_bounds.x + first_bounds.width);
    failures++;
  }
  if (next_line_bounds.y <= baseline0) {
    printf("FAIL: 'W' y (%.2f) should be > line 0 baseline (%.2f)\n",
           next_line_bounds.y, baseline0);
    failures++;
  }
  printf(failures ? "\nBug reproduced: glyph bounds x/y are swapped.\n"
                  : "PASS: glyph bounds are correct.\n");

  ImpellerGlyphInfoRelease(next_line);
  ImpellerGlyphInfoRelease(second);
  ImpellerGlyphInfoRelease(first);
  ImpellerParagraphRelease(paragraph);
  ImpellerParagraphBuilderRelease(builder);
  ImpellerParagraphStyleRelease(style);
  ImpellerTypographyContextRelease(context);
  return failures ? 1 : 0;
}

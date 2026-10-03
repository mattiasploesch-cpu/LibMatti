#!/usr/bin/env python3
"""Generate the C port of ChunkHeightAndBiomeFix's BIOMES_BY_ID table.

Java keeps the old numeric biome ids as a static Int2ObjectMap filled in the
class initializer; this script turns those put() calls into a sorted C table.

Usage: gen_biome_ids.py <ChunkHeightAndBiomeFix.java> <out.c>
"""
import re
import sys

src = open(sys.argv[1]).read()
out_path = sys.argv[2]

pairs = [(int(i), name) for i, name in re.findall(r'BIOMES_BY_ID\.put\((\d+),\s*"([^"]+)"\);', src)]
if not pairs:
    raise SystemExit('no BIOMES_BY_ID entries found')
pairs.sort()

lines = [
    '// Port of the ChunkHeightAndBiomeFix.BIOMES_BY_ID table (P7.3) - the old',
    '// numeric biome ids the 1.17 chunks stored, mapped to their biome ids.',
    '//',
    '// GENERATED from ChunkHeightAndBiomeFix.java by tools/gen_biome_ids.py - do',
    '// not edit by hand; re-run the generator instead.',
    '',
    '#include "libmatti/net/minecraft/util/datafix/fixes/BiomeIds.h"',
    '',
    '#include <stddef.h>',
    '',
    '// Java: BIOMES_BY_ID.put(id, "minecraft:...")',
    'static const BIOME_ID BIOME_IDS[] = {',
]
for i, name in pairs:
    lines.append('    {%d, "%s"},' % (i, name))
lines += ['};', '']
lines += [
    'const char *LIBMATTI_MC_BiomeIds_NameOf(int id)',
    '{',
    '    // Java: BIOMES_BY_ID.getOrDefault(id, "minecraft:plains")',
    '    if (id < 0 || id > BIOME_ID_MAX)',
    '        return "minecraft:plains";',
    '    for (size_t i = 0; i < sizeof(BIOME_IDS) / sizeof(BIOME_IDS[0]); i++)',
    '        if (BIOME_IDS[i].id == id)',
    '            return BIOME_IDS[i].name;',
    '    return "minecraft:plains";',
    '}',
    '',
]

open(out_path, 'w').write('\n'.join(lines))
print('%d biome ids -> %s' % (len(pairs), out_path))
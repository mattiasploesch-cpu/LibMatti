#!/usr/bin/env python3
"""Generate the C port of BlockStateData from the Java source.

The Java class is a 9000-line literal: ~390 named Map<String,String> property
maps plus ~1700 register(oldId, create(name, props?)...) calls. This script
parses both and emits a flat C table:

  PROPS[][2]      the distinct property maps as flat key/value rows
  VARIANTS[]      name + property row index (-1 = no properties)
  ENTRIES[]       old block id -> first variant + variant count
  OLD_TO_ENTRY[]  the MAP[] lookup that getTag() indexes

Usage: gen_block_state_data.py <BlockStateData.java> <out.c>
"""
import re
import sys

src = open(sys.argv[1]).read()
out_path = sys.argv[2]


def split_args(body):
    """split a call argument list on top-level commas"""
    args, depth, current, in_str, escape = [], 0, [], False, False
    for ch in body:
        if escape:
            current.append(ch)
            escape = False
            continue
        if ch == '\\' and in_str:
            current.append(ch)
            escape = True
            continue
        if in_str:
            current.append(ch)
            if ch == '"':
                in_str = False
            continue
        if ch == '"':
            in_str = True
            current.append(ch)
            continue
        if ch in '([':
            depth += 1
        elif ch in ')]':
            depth -= 1
        if ch == ',' and depth == 0:
            args.append(''.join(current).strip())
            current = []
            continue
        current.append(ch)
    tail = ''.join(current).strip()
    if tail:
        args.append(tail)
    return args


def find_calls(text, name):
    """yield the argument body of every name(...) call in the source"""
    for m in re.finditer(r'(?<![\w.])%s\(' % re.escape(name), text):
        start = m.end()
        depth, i, in_str, escape = 1, start, False, False
        while depth > 0 and i < len(text):
            ch = text[i]
            if escape:
                escape = False
            elif ch == '\\' and in_str:
                escape = True
            elif ch == '"':
                in_str = not in_str
            elif not in_str:
                if ch == '(':
                    depth += 1
                elif ch == ')':
                    depth -= 1
            i += 1
        yield text[start:i - 1]


# --- the named Map.of constants -------------------------------------------------
prop_maps = {}
for m in re.finditer(r'Map<String, String>\s+(\w+)\s*=\s*Map\.of\(([^;]*?)\)\s*;', src, re.S):
    name, body = m.group(1), m.group(2)
    prop_maps[name] = tuple(re.findall(r'"([^"]*)"\s*,\s*"([^"]*)"', body))


def parse_props(expr):
    """the second create() argument: a constant name or an inline Map.of"""
    if expr is None:
        return ()
    expr = expr.strip()
    if expr in prop_maps:
        return prop_maps[expr]
    if expr.startswith('Map.of('):
        return tuple(re.findall(r'"([^"]*)"\s*,\s*"([^"]*)"', expr))
    raise SystemExit('unknown property expression: %r' % expr)


CREATE = re.compile(r'"([^"]*)"\s*(?:,\s*(.*))?$', re.S)

entries = []  # (oldId, [(name, props), ...])
for body in find_calls(src, 'register'):
    args = split_args(body)
    if not args or not re.fullmatch(r'-?\d+', args[0]):
        continue
    variants = []
    for arg in args[1:]:
        arg = arg.strip()
        if not arg.startswith('create('):
            raise SystemExit('unexpected register argument: %r' % arg)
        inner = arg[len('create('):].rsplit(')', 1)[0].strip()
        m = CREATE.fullmatch(inner)
        if m is None:
            raise SystemExit('cannot parse create: %r' % arg)
        variants.append((m.group(1), parse_props(m.group(2))))
    entries.append((old_id := int(args[0]), variants))

if not entries:
    raise SystemExit('no register() calls parsed')

entries.sort(key=lambda e: e[0])

# --- intern every property map first so the tables can be written in one pass ---
prop_index, prop_rows = {}, []


def intern(props):
    key = tuple(props)
    if key not in prop_index:
        prop_index[key] = len(prop_rows)
        prop_rows.append(key)
    return prop_index[key]


entry_rows = []
variant_rows = []
for old_id, variants in entries:
    first = len(variant_rows)
    for name, props in variants:
        variant_rows.append((name, intern(props) if props else -1))
    entry_rows.append((old_id, first, len(variants)))

# OLD_TO_ENTRY: the id -> entry lookup, mirroring Java's MAP[] (air = entry 0)
old_to_entry = [-1] * 4096
for at, (old_id, first, count) in enumerate(entry_rows):
    for i in range(count):
        if 0 <= old_id + i < 4096:
            old_to_entry[old_id + i] = at

lines = [
    '// Port of net.minecraft.util.datafix.fixes.BlockStateData (P7.3) - the 1.13',
    '// numeric block id table the 1.12/1.15 chunk fix resolves raw section ids',
    '// with (Java: MAP[4096], filled by the register() bootstrap calls).',
    '//',
    '// GENERATED from BlockStateData.java by tools/gen_block_state_data.py - do not',
    '// edit by hand; re-run the generator instead.',
    '',
    '#include "libmatti/net/minecraft/util/datafix/fixes/BlockStateData.h"',
    '',
    '#include <stddef.h>',
    '#include <stdint.h>',
    '',
    '// Java: the two table row shapes',
    'typedef struct',
    '{',
    '    const char *name;',
    '    int16_t props;   // the PROPS[] row index, -1 without properties',
    '} VARIANT;',
    '',
    'typedef struct',
    '{',
    '    int16_t oldId;      // the numeric id the entry starts at',
    '    int16_t firstVariant;',
    '    int16_t count;      // how many ids the entry covers',
    '} ENTRY;',
    '',
    '// Java: the pair count per property row',
    'static const int8_t PROPS_COUNT[] = {' + ', '.join(str(len(r)) for r in prop_rows) + '};',
    '',
    '// Java: the Map<String, String> constants, flattened into key/value rows',
    'static const char *const PROPS[][2] = {',
]
for row in prop_rows:
    # the rows are flattened: every pair takes one PROPS[][2] line, and
    # PROPS_COUNT carries how many pairs the row holds (an empty map still
    # needs a line - variants without properties carry the -1 index instead)
    lines.append('    ' + ' '.join('{"%s", "%s"},' % pair for pair in row) if row else '    {"", ""},')
lines += ['};', '']

lines += ['// Java: the create(name, properties) tags, in register order',
          'static const VARIANT VARIANTS[] = {']
for name, props in variant_rows:
    # Java: create() stores the name verbatim - the register() calls already
    # pass the namespaced ids ("minecraft:stone"), so nothing is prepended
    lines.append('    {"%s", %d},' % (name, props))
lines += ['};', '']

lines += ['// Java: register(oldId, create(...)...) - the id range each entry covers',
          'static const ENTRY ENTRIES[] = {']
for old_id, first, count in entry_rows:
    lines.append('    {%d, %d, %d},' % (old_id, first, count))
lines += ['};', '']

lines += ['// Java: MAP[] - the entry getTag() resolves an id through',
          'static const int16_t OLD_TO_ENTRY[4096] = {']
for start in range(0, 4096, 32):
    lines.append('    ' + ', '.join(str(v) for v in old_to_entry[start:start + 32]) + ',')
lines += ['};', '']

lines += [
    'void LIBMATTI_MC_BlockStateData_Of(int id, LIBMATTI_MC_BlockStateData *out)',
    '{',
    '    // Java: Dynamic<?> dynamic = null;',
    '    //   if (p_14953_ >= 0 && p_14953_ < MAP.length) { dynamic = MAP[p_14953_]; }',
    '    //   return dynamic == null ? MAP[0] : dynamic;',
    '    if (out == NULL)',
    '        return;',
    '    if (id < 0 || id >= 4096 || OLD_TO_ENTRY[id] < 0)',
    '        id = 0;',
    '    const int entry = OLD_TO_ENTRY[id];',
    '    const int variant = id - ENTRIES[entry].oldId;',
    '    if (variant < 0 || variant >= ENTRIES[entry].count)',
    '        id = 0;',
    '    const int fallback = OLD_TO_ENTRY[0];',
    '    if (id == 0 || OLD_TO_ENTRY[id] < 0)',
    '    {',
    '        const VARIANT *v = &VARIANTS[ENTRIES[fallback].firstVariant];',
    '        out->name = v->name;',
    '        out->props = NULL;',
    '        out->propCount = 0;',
    '        return;',
    '    }',
    '    const VARIANT *v = &VARIANTS[ENTRIES[entry].firstVariant + variant];',
    '    out->name = v->name;',
    '    out->props = v->props < 0 ? NULL : &PROPS[v->props][0];',
    '    out->propCount = v->props < 0 ? 0 : PROPS_COUNT[v->props];',
    '}',
    '',
    'const char *LIBMATTI_MC_BlockStateData_NameOf(int id)',
    '{',
    '    LIBMATTI_MC_BlockStateData data;',
    '    LIBMATTI_MC_BlockStateData_Of(id, &data);',
    '    return data.name;',
    '}',
    '',
]

open(out_path, 'w').write('\n'.join(lines))
print('%d entries, %d variants, %d property maps -> %s'
      % (len(entry_rows), len(variant_rows), len(prop_rows), out_path))
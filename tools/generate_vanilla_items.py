#!/usr/bin/env python3
# Regenerates VanillaItems.c from vendor/MCP-Reborn's Items.java, one entry per
# public static final Item in declaration order, with the per-item property
# chains translated 1:1 into the C ItemProperties calls.
#
#     python3 tools/generate_vanilla_items.py
import re
import sys

ITEMS_JAVA = 'vendor/MCP-Reborn/src/main/java/net/minecraft/world/item/Items.java'
OUT_FILE = 'library/src/libmatti/net/minecraft/server/bootstrap/VanillaItems.c'

# ---------------------------------------------------------------------------
# source extraction


def extract_statics(text):
    """Every `public static final Item NAME = EXPR;` with balanced parens."""
    out = []
    for m in re.finditer(r'public static final Item (\w+) = ', text):
        name = m.group(1)
        i = m.end()
        depth = 0
        for j in range(i, len(text)):
            c = text[j]
            if c in '([':
                depth += 1
            elif c in ')]':
                depth -= 1
            elif c == ';' and depth == 0:
                out.append((name, ' '.join(text[i:j].split())))
                break
    return out


def split_top_args(s):
    """Split a top-level comma list on depth 0."""
    args, depth, start = [], 0, 0
    for i, c in enumerate(s):
        if c in '([':
            depth += 1
        elif c in ')]':
            depth -= 1
        elif c == ',' and depth == 0:
            args.append(s[start:i].strip())
            start = i + 1
    args.append(s[start:].strip())
    return args


def call_chain(expr, param=None):
    """Parse `new Item.Properties().m(a).m(b)` or `param.m(a).m(b)` into
    [(method, args)] at top level."""
    chain = []
    # strip a leading `new Item.Properties()` / `param` receiver
    m = re.match(r'new Item\.Properties\(\)\s*', expr)
    if m:
        rest = expr[m.end():]
    elif param is not None:
        m = re.match(re.escape(param) + r'\s*', expr)
        if not m:
            return chain
        rest = expr[m.end():]
    else:
        return chain
    i = 0
    while i < len(rest):
        m = re.match(r'\.\s*(\w+)\s*\(', rest[i:])
        if not m:
            break
        method = m.group(1)
        start = i + m.end()
        depth, j = 1, start
        while j < len(rest) and depth:
            if rest[j] in '([':
                depth += 1
            elif rest[j] in ')]':
                depth -= 1
            j += 1
        chain.append((method, rest[start:j - 1]))
        i = j
    return chain


def find_call(expr, name):
    """The argument list of expr's top call when it is `name(...)`, else None."""
    m = re.match(re.escape(name) + r'\s*\(', expr)
    if not m:
        return None
    start = m.end()
    depth, j = 1, start
    while j < len(expr) and depth:
        if expr[j] in '([':
            depth += 1
        elif expr[j] in ')]':
            depth -= 1
        j += 1
    if depth:
        return None
    return expr[start:j - 1]


# ---------------------------------------------------------------------------
# chain translation


def c_float(v):
    v = v.strip()
    if re.fullmatch(r'-?\d+', v):
        return v
    return v.replace('F', 'f')


PAYLOAD_METHODS = {
    'food': 'payload component (game-port content)',
    'component': 'payload component (game-port content)',
    'trimMaterial': 'payload component (game-port content)',
    'jukeboxPlayable': 'payload component (game-port content)',
    'useCooldown': 'payload component (game-port content)',
    'equippableUnswappable': 'payload component (game-port content)',
    'usingConvertsTo': 'payload component (game-port content)',
    'spawnEgg': 'payload component ENTITY_DATA (game-port content)',
    'repairable': 'payload component (game-port content)',
    'attributes': 'attribute payload (game-port content)',
    'horseArmor': 'attributes + EQUIPPABLE payload (game-port content)',
    'nautilusArmor': 'attributes + EQUIPPABLE payload (game-port content)',
}


class Gen:
    def __init__(self):
        self.item_index = {}   # Java constant -> entry index
        self.errors = []

    def translate_chain(self, chain, comment):
        """Emit the C statements for one property chain."""
        lines = []
        for method, args in chain:
            a = split_top_args(args) if args.strip() else []
            if method == 'stacksTo':
                lines.append(f'    LIBMATTI_MC_ItemProperties_StacksTo(p, {a[0]});')
            elif method == 'durability':
                lines.append(f'    LIBMATTI_MC_ItemProperties_Durability(p, {a[0]});')
            elif method == 'enchantable':
                lines.append(f'    LIBMATTI_MC_ItemProperties_Enchantable(p, {a[0]});')
            elif method == 'rarity':
                lines.append(f'    LIBMATTI_MC_ItemProperties_Rarity(p, LIBMATTI_MC_Rarity_{a[0].split(".")[1]});')
            elif method == 'fireResistant':
                lines.append('    LIBMATTI_MC_ItemProperties_FireResistant(p);')
            elif method == 'craftRemainder':
                target = a[0].strip()
                idx = self.item_index.get(target)
                if idx is None:
                    self.errors.append(f'craftRemainder({target}) before declaration')
                    continue
                lines.append(f'    LIBMATTI_MC_ItemProperties_CraftRemainder(p, entries[{idx}].item);')
            elif method == 'useBlockDescriptionPrefix':
                lines.append('    LIBMATTI_MC_ItemProperties_UseBlockDescriptionPrefix(p);')
            elif method == 'useItemDescriptionPrefix':
                lines.append('    LIBMATTI_MC_ItemProperties_UseItemDescriptionPrefix(p);')
            elif method == 'overrideDescription':
                lines.append(f'    LIBMATTI_MC_ItemProperties_OverrideDescription(p, {a[0]});')
            elif method in ('sword', 'pickaxe', 'shovel', 'axe', 'hoe'):
                mat = a[0].split('.')[1]
                dmg, speed = c_float(a[1]), c_float(a[2])
                lines.append(f'    LIBMATTI_MC_ItemProperties_{method.capitalize()}(p, LIBMATTI_MC_ToolMaterial_{mat}(), {dmg}, {speed});')
            elif method == 'tool':
                mat = a[0].split('.')[1]
                lines.append(f'    LIBMATTI_MC_ItemProperties_Tool(p, LIBMATTI_MC_ToolMaterial_{mat}());')
            elif method == 'humanoidArmor':
                mat = a[0].split('.')[1]
                atype = a[1].split('.')[1]
                lines.append(f'    LIBMATTI_MC_ItemProperties_HumanoidArmor(p, LIBMATTI_MC_ArmorMaterials_{mat}(), LIBMATTI_MC_ArmorType_{atype});')
            elif method == 'wolfArmor':
                mat = a[0].split('.')[1]
                lines.append(f'    LIBMATTI_MC_ItemProperties_WolfArmor(p, LIBMATTI_MC_ArmorMaterials_{mat}());')
            elif method == 'spear':
                mat = a[0].split('.')[1]
                lines.append(f'    // Java: .spear({", ".join(a)}) - durability/enchantable below, the KINETIC_WEAPON payload is game-port content')
                lines.append(f'    LIBMATTI_MC_ItemProperties_Durability(p, LIBMATTI_MC_ToolMaterial_{mat}()->durability);')
                lines.append(f'    LIBMATTI_MC_ItemProperties_Enchantable(p, LIBMATTI_MC_ToolMaterial_{mat}()->enchantmentValue);')
            elif method in PAYLOAD_METHODS:
                lines.append(f'    // Java: .{method}({args}) - {PAYLOAD_METHODS[method]}')
            else:
                self.errors.append(f'unhandled method .{method}({args})')
                lines.append(f'    // Java: .{method}({args}) - TODO unhandled property')
        return lines, comment


def ctor_chain(expr, param):
    """The Item.Properties mutation a typed constructor applies
    (`p -> new ShovelItem(ToolMaterial.STONE, 1.5F, -3.0F, p)`)."""
    m = re.match(r'\w+\s*->\s*new\s+(\w+)\(', expr)
    if not m:
        return None
    cls = m.group(1)
    # re-find with the class name
    start = expr.find('new ' + cls) + len('new ' + cls)
    while start < len(expr) and expr[start] != '(':
        start += 1
    if start >= len(expr):
        return None
    depth, j = 1, start + 1
    while j < len(expr) and depth:
        if expr[j] in '([':
            depth += 1
        elif expr[j] in ')]':
            depth -= 1
        j += 1
    args = split_top_args(expr[start + 1:j - 1])
    toolmap = {'ShovelItem': 'Shovel', 'AxeItem': 'Axe', 'HoeItem': 'Hoe'}
    if cls in toolmap and len(args) == 4 and args[0].startswith('ToolMaterial.'):
        mat = args[0].split('.')[1]
        return [('tool' if cls == 'ShovelItem' else toolmap[cls].lower(),
                 f'{args[0]}, {args[1]}, {args[2]}')]
    return None


def resolve(entry_name, expr):
    """(id, block, base_chain, ctor_chain, notes) for one static."""
    # --- registerSpawnEgg ---
    args = find_call(expr, 'registerSpawnEgg')
    if args is not None:
        entity = args.split('.')[1].lower()
        return (f'{entity}_spawn_egg', None, [], [],
                [f'// Java: .spawnEgg({args}) - payload component ENTITY_DATA (game-port content)'])

    # --- WeatheringCopperItems.create - expands to 8 block items, skipped like
    # the previous generation (the 3 record statics are not single Items) ---
    if expr.startswith('WeatheringCopperItems.create'):
        return None

    # --- registerBlock / registerItem ---
    for helper in ('registerBlock', 'registerItem'):
        args = find_call(expr, helper)
        if args is None:
            continue
        parts = split_top_args(args)
        block, base_expr, ctor_expr = None, None, None
        if helper == 'registerBlock':
            block = parts[0].split('.')[1]
            rest = parts[1:]
            # multi-block variants: only the first block receives the item
            if rest and re.fullmatch(r'Blocks\.\w+', rest[0]):
                rest = []
            if rest:
                first = rest[0]
                if first.startswith('Blocks.'):
                    block = first.split('.')[1]
                    rest = rest[1:]
                elif re.match(r'\w+\s*->', first) or '::' in first or first.startswith('createBlockItemWithCustomItemName'):
                    ctor_expr = first
                    rest = rest[1:]
                elif first.startswith('new Item.Properties'):
                    base_expr = first
                    rest = rest[1:]
                else:
                    ctor_expr = first
                    rest = rest[1:]
            if rest:
                base_expr = rest[0]
        else:
            if re.match(r'net\.minecraft\.references\.Items\.\w+', parts[0]):
                item_id = parts[0].split('.')[-1].lower()
            else:
                item_id = parts[0].strip('"')
            rest = parts[1:]
            if rest:
                first = rest[0]
                if first.startswith('createBlockItemWithCustomItemName'):
                    block = find_call(first, 'createBlockItemWithCustomItemName').split('.')[1]
                    ctor_expr = 'p -> p.useItemDescriptionPrefix()'
                    rest = rest[1:]
                elif first.startswith('new Item.Properties'):
                    base_expr = first
                    rest = rest[1:]
                elif re.match(r'\w+\s*->', first) or '::' in first:
                    ctor_expr = first
                    rest = rest[1:]
                else:
                    base_expr = first
                    rest = rest[1:]
            if rest:
                base_expr = rest[0]
            return (item_id, block, call_chain(base_expr) if base_expr else [],
                    ctor_expr, [])

        item_id = block.lower() if block else None
        base_chain = call_chain(base_expr) if base_expr else []
        # UnaryOperator lambdas: `p -> p.component(...)` or `p -> p.m(...).n(...)`
        if ctor_expr and re.match(r'\w+\s*->\s*\w+\.', ctor_expr):
            param = re.match(r'(\w+)\s*->\s*(\w+)\.', ctor_expr).group(1)
            base_chain += call_chain(ctor_expr, param)
            ctor_expr = None
        return (item_id, block, base_chain, ctor_expr, [])

    # --- AIR: new AirItem(Blocks.AIR, new Item.Properties()) ---
    if expr.startswith('new AirItem'):
        return ('air', 'AIR', [], None, [])

    return 'unknown'


# ---------------------------------------------------------------------------


def main():
    src = open(ITEMS_JAVA).read()
    statics = extract_statics(src)

    gen = Gen()
    entries = []
    for name, expr in statics:
        r = resolve(name, expr)
        if r is None:  # WeatheringCopperItems record static
            entries.append((name, None, None, None, None,
                            [f'// Java: {name} = WeatheringCopperItems.create(...) - the record expands to 8 block items, covered by their own statics']))
            continue
        if r == 'unknown':
            gen.errors.append(f'{name}: unhandled expression {expr[:80]}')
            entries.append((name, name.lower(), None, None, None,
                            [f'// Java: {name} - unhandled expression']))
            continue
        item_id, block, base_chain, ctor_expr, notes = r
        chain = list(base_chain)
        if ctor_expr:
            cc = ctor_chain(ctor_expr, 'p')
            if cc:
                chain += cc
        ctor_comment = []
        if ctor_expr and not re.match(r'\w+\s*->\s*\w+\.', ctor_expr):
            ctor_comment = [f'// Java: {ctor_expr.split("(")[0].strip()} - typed item class (game-port content)']
        entries.append((name, item_id, block, chain, ctor_expr, notes + ctor_comment))

    # index map for craftRemainder back references
    for i, (name, item_id, block, chain, ctor_expr, notes) in enumerate(entries):
        gen.item_index[name] = i

    # ---- props functions ----
    props_fns = []
    used = set()
    for i, (name, item_id, block, chain, ctor_expr, notes) in enumerate(entries):
        if not chain:
            continue
        fn = f'props_{name}'
        used.add(name)
        body = []
        for line in notes:
            body.append(line)
        stmts, _ = gen.translate_chain(chain, None)
        body += stmts
        props_fns.append((fn, name, body))

    # ---- emit ----
    out = []
    # (the entries table must come before the props functions - craftRemainder
    # references entries[IDX].item)
    out.append('// Port of net.minecraft.world.item.Items (the registry-data part).')
    out.append('// Generated from the vendor Java sources (vendor/MCP-Reborn .../world/item/Items.java):')
    out.append('// one row per public static final Item, in Java declaration order. The block column is')
    out.append("// Java's BlockItem back-link (registerBlock / createBlockItemWithCustomItemName): the")
    out.append('// Blocks constant whose block the item places; NULL means a plain Item. The props')
    out.append('// column applies the per-item Item.Properties chain 1:1 (stacksTo, durability,')
    out.append('// rarity, ..., the payload components stay commented game-port content).')
    out.append('')
    out.append('#include "libmatti/net/minecraft/server/bootstrap/VanillaItems.h"')
    out.append('')
    out.append('#include "libmatti/net/minecraft/server/bootstrap/VanillaBlocks.h"')
    out.append('')
    out.append('#include "libmatti/net/minecraft/core/Registry.h"')
    out.append('#include "libmatti/net/minecraft/core/registries/BuiltInRegistries.h"')
    out.append('#include "libmatti/net/minecraft/core/registries/Registries.h"')
    out.append('#include "libmatti/net/minecraft/resources/Identifier.h"')
    out.append('#include "libmatti/net/minecraft/world/item/Item.h"')
    out.append('#include "libmatti/net/minecraft/world/item/Rarity.h"')
    out.append('#include "libmatti/net/minecraft/world/item/ToolMaterial.h"')
    out.append('#include "libmatti/net/minecraft/world/item/equipment/ArmorMaterials.h"')
    out.append('#include "libmatti/net/minecraft/world/item/equipment/ArmorType.h"')
    out.append('')
    out.append('#include <stdlib.h>')
    out.append('#include <string.h>')
    out.append('')
    out.append('enum')
    out.append('{')
    for i, (name, item_id, block, chain, ctor_expr, notes) in enumerate(entries):
        out.append(f'    {name} = {i},')
    out.append(f'    LIBMATTI_MC_VanillaItems_COUNT_LOCAL = {len(entries)}')
    out.append('};')
    out.append('')

    # entries table (declared first - the props functions reference entries[])
    out.append('typedef struct ItemEntry')
    out.append('{')
    out.append('    // Java: the static\'s name (Items.STONE)')
    out.append('    const char *name;')
    out.append('    // Java: registerItem("<id>", ...) / registerBlock(Blocks.X) - the ResourceKey path')
    out.append('    const char *id;')
    out.append('    // Java: new BlockItem(Blocks.X, ...) - the Blocks constant, NULL for a plain Item')
    out.append('    const char *block;')
    out.append('    // Java: the Item.Properties chain, NULL for default properties')
    out.append('    void (*props)(LIBMATTI_MC_ItemProperties *p);')
    out.append('    // Java: the Item static itself')
    out.append('    LIBMATTI_MC_Item *item;')
    out.append('} ItemEntry;')
    out.append('')
    out.append(f'static ItemEntry entries[{len(entries)}];')
    out.append('static int initialized = 0;')
    out.append('')

    # props functions
    for fn, name, body in props_fns:
        out.append(f'// Java: {name} - the Item.Properties chain')
        out.append(f'static void {fn}(LIBMATTI_MC_ItemProperties *p)')
        out.append('{')
        out += body
        out.append('}')
        out.append('')

    out.append('static void initialize(void)')
    out.append('{')
    out.append('    if (initialized)')
    out.append('        return;')
    out.append('    initialized = 1;')
    out.append('')
    for i, (name, item_id, block, chain, ctor_expr, notes) in enumerate(entries):
        for note in notes:
            out.append(f'    {note}')
        out.append(f'    entries[{name}].name = "{name}";')
        if item_id:
            out.append(f'    entries[{name}].id = "{item_id}";')
        out.append(f'    entries[{name}].block = {f'"{block}"' if block else 'NULL'};')
        out.append(f'    entries[{name}].props = {f"props_{name}" if name in used else "NULL"};')
    out.append('    // Java: new Item(Properties) / new BlockItem(Blocks.X, Properties) - the back-links')
    out.append('    // resolve now; every item gets the prototype map with its id so the description id')
    out.append('    // (item.<ns>.<path>) resolves at construction, exactly like Java\'s register()')
    out.append(f'    for (int i = 0; i < {len(entries)}; i++)')
    out.append('    {')
    out.append('        LIBMATTI_MC_Identifier *identifier = LIBMATTI_MC_Identifier_FromNamespaceAndPath("minecraft", entries[i].id);')
    out.append('        LIBMATTI_MC_ResourceKey *key =')
    out.append('            LIBMATTI_MC_ResourceKey_Create(LIBMATTI_MC_Registries_ITEM(), identifier);')
    out.append('        LIBMATTI_MC_ItemProperties *properties = LIBMATTI_MC_ItemProperties_New();')
    out.append('        LIBMATTI_MC_ItemProperties_SetId(properties, key);')
    out.append('        if (entries[i].props != NULL)')
    out.append('            entries[i].props(properties);')
    out.append('        if (entries[i].block == NULL)')
    out.append('            entries[i].item = LIBMATTI_MC_Item_New(properties);')
    out.append('        else')
    out.append('            entries[i].item = LIBMATTI_MC_BlockItem_New(LIBMATTI_MC_VanillaBlocks_GetByName(entries[i].block), properties);')
    out.append('        LIBMATTI_MC_Identifier_Free(identifier);')
    out.append('    }')
    out.append('}')

    # keep the public functions of the current file (register/All/GetByName/...)
    tail = open(OUT_FILE).read()
    marker = '// Java: the static initialisation of Items'
    idx = tail.find(marker)
    if idx < 0:
        raise SystemExit('cannot find the public-function tail in the current file')
    out.append('')
    out.append(tail[idx:])

    open(OUT_FILE, 'w').write('\n'.join(out) + '\n')

    print(f'entries: {len(entries)}, props functions: {len(props_fns)}')
    if gen.errors:
        print('ERRORS:')
        for e in gen.errors:
            print('  ', e)
        sys.exit(1)


if __name__ == '__main__':
    main()

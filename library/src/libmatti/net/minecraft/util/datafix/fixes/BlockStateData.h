// Port of net.minecraft.util.datafix.fixes.BlockStateData (P7.3) - the 1.13
// numeric block id table. The table itself is generated from the Java source by
// tools/gen_block_state_data.py.

#ifndef MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXBLOCKSTATEDATA_H
#define MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXBLOCKSTATEDATA_H

#ifdef __cplusplus
extern "C" {
#endif

// Java: the Dynamic<?> a create(name, properties) tag holds - the block id and
// its property map (the Dynamic's "Name"/"Properties" fields)
typedef struct LIBMATTI_MC_BlockStateData
{
    const char *name;
    // flat key/value pairs, propCount * 2 entries
    const char *const *props;
    int propCount;
} LIBMATTI_MC_BlockStateData;

// Java: public static Dynamic<?> getTag(int) - an id outside the table falls
// back to air (MAP[0]), exactly like the Java lookup does.
void LIBMATTI_MC_BlockStateData_Of(int id, LIBMATTI_MC_BlockStateData *out);

// Java: the block id of getTag(id).get("Name")
const char *LIBMATTI_MC_BlockStateData_NameOf(int id);

#ifdef __cplusplus
}
#endif

#endif // MATTICRAFT_NET_MINECRAFT_UTIL_DATAFIX_FIXBLOCKSTATEDATA_H
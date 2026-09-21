// Port of net.minecraft.world.level.block.state.properties.NoteBlockInstrument.

#include "libmatti/net/minecraft/world/level/block/state/properties/NoteBlockInstrument.h"

const char *LIBMATTI_MC_NoteBlockInstrument_SerializedName(LIBMATTI_MC_NoteBlockInstrument instrument)
{
    switch (instrument)
    {
    case LIBMATTI_MC_NoteBlockInstrument_HARP: return "harp";
    case LIBMATTI_MC_NoteBlockInstrument_BASEDRUM: return "basedrum";
    case LIBMATTI_MC_NoteBlockInstrument_SNARE: return "snare";
    case LIBMATTI_MC_NoteBlockInstrument_HAT: return "hat";
    case LIBMATTI_MC_NoteBlockInstrument_BASS: return "bass";
    case LIBMATTI_MC_NoteBlockInstrument_FLUTE: return "flute";
    case LIBMATTI_MC_NoteBlockInstrument_BELL: return "bell";
    case LIBMATTI_MC_NoteBlockInstrument_GUITAR: return "guitar";
    case LIBMATTI_MC_NoteBlockInstrument_CHIME: return "chime";
    case LIBMATTI_MC_NoteBlockInstrument_XYLOPHONE: return "xylophone";
    case LIBMATTI_MC_NoteBlockInstrument_IRON_XYLOPHONE: return "iron_xylophone";
    case LIBMATTI_MC_NoteBlockInstrument_COW_BELL: return "cow_bell";
    case LIBMATTI_MC_NoteBlockInstrument_DIDGERIDOO: return "didgeridoo";
    case LIBMATTI_MC_NoteBlockInstrument_BIT: return "bit";
    case LIBMATTI_MC_NoteBlockInstrument_BANJO: return "banjo";
    case LIBMATTI_MC_NoteBlockInstrument_PLING: return "pling";
    case LIBMATTI_MC_NoteBlockInstrument_ZOMBIE: return "zombie";
    case LIBMATTI_MC_NoteBlockInstrument_SKELETON: return "skeleton";
    case LIBMATTI_MC_NoteBlockInstrument_CREEPER: return "creeper";
    case LIBMATTI_MC_NoteBlockInstrument_DRAGON: return "dragon";
    case LIBMATTI_MC_NoteBlockInstrument_WITHER_SKELETON: return "wither_skeleton";
    case LIBMATTI_MC_NoteBlockInstrument_PIGLIN: return "piglin";
    case LIBMATTI_MC_NoteBlockInstrument_CUSTOM_HEAD: return "custom_head";
    }
    return "harp";
}

// Port of net.minecraft.world.level.block.state.properties.NoteBlockInstrument.
// Java: implements StringRepresentable - the serialized names are carried next to
// the kind tag (the sound/harshness parts stay with the game port).

#ifndef MATTICRAFT_MC_BLOCKSTATE_PROPERTIES_NOTEBLOCKINSTRUMENT_H
#define MATTICRAFT_MC_BLOCKSTATE_PROPERTIES_NOTEBLOCKINSTRUMENT_H

// Java: public enum NoteBlockInstrument - in declaration order
typedef enum
{
    LIBMATTI_MC_NoteBlockInstrument_HARP,
    LIBMATTI_MC_NoteBlockInstrument_BASEDRUM,
    LIBMATTI_MC_NoteBlockInstrument_SNARE,
    LIBMATTI_MC_NoteBlockInstrument_HAT,
    LIBMATTI_MC_NoteBlockInstrument_BASS,
    LIBMATTI_MC_NoteBlockInstrument_FLUTE,
    LIBMATTI_MC_NoteBlockInstrument_BELL,
    LIBMATTI_MC_NoteBlockInstrument_GUITAR,
    LIBMATTI_MC_NoteBlockInstrument_CHIME,
    LIBMATTI_MC_NoteBlockInstrument_XYLOPHONE,
    LIBMATTI_MC_NoteBlockInstrument_IRON_XYLOPHONE,
    LIBMATTI_MC_NoteBlockInstrument_COW_BELL,
    LIBMATTI_MC_NoteBlockInstrument_DIDGERIDOO,
    LIBMATTI_MC_NoteBlockInstrument_BIT,
    LIBMATTI_MC_NoteBlockInstrument_BANJO,
    LIBMATTI_MC_NoteBlockInstrument_PLING,
    LIBMATTI_MC_NoteBlockInstrument_ZOMBIE,
    LIBMATTI_MC_NoteBlockInstrument_SKELETON,
    LIBMATTI_MC_NoteBlockInstrument_CREEPER,
    LIBMATTI_MC_NoteBlockInstrument_DRAGON,
    LIBMATTI_MC_NoteBlockInstrument_WITHER_SKELETON,
    LIBMATTI_MC_NoteBlockInstrument_PIGLIN,
    LIBMATTI_MC_NoteBlockInstrument_CUSTOM_HEAD
} LIBMATTI_MC_NoteBlockInstrument;

// Java: StringRepresentable.getSerializedName()
const char *LIBMATTI_MC_NoteBlockInstrument_SerializedName(LIBMATTI_MC_NoteBlockInstrument instrument);

#endif

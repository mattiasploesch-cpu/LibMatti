// Port of net.minecraft.world.level.material.PushReaction
// and net.minecraft.world.level.block.state.properties.NoteBlockInstrument.

#ifndef MATTICRAFT_MC_WORLD_LEVEL_MATERIAL_PUSHREACTION_H
#define MATTICRAFT_MC_WORLD_LEVEL_MATERIAL_PUSHREACTION_H

// Java: public enum PushReaction { NORMAL, DESTROY, BLOCK, PUSH_ONLY, IGNORE }
typedef enum
{
    LIBMATTI_MC_PushReaction_NORMAL,
    LIBMATTI_MC_PushReaction_DESTROY,
    LIBMATTI_MC_PushReaction_BLOCK,
    LIBMATTI_MC_PushReaction_PUSH_ONLY,
    LIBMATTI_MC_PushReaction_IGNORE
} LIBMATTI_MC_PushReaction;

#endif

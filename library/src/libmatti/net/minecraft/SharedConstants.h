// Port of net.minecraft.SharedConstants.
// Only the constants the ported code reads live here; the debug-flag surface is
// the DEBUG_* system-property family, evaluated like Java's booleanProperty.

#ifndef MATTICRAFT_MC_SHAREDCONSTANTS_H
#define MATTICRAFT_MC_SHAREDCONSTANTS_H

// Java: public static final int RESOURCE_PACK_FORMAT_MAJOR / MINOR
#define LIBMATTI_MC_SharedConstants_RESOURCE_PACK_FORMAT_MAJOR 75
#define LIBMATTI_MC_SharedConstants_RESOURCE_PACK_FORMAT_MINOR 0
// Java: public static final int DATA_PACK_FORMAT_MAJOR / MINOR
#define LIBMATTI_MC_SharedConstants_DATA_PACK_FORMAT_MAJOR 94
#define LIBMATTI_MC_SharedConstants_DATA_PACK_FORMAT_MINOR 1
// Java: public static final boolean IS_RUNNING_IN_IDE
extern int LIBMATTI_MC_SharedConstants_IS_RUNNING_IN_IDE;

// Java: public static WorldVersion getCurrentVersion() - the port carries the
// pack versions the launcher runs as (1.21.11: 75.0 / 94.1)
int LIBMATTI_MC_SharedConstants_GetResourcePackVersionMajor(void);
int LIBMATTI_MC_SharedConstants_GetResourcePackVersionMinor(void);
int LIBMATTI_MC_SharedConstants_GetDataPackVersionMajor(void);
int LIBMATTI_MC_SharedConstants_GetDataPackVersionMinor(void);

#endif //MATTICRAFT_MC_SHAREDCONSTANTS_H

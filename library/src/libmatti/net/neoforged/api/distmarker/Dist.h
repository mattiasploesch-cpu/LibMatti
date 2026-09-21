#ifndef MATTICRAFT_DISTMARKER_DIST_H
#define MATTICRAFT_DISTMARKER_DIST_H

// Port of net.neoforged.api.distmarker.Dist - the enum with the two values FML
// passes to the entrypoints (the api artifact ships only annotations besides it).
typedef enum
{
    LIBMATTI_DIST_CLIENT,
    LIBMATTI_DIST_DEDICATED_SERVER
} LIBMATTI_DIST_Dist;

#endif //MATTICRAFT_DISTMARKER_DIST_H

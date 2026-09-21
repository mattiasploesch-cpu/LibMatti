//
// Port of cpw.mods.modlauncher.LogMarkers.
// Java markers come from org.apache.logging.log4j.MarkerManager; the C port
// keeps only the marker names.
//

#ifndef MATTICRAFT_MODLAUNCHER_LOGMARKERS_H
#define MATTICRAFT_MODLAUNCHER_LOGMARKERS_H

typedef struct LIBMATTI_ML_LogMarker
{
    const char *name;
} LIBMATTI_ML_LogMarker;

// Java: static final Marker MODLAUNCHER = MarkerManager.getMarker("MODLAUNCHER");
extern const LIBMATTI_ML_LogMarker LIBMATTI_ML_MARKER_MODLAUNCHER;
// Java: ClassLoaderContextSelector marker "CLASSLOADING" with parent MODLAUNCHER
extern const LIBMATTI_ML_LogMarker LIBMATTI_ML_MARKER_CLASSLOADING;
// Java: CLASSLOADING sibling "LAUNCHPLUGIN" with parent MODLAUNCHER
extern const LIBMATTI_ML_LogMarker LIBMATTI_ML_MARKER_LAUNCHPLUGIN;

#endif //MATTICRAFT_MODLAUNCHER_LOGMARKERS_H

// Port of net.neoforged.fml.loading.LogMarkers.
// Java markers come from org.slf4j.MarkerFactory; the C port keeps only the
// marker names and reuses the logger's marker type.

#ifndef MATTICRAFT_FML_LOADING_LOGMARKERS_H
#define MATTICRAFT_FML_LOADING_LOGMARKERS_H

#include "libmatti/cpw/modlauncher/LogMarkers.h"

// Java: public static final Marker CORE = MarkerFactory.getMarker("CORE");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_MARKER_CORE;
// Java: public static final Marker LOADING = MarkerFactory.getMarker("LOADING");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_MARKER_LOADING;
// Java: public static final Marker SCAN = MarkerFactory.getMarker("SCAN");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_MARKER_SCAN;
// Java: public static final Marker SPLASH = MarkerFactory.getMarker("SPLASH");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_MARKER_SPLASH;

#endif //MATTICRAFT_FML_LOADING_LOGMARKERS_H

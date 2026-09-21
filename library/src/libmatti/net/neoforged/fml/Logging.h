// Port of net.neoforged.fml.Logging.
// Java markers come from org.apache.logging.log4j.MarkerManager; the C port keeps the marker
// names and reuses the logger's marker type (parents are therefore not modelled).

#ifndef MATTICRAFT_FML_LOGGING_H
#define MATTICRAFT_FML_LOGGING_H

#include "libmatti/cpw/modlauncher/LogMarkers.h"

// Java: public static final Marker CORE = MarkerManager.getMarker("CORE");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_Logging_CORE;
// Java: public static final Marker LOADING = MarkerManager.getMarker("LOADING");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_Logging_LOADING;
// Java: public static final Marker SCAN = MarkerManager.getMarker("SCAN");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_Logging_SCAN;
// Java: public static final Marker SPLASH = MarkerManager.getMarker("SPLASH");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_Logging_SPLASH;
// Java: public static final Marker CAPABILITIES = MarkerManager.getMarker("CAPABILITIES");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_Logging_CAPABILITIES;
// Java: public static final Marker MODELLOADING = MarkerManager.getMarker("MODELLOADING");
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_Logging_MODELLOADING;
// Java: public static final Marker FORGEMOD = MarkerManager.getMarker("FORGEMOD").addParents(LOADING);
extern const LIBMATTI_ML_LogMarker LIBMATTI_FML_Logging_FORGEMOD;

#endif //MATTICRAFT_FML_LOGGING_H

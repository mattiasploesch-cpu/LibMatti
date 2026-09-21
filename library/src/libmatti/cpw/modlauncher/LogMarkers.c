//
// Port of cpw.mods.modlauncher.LogMarkers.
//

#include "LogMarkers.h"

// Java: static final Marker MODLAUNCHER = MarkerManager.getMarker("MODLAUNCHER")
const LIBMATTI_ML_LogMarker LIBMATTI_ML_MARKER_MODLAUNCHER = {"MODLAUNCHER"};
// Java: MarkerManager.getMarker("CLASSLOADING").addParents(MODLAUNCHER)
const LIBMATTI_ML_LogMarker LIBMATTI_ML_MARKER_CLASSLOADING = {"CLASSLOADING"};
// Java: MarkerManager.getMarker("LAUNCHPLUGIN").addParents(MODLAUNCHER)
const LIBMATTI_ML_LogMarker LIBMATTI_ML_MARKER_LAUNCHPLUGIN = {"LAUNCHPLUGIN"};

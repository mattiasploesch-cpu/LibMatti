// Port of net.neoforged.fml.loading.LibraryFinder.

#ifndef MATTICRAFT_FML_LOADING_LIBRARYFINDER_H
#define MATTICRAFT_FML_LOADING_LIBRARYFINDER_H

#include "libmatti/net/neoforged/fml/loading/MavenCoordinate.h"

// Java: static Path findLibsPath() - a new string, NULL when the property is missing
char *LIBMATTI_FML_LibraryFinder_FindLibsPath(void);
// Java: public static Path findPathForMaven(String group, String artifact, String extension, String classifier, String version)
char *LIBMATTI_FML_LibraryFinder_FindPathForMaven(const char *group, const char *artifact, const char *extension,
                                                  const char *classifier, const char *version);
// Java: public static Path findPathForMaven(MavenCoordinate artifact)
char *LIBMATTI_FML_LibraryFinder_FindPathForMavenCoordinate(const LIBMATTI_FML_MavenCoordinate *artifact);

#endif //MATTICRAFT_FML_LOADING_LIBRARYFINDER_H

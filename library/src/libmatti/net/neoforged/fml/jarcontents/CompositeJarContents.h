// Port of net.neoforged.fml.jarcontents.CompositeJarContents.

#ifndef MATTICRAFT_FML_JARCONTENTS_COMPOSITEJARCONTENTS_H
#define MATTICRAFT_FML_JARCONTENTS_COMPOSITEJARCONTENTS_H

#include "libmatti/net/neoforged/fml/jarcontents/JarContents.h"

// Java: public CompositeJarContents(List<JarContents> delegates)
LIBMATTI_FML_JarContents *LIBMATTI_FML_CompositeJarContents_New(LIBMATTI_FML_JarContents **delegates,
                                                               LIBMATTI_FML_JarContents_PathFilter *filters,
                                                               void **filterUserdata, size_t delegateCount);

// Java: public boolean isFiltered()
int LIBMATTI_FML_CompositeJarContents_IsFiltered(const LIBMATTI_FML_JarContents *contents);
// Java: public List<JarContents> getDelegates()
LIBMATTI_FML_JarContents **LIBMATTI_FML_CompositeJarContents_GetDelegates(const LIBMATTI_FML_JarContents *contents,
                                                                         size_t *count);

#endif //MATTICRAFT_FML_JARCONTENTS_COMPOSITEJARCONTENTS_H

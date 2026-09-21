// Port of net.neoforged.fml.jarcontents.JarResourceVisitor.

#ifndef MATTICRAFT_FML_JARCONTENTS_JARRESOURCEVISITOR_H
#define MATTICRAFT_FML_JARCONTENTS_JARRESOURCEVISITOR_H

#include "libmatti/net/neoforged/fml/jarcontents/JarResource.h"

// Java: @FunctionalInterface public interface JarResourceVisitor { void visit(String relativePath, JarResource resource); }
typedef void (*LIBMATTI_FML_JarResourceVisitor)(const char *relativePath, LIBMATTI_FML_JarResource *resource,
                                               void *userdata);

#endif //MATTICRAFT_FML_JARCONTENTS_JARRESOURCEVISITOR_H

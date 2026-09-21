// Port of net.neoforged.fml.jarcontents.JarResourceAttributes.

#ifndef MATTICRAFT_FML_JARCONTENTS_JARRESOURCEATTRIBUTES_H
#define MATTICRAFT_FML_JARCONTENTS_JARRESOURCEATTRIBUTES_H

// Java: public record JarResourceAttributes(FileTime lastModified, long size)
typedef struct
{
    // Java: FileTime lastModified
    long lastModifiedMillis;
    long size;
} LIBMATTI_FML_JarResourceAttributes;

#endif //MATTICRAFT_FML_JARCONTENTS_JARRESOURCEATTRIBUTES_H

// Port of net.neoforged.neoforgespi.transformation.ProcessorName.

#ifndef MATTICRAFT_NEOFORGESPI_TRANSFORMATION_PROCESSORNAME_H
#define MATTICRAFT_NEOFORGESPI_TRANSFORMATION_PROCESSORNAME_H

// Java: public record ProcessorName(String namespace, String path) implements Comparable<ProcessorName>
typedef struct LIBMATTI_NEOFORGESPI_ProcessorName
{
    char *namespace;
    char *path;
} LIBMATTI_NEOFORGESPI_ProcessorName;

// Java: the compact constructor. Java throws IllegalArgumentException for an invalid name;
// the C port reports that as NULL.
LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ProcessorName_New(const char *namespace, const char *path);
// Java: public static ProcessorName parse(String fullName)
LIBMATTI_NEOFORGESPI_ProcessorName *LIBMATTI_NEOFORGESPI_ProcessorName_Parse(const char *fullName);
void LIBMATTI_NEOFORGESPI_ProcessorName_Free(LIBMATTI_NEOFORGESPI_ProcessorName *name);

// Java: @Override public String toString() - caller frees
char *LIBMATTI_NEOFORGESPI_ProcessorName_ToString(const LIBMATTI_NEOFORGESPI_ProcessorName *name);
// Java: public int compareTo(ProcessorName other)
int LIBMATTI_NEOFORGESPI_ProcessorName_CompareTo(const LIBMATTI_NEOFORGESPI_ProcessorName *name,
                                                 const LIBMATTI_NEOFORGESPI_ProcessorName *other);

#endif //MATTICRAFT_NEOFORGESPI_TRANSFORMATION_PROCESSORNAME_H

// Port of net.neoforged.neoforgespi.locating.ModFileDiscoveryAttributes.

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_MODFILEDISCOVERYATTRIBUTES_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_MODFILEDISCOVERYATTRIBUTES_H

// TODO: net.neoforged.neoforgespi.locating.IModFile
typedef struct LIBMATTI_NEOFORGESPI_IModFile LIBMATTI_NEOFORGESPI_IModFile;
// TODO: net.neoforged.neoforgespi.locating.IModFileReader
typedef struct LIBMATTI_NEOFORGESPI_IModFileReader LIBMATTI_NEOFORGESPI_IModFileReader;
// TODO: net.neoforged.neoforgespi.locating.IModFileCandidateLocator
typedef struct LIBMATTI_NEOFORGESPI_IModFileCandidateLocator LIBMATTI_NEOFORGESPI_IModFileCandidateLocator;
// TODO: net.neoforged.neoforgespi.locating.IDependencyLocator
typedef struct LIBMATTI_NEOFORGESPI_IDependencyLocator LIBMATTI_NEOFORGESPI_IDependencyLocator;

// Java: public record ModFileDiscoveryAttributes(@Nullable IModFile parent, @Nullable IModFileReader reader,
//         @Nullable IModFileCandidateLocator locator, @Nullable IDependencyLocator dependencyLocator)
typedef struct LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes
{
    LIBMATTI_NEOFORGESPI_IModFile *parent; // Java: @Nullable IModFile parent
    LIBMATTI_NEOFORGESPI_IModFileReader *reader; // Java: @Nullable IModFileReader reader
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *locator; // Java: @Nullable IModFileCandidateLocator locator
    LIBMATTI_NEOFORGESPI_IDependencyLocator *dependencyLocator; // Java: @Nullable IDependencyLocator dependencyLocator
} LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes;

// Java: public static final ModFileDiscoveryAttributes DEFAULT = new ModFileDiscoveryAttributes(null, null, null, null)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Default(void);

// Java: public ModFileDiscoveryAttributes withParent(IModFile parent)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithParent(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes, LIBMATTI_NEOFORGESPI_IModFile *parent);
// Java: public ModFileDiscoveryAttributes withReader(IModFileReader reader)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithReader(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes, LIBMATTI_NEOFORGESPI_IModFileReader *reader);
// Java: public ModFileDiscoveryAttributes withLocator(IModFileCandidateLocator locator)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithLocator(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IModFileCandidateLocator *locator);
// Java: public ModFileDiscoveryAttributes withDependencyLocator(IDependencyLocator dependencyLocator)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_WithDependencyLocator(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    LIBMATTI_NEOFORGESPI_IDependencyLocator *dependencyLocator);
// Java: public ModFileDiscoveryAttributes merge(ModFileDiscoveryAttributes attributes)
LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_Merge(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes,
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *other);
// Java: @Override public String toString()
char *LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes_ToString(
    const LIBMATTI_NEOFORGESPI_ModFileDiscoveryAttributes *attributes);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_MODFILEDISCOVERYATTRIBUTES_H

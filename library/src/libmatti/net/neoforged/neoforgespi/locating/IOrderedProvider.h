// Port of net.neoforged.neoforgespi.locating.IOrderedProvider.

#ifndef MATTICRAFT_NEOFORGESPI_LOCATING_IORDEREDPROVIDER_H
#define MATTICRAFT_NEOFORGESPI_LOCATING_IORDEREDPROVIDER_H

// Java: int DEFAULT_PRIORITY
#define LIBMATTI_NEOFORGESPI_IOrderedProvider_DEFAULT_PRIORITY 0
// Java: int HIGHEST_SYSTEM_PRIORITY
#define LIBMATTI_NEOFORGESPI_IOrderedProvider_HIGHEST_SYSTEM_PRIORITY 1000
// Java: int LOWEST_SYSTEM_PRIORITY
#define LIBMATTI_NEOFORGESPI_IOrderedProvider_LOWEST_SYSTEM_PRIORITY -1000

// Java: public interface IOrderedProvider
typedef struct LIBMATTI_NEOFORGESPI_IOrderedProvider LIBMATTI_NEOFORGESPI_IOrderedProvider;

struct LIBMATTI_NEOFORGESPI_IOrderedProvider
{
    void *self;

    // Java: default int getPriority()
    int (*getPriority)(void *self);
};

// Java: public default int getPriority()
int LIBMATTI_NEOFORGESPI_IOrderedProvider_GetPriority(const LIBMATTI_NEOFORGESPI_IOrderedProvider *provider);

#endif //MATTICRAFT_NEOFORGESPI_LOCATING_IORDEREDPROVIDER_H

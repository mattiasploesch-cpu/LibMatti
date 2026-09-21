// Port of net.neoforged.neoforgespi.IIssueReporting.

#ifndef MATTICRAFT_NEOFORGESPI_IISSUEREPORTING_H
#define MATTICRAFT_NEOFORGESPI_IISSUEREPORTING_H

#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"

// Java: public interface IIssueReporting
typedef struct LIBMATTI_NEOFORGESPI_IIssueReporting LIBMATTI_NEOFORGESPI_IIssueReporting;

struct LIBMATTI_NEOFORGESPI_IIssueReporting
{
    void *self;

    // Java: void addIssue(ModLoadingIssue issue)
    void (*addIssue)(void *self, LIBMATTI_FML_ModLoadingIssue *issue);
};

// Java: public void addIssue(ModLoadingIssue issue)
void LIBMATTI_NEOFORGESPI_IIssueReporting_AddIssue(const LIBMATTI_NEOFORGESPI_IIssueReporting *reporter,
                                                   LIBMATTI_FML_ModLoadingIssue *issue);

#endif //MATTICRAFT_NEOFORGESPI_IISSUEREPORTING_H

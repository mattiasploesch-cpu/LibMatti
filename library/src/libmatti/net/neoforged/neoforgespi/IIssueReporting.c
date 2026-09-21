#include "libmatti/net/neoforged/neoforgespi/IIssueReporting.h"

// Java: public void addIssue(ModLoadingIssue issue)
void LIBMATTI_NEOFORGESPI_IIssueReporting_AddIssue(const LIBMATTI_NEOFORGESPI_IIssueReporting *reporter,
                                                   LIBMATTI_FML_ModLoadingIssue *issue)
{
    reporter->addIssue(reporter->self, issue);
}

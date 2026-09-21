// Port of net.neoforged.fml.ModLoadingException.

#ifndef MATTICRAFT_FML_MODLOADINGEXCEPTION_H
#define MATTICRAFT_FML_MODLOADINGEXCEPTION_H

#include "libmatti/java/lang/Throwable.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"

#include <stddef.h>

// Java: public class ModLoadingException extends RuntimeException
typedef struct
{
    LIBMATTI_JL_Throwable base;
    LIBMATTI_FML_ModLoadingIssue **issues;
    size_t issueCount;
} LIBMATTI_FML_ModLoadingException;

// Java: public ModLoadingException(ModLoadingIssue issue)
LIBMATTI_FML_ModLoadingException *LIBMATTI_FML_ModLoadingException_New(LIBMATTI_FML_ModLoadingIssue *issue);
// Java: public ModLoadingException(List<ModLoadingIssue> issues)
LIBMATTI_FML_ModLoadingException *LIBMATTI_FML_ModLoadingException_NewWithIssues(LIBMATTI_FML_ModLoadingIssue **issues,
                                                                                 size_t issueCount);

// Java: public List<ModLoadingIssue> getIssues()
LIBMATTI_FML_ModLoadingIssue **LIBMATTI_FML_ModLoadingException_GetIssues(
    const LIBMATTI_FML_ModLoadingException *exception, size_t *count);

// Java: @Override public String getMessage() - the returned string is new, the caller frees it
char *LIBMATTI_FML_ModLoadingException_GetMessage(const LIBMATTI_FML_ModLoadingException *exception);

void LIBMATTI_FML_ModLoadingException_Free(LIBMATTI_FML_ModLoadingException *exception);

#endif //MATTICRAFT_FML_MODLOADINGEXCEPTION_H

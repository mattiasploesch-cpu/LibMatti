// Port of net.neoforged.fml.i18n.FMLTranslations.

#ifndef MATTICRAFT_FML_FMLTRANSLATIONS_H
#define MATTICRAFT_FML_FMLTRANSLATIONS_H

#include "libmatti/java/lang/StringBuilder.h"
#include "libmatti/java/lang/Throwable.h"
#include "libmatti/net/neoforged/fml/ModLoadingIssue.h"

#include <stddef.h>

// Java: CUSTOM_FACTORIES keys (org.apache.commons.lang3.text.FormatFactory is external)
extern const char *const LIBMATTI_FML_FMLTranslations_CUSTOM_FACTORY_NAMES[];
#define LIBMATTI_FML_FMLTRANSLATIONS_CUSTOM_FACTORY_COUNT 10

// Java: public static String getPattern(String patternName, Supplier<String> fallback)
char *LIBMATTI_FML_FMLTranslations_GetPattern(const char *patternName, const char *fallback);
// Java: public static String parseMessage(String i18nMessage, Object... args)
char *LIBMATTI_FML_FMLTranslations_ParseMessage(const char *i18nMessage, const char **args, size_t argCount);
// Java: public static String parseMessageWithFallback(String i18nMessage, Supplier<String> fallback, Object... args)
char *LIBMATTI_FML_FMLTranslations_ParseMessageWithFallback(const char *i18nMessage, const char *fallback,
                                                            const char **args, size_t argCount);
// Java: public static String parseEnglishMessage(String i18n, Object... args)
char *LIBMATTI_FML_FMLTranslations_ParseEnglishMessage(const char *i18n, const char **args, size_t argCount);
// Java: public static String parseFormat(String format, Object... args)
char *LIBMATTI_FML_FMLTranslations_ParseFormat(const char *format, const char **args, size_t argCount);
// Java: public static String translateIssueEnglish(ModLoadingIssue issue) - NULL = the Java method threw
char *LIBMATTI_FML_FMLTranslations_TranslateIssueEnglish(const LIBMATTI_FML_ModLoadingIssue *issue);
// Java: public static String translateIssue(ModLoadingIssue issue) - NULL = the Java method threw
char *LIBMATTI_FML_FMLTranslations_TranslateIssue(const LIBMATTI_FML_ModLoadingIssue *issue);
// Java: public static String stripControlCodes(String text)
char *LIBMATTI_FML_FMLTranslations_StripControlCodes(const char *text);

// Java: private static Object[] getTranslationArgs(ModLoadingIssue issue)
// Returns the padded argument list (index 0.. and the implicit args at 100/101/102); the caller frees it.
char **LIBMATTI_FML_FMLTranslations_GetTranslationArgs(const LIBMATTI_FML_ModLoadingIssue *issue, size_t *count);
// Java: private static Object formatArg(Object arg)
char *LIBMATTI_FML_FMLTranslations_FormatArg(const char *arg);

// Java: private static void formatException(StringBuffer stringBuffer, Throwable t, String args)
void LIBMATTI_FML_FMLTranslations_FormatException(LIBMATTI_JL_StringBuilder *stringBuffer, LIBMATTI_JL_Throwable *t,
                                                  const char *args);
// Java: private static void formatOrNull(StringBuffer stringBuffer, Object o, String args)
void LIBMATTI_FML_FMLTranslations_FormatOrNull(LIBMATTI_JL_StringBuilder *stringBuffer, const char *value,
                                               const char *args);
// Java: private static void formatOptional(StringBuffer stringBuffer, Optional<?> value, String args)
void LIBMATTI_FML_FMLTranslations_FormatOptional(LIBMATTI_JL_StringBuilder *stringBuffer, const char *value,
                                                 const char *args);

#endif //MATTICRAFT_FML_FMLTRANSLATIONS_H

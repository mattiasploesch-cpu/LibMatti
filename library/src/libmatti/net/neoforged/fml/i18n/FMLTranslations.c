#include "libmatti/net/neoforged/fml/i18n/FMLTranslations.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/i18n/I18nManager.h"
#include "libmatti/net/neoforged/fml/loading/LogMarkers.h"
#include "libmatti/net/neoforged/fml/util/PathPrettyPrinting.h"
#include "libmatti/net/neoforged/neoforgespi/language/IModInfo.h"
#include "libmatti/org/apache/commons/lang3/text/StrSubstitutor.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: private static final Logger LOGGER = LogManager.getLogger();
static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

const char *const LIBMATTI_FML_FMLTranslations_CUSTOM_FACTORY_NAMES[LIBMATTI_FML_FMLTRANSLATIONS_CUSTOM_FACTORY_COUNT] = {
    "modinfo", "lower", "upper", "exc", "vr", "featurebound", "i18n", "i18ntranslate", "ornull", "optional"
};

// Java: PATTERN_CONTROL_CODE = Pattern.compile("(?i)\u00A7[0-9A-FK-OR]")
static int is_control_code_char(char value)
{
    char upper = (char) toupper((unsigned char) value);
    return (value >= '0' && value <= '9') || (upper >= 'A' && upper <= 'F') || upper == 'K' || upper == 'O'
           || upper == 'R';
}

// Java: public static String stripControlCodes(String text)
char *LIBMATTI_FML_FMLTranslations_StripControlCodes(const char *text)
{
    size_t length = strlen(text);
    char *result = malloc(length + 1);
    size_t written = 0;

    for (size_t i = 0; i < length; i++)
    {
        // Java: \u00A7 is the section sign, UTF-8 encoded as C2 A7
        if (i + 2 < length && (unsigned char) text[i] == 0xC2 && (unsigned char) text[i + 1] == 0xA7
            && is_control_code_char(text[i + 2]))
        {
            i += 2;
            continue;
        }

        result[written++] = text[i];
    }

    result[written] = '\0';
    return result;
}

// Java: public static String getPattern(String patternName, Supplier<String> fallback)
char *LIBMATTI_FML_FMLTranslations_GetPattern(const char *patternName, const char *fallback)
{
    // Java: var translated = I18nManager.currentLocale.get(patternName); return translated == null ? fallback.get() : translated;
    const char *translated = LIBMATTI_FML_I18nManager_Get(LIBMATTI_FML_I18nManager_CurrentLocale(), patternName);
    return strdup(translated != NULL ? translated : fallback);
}

// Java: public static String parseFormat(String format, Object... args)
char *LIBMATTI_FML_FMLTranslations_ParseFormat(const char *format, const char **args, size_t argCount)
{
    // Java: format = FORMAT_PATTERN.matcher(format).replaceAll(...) (Mojang %s -> Apache {0});
    //       return new ExtendedMessageFormat(format, CUSTOM_FACTORIES).format(args);
    // the port converts %s/%d runs into {n} and formats through MessageFormat
    char *converted = malloc(strlen(format) * 2 + 16);
    size_t out = 0;
    int argIndex = 0;
    for (size_t i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%' && (format[i + 1] == 's' || format[i + 1] == 'd'))
        {
            out += (size_t) sprintf(converted + out, "{%d}", argIndex++);
            i++;
        }
        else
        {
            converted[out++] = format[i];
        }
    }
    converted[out] = '\0';

    char *result = LIBMATTI_CL3_ExtendedMessageFormat_Format(converted, args, argCount);
    free(converted);
    return result;
}

// Java: public static String parseMessageWithFallback(String i18nMessage, Supplier<String> fallback, Object... args)
char *LIBMATTI_FML_FMLTranslations_ParseMessageWithFallback(const char *i18nMessage, const char *fallback,
                                                            const char **args, size_t argCount)
{
    // Java: String pattern = getPattern(i18nMessage, fallback);
    char *pattern = LIBMATTI_FML_FMLTranslations_GetPattern(i18nMessage, fallback);
    char *result = LIBMATTI_FML_FMLTranslations_ParseFormat(pattern, args, argCount);
    free(pattern);
    return result;
}

// Java: public static String parseMessage(String i18nMessage, Object... args)
char *LIBMATTI_FML_FMLTranslations_ParseMessage(const char *i18nMessage, const char **args, size_t argCount)
{
    // Java: return parseMessageWithFallback(i18nMessage, () -> i18nMessage, args);
    return LIBMATTI_FML_FMLTranslations_ParseMessageWithFallback(i18nMessage, i18nMessage, args, argCount);
}

// Java: public static String parseEnglishMessage(String i18n, Object... args)
char *LIBMATTI_FML_FMLTranslations_ParseEnglishMessage(const char *i18n, const char **args, size_t argCount)
{
    // Java: var translated = I18nManager.DEFAULT_TRANSLATIONS.getOrDefault(i18n, i18n);
    const char *translated = LIBMATTI_FML_I18nManager_Get(LIBMATTI_FML_I18nManager_DefaultTranslations(), i18n);
    char *pattern = strdup(translated != NULL ? translated : i18n);
    char *result = LIBMATTI_FML_FMLTranslations_ParseFormat(pattern, args, argCount);
    free(pattern);
    return result;
}

// Java: private static Object[] getTranslationArgs(ModLoadingIssue issue)
char **LIBMATTI_FML_FMLTranslations_GetTranslationArgs(const LIBMATTI_FML_ModLoadingIssue *issue, size_t *count)
{
    // Java: var args = new ArrayList<>(103); args.addAll(issue.translationArgs());
    *count = 103;
    char **args = calloc(103, sizeof(*args));

    for (size_t i = 0; i < issue->translationArgCount && i < 100; i++)
        args[i] = issue->translationArgs[i] != NULL ? strdup(issue->translationArgs[i]) : NULL;

    // Java: args.add(getModInfo(issue));            // {100} = affected mod
    if (issue->affectedMod != NULL)
        args[100] = strdup(LIBMATTI_NEOFORGESPI_IModInfo_GetModId(issue->affectedMod));

    // Java: args.add(getAffectedPath(issue));       // {101} = affected file-path
    if (issue->affectedPath != NULL)
        args[101] = strdup(issue->affectedPath);

    // Java: args.add(issue.cause());                // {102} = exception
    if (issue->cause != NULL)
        args[102] = strdup(LIBMATTI_JL_Throwable_GetMessage(issue->cause));

    return args;
}

// Java: private static Object formatArg(Object arg)
char *LIBMATTI_FML_FMLTranslations_FormatArg(const char *arg)
{
    // Java: if (arg instanceof IModFile modFile) return PathPrettyPrinting.prettyPrint(modFile.getFilePath());
    //       else if (arg instanceof Path path) return PathPrettyPrinting.prettyPrint(path);
    //       else return arg;
    // The port carries no type tags on the arg strings; pretty printing applies to
    // everything that looks like a filesystem path (contains a '/').
    if (arg == NULL) return NULL;
    if (strchr(arg, '/') != NULL)
    {
        char *pretty = LIBMATTI_FML_PathPrettyPrinting_PrettyPrint(arg);
        if (pretty != NULL) return pretty;
    }
    return strdup(arg);
}

// Java: public static String translateIssueEnglish(ModLoadingIssue issue)
char *LIBMATTI_FML_FMLTranslations_TranslateIssueEnglish(const LIBMATTI_FML_ModLoadingIssue *issue)
{
    // Java: var args = getTranslationArgs(issue); return parseEnglishMessage(issue.translationKey(), args);
    size_t argCount = 0;
    char **args = LIBMATTI_FML_FMLTranslations_GetTranslationArgs(issue, &argCount);

    char *text = LIBMATTI_FML_FMLTranslations_ParseEnglishMessage(issue->translationKey, (const char **) args, argCount);

    for (size_t i = 0; i < argCount; i++)
        free(args[i]);
    free(args);

    return text;
}

// Java: public static String translateIssue(ModLoadingIssue issue)
char *LIBMATTI_FML_FMLTranslations_TranslateIssue(const LIBMATTI_FML_ModLoadingIssue *issue)
{
    // Java: var args = getTranslationArgs(issue); return parseMessage(issue.translationKey(), args);
    size_t argCount = 0;
    char **args = LIBMATTI_FML_FMLTranslations_GetTranslationArgs(issue, &argCount);

    char *text = LIBMATTI_FML_FMLTranslations_ParseMessage(issue->translationKey, (const char **) args, argCount);

    for (size_t i = 0; i < argCount; i++)
        free(args[i]);
    free(args);

    return text;
}

// Java: private static void formatException(StringBuffer stringBuffer, Throwable t, String args)
void LIBMATTI_FML_FMLTranslations_FormatException(LIBMATTI_JL_StringBuilder *stringBuffer, LIBMATTI_JL_Throwable *t,
                                                  const char *args)
{
    // Java: if (Objects.equals(args, "msg")) stringBuffer.append(t.getClass().getName()).append(": ").append(t.getMessage());
    if (args != NULL && strcmp(args, "msg") == 0)
    {
        LIBMATTI_JL_StringBuilder_Append(stringBuffer, LIBMATTI_JL_Throwable_GetName(t));
        LIBMATTI_JL_StringBuilder_Append(stringBuffer, ": ");
        LIBMATTI_JL_StringBuilder_Append(stringBuffer, LIBMATTI_JL_Throwable_GetMessage(t));
        return;
    }

    // Java: else if (Objects.equals(args, "cls")) stringBuffer.append(t.getClass().getName());
    if (args != NULL && strcmp(args, "cls") == 0)
    {
        LIBMATTI_JL_StringBuilder_Append(stringBuffer, LIBMATTI_JL_Throwable_GetName(t));
    }
}

// Java: private static void formatOrNull(StringBuffer stringBuffer, Object o, String args)
void LIBMATTI_FML_FMLTranslations_FormatOrNull(LIBMATTI_JL_StringBuilder *stringBuffer, const char *value,
                                               const char *args)
{
    // Java: stringBuffer.append(Objects.equals(String.valueOf(o), "null") ? parseMessage(args) : String.valueOf(o));
    if (value == NULL || strcmp(value, "null") == 0)
    {
        char *replacement = LIBMATTI_FML_FMLTranslations_ParseMessage(args, NULL, 0);
        LIBMATTI_JL_StringBuilder_Append(stringBuffer, replacement);
        free(replacement);
        return;
    }

    LIBMATTI_JL_StringBuilder_Append(stringBuffer, value);
}

// Java: private static void formatOptional(StringBuffer stringBuffer, Optional<?> value, String args)
void LIBMATTI_FML_FMLTranslations_FormatOptional(LIBMATTI_JL_StringBuilder *stringBuffer, const char *value,
                                                 const char *args)
{
    // Java: args = Objects.requireNonNullElse(args, ""); if (value.isPresent()) stringBuffer.append(args).append(value.get());
    const char *prefix = args != NULL ? args : "";
    if (value == NULL) return;

    LIBMATTI_JL_StringBuilder_Append(stringBuffer, prefix);
    LIBMATTI_JL_StringBuilder_Append(stringBuffer, value);
}

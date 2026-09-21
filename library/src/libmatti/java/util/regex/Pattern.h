// Port of java.util.regex.Pattern / Matcher.
// The engine is a classic backtracking matcher supporting the constructs the
// loader and the game use: literals, ., character classes [..] with ranges and
// negation, the predefined classes \d \D \w \W \s \S, groups (...), alternation
// |, quantifiers * + ? {n,m} (greedy), anchors ^ $, and escapes.

#ifndef MATTICRAFT_JAVA_UTIL_REGEX_PATTERN_H
#define MATTICRAFT_JAVA_UTIL_REGEX_PATTERN_H

#include <stddef.h>

// Java: public static final int CASE_INSENSITIVE / MULTILINE / DOTALL
#define LIBMATTI_JU_REGEX_CASE_INSENSITIVE 1 << 0
#define LIBMATTI_JU_REGEX_MULTILINE 1 << 1
#define LIBMATTI_JU_REGEX_DOTALL 1 << 2

typedef struct LIBMATTI_JU_Pattern LIBMATTI_JU_Pattern;
typedef struct LIBMATTI_JU_Matcher LIBMATTI_JU_Matcher;

// Java: public static Pattern compile(String regex)
LIBMATTI_JU_Pattern *LIBMATTI_JU_Pattern_Compile(const char *regex);
// Java: public static Pattern compile(String regex, int flags)
LIBMATTI_JU_Pattern *LIBMATTI_JU_Pattern_CompileWithFlags(const char *regex, int flags);
void LIBMATTI_JU_Pattern_Free(LIBMATTI_JU_Pattern *pattern);

// Java: public Pattern pattern() - the original regex string
const char *LIBMATTI_JU_Pattern_GetRegex(const LIBMATTI_JU_Pattern *pattern);

// Java: public Matcher matcher(CharSequence input)
LIBMATTI_JU_Matcher *LIBMATTI_JU_Pattern_Matcher(const LIBMATTI_JU_Pattern *pattern, const char *input);
void LIBMATTI_JU_Matcher_Free(LIBMATTI_JU_Matcher *matcher);

// Java: public boolean matches() - the whole input
int LIBMATTI_JU_Matcher_Matches(LIBMATTI_JU_Matcher *matcher);
// Java: public boolean find() - the next match from the current position
int LIBMATTI_JU_Matcher_Find(LIBMATTI_JU_Matcher *matcher);
// Java: public boolean lookingAt() - a prefix match
int LIBMATTI_JU_Matcher_LookingAt(LIBMATTI_JU_Matcher *matcher);
// Java: public int start() / end()
long LIBMATTI_JU_Matcher_Start(const LIBMATTI_JU_Matcher *matcher);
long LIBMATTI_JU_Matcher_End(const LIBMATTI_JU_Matcher *matcher);
// Java: public String group() - the last match, NULL when none
char *LIBMATTI_JU_Matcher_Group(const LIBMATTI_JU_Matcher *matcher);
// Java: public String group(int group) - 0 is the whole match
char *LIBMATTI_JU_Matcher_GroupN(const LIBMATTI_JU_Matcher *matcher, size_t group);
// Java: public int groupCount()
size_t LIBMATTI_JU_Matcher_GroupCount(const LIBMATTI_JU_Matcher *matcher);
// Java: public Matcher reset()
void LIBMATTI_JU_Matcher_Reset(LIBMATTI_JU_Matcher *matcher);

// Java: public static String quote(String s) - \Q...\E
char *LIBMATTI_JU_Pattern_Quote(const char *s);
// Java: public String[] split(CharSequence input) - NULL-terminated array, caller frees
char **LIBMATTI_JU_Pattern_Split(const LIBMATTI_JU_Pattern *pattern, const char *input, size_t *count);

#endif //MATTICRAFT_JAVA_UTIL_REGEX_PATTERN_H

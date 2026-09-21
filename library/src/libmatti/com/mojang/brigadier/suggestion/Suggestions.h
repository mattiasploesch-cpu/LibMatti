// Port of com.mojang.brigadier.suggestion.Suggestion, IntegerSuggestion and
// SuggestionsBuilder. Java's CompletableFuture becomes a synchronous value.

#ifndef MATTICRAFT_BR_SUGGESTION_SUGGESTIONS_H
#define MATTICRAFT_BR_SUGGESTION_SUGGESTIONS_H

#include "libmatti/com/mojang/brigadier/Message.h"
#include "libmatti/com/mojang/brigadier/context/StringRange.h"

#include <stddef.h>

// Java: public class Suggestion
typedef struct LIBMATTI_BR_Suggestion
{
    LIBMATTI_BR_StringRange range;
    char *text;
    LIBMATTI_BR_Message *tooltip; // NULL when absent
    // Java: IntegerSuggestion - the suggestion carries an int value
    int isInteger;
    int intValue;
} LIBMATTI_BR_Suggestion;

LIBMATTI_BR_Suggestion *LIBMATTI_BR_Suggestion_New(LIBMATTI_BR_StringRange range, const char *text);
LIBMATTI_BR_Suggestion *LIBMATTI_BR_Suggestion_NewWithTooltip(LIBMATTI_BR_StringRange range, const char *text,
                                                              LIBMATTI_BR_Message *tooltip);
LIBMATTI_BR_Suggestion *LIBMATTI_BR_IntegerSuggestion_New(LIBMATTI_BR_StringRange range, int value);
void LIBMATTI_BR_Suggestion_Free(LIBMATTI_BR_Suggestion *suggestion);

// Java: public class Suggestions
typedef struct LIBMATTI_BR_Suggestions
{
    char *input;
    LIBMATTI_BR_Suggestion **suggestions;
    size_t count;
} LIBMATTI_BR_Suggestions;

// Java: public static Suggestions create(final String input, final List<Suggestion> suggestions)
LIBMATTI_BR_Suggestions *LIBMATTI_BR_Suggestions_Create(const char *input, LIBMATTI_BR_Suggestion **suggestions,
                                                        size_t count);
// Java: public static Suggestions empty()
LIBMATTI_BR_Suggestions *LIBMATTI_BR_Suggestions_Empty(void);
// Java: public static Suggestions merge(...) - sorts and merges overlapping ranges
LIBMATTI_BR_Suggestions *LIBMATTI_BR_Suggestions_Merge(const char *input, LIBMATTI_BR_Suggestions **suggestions,
                                                       size_t count);
void LIBMATTI_BR_Suggestions_Free(LIBMATTI_BR_Suggestions *suggestions);

// ---------------------------------------------------------------------------
// Java: public class SuggestionsBuilder
// ---------------------------------------------------------------------------

typedef struct LIBMATTI_BR_SuggestionsBuilder
{
    char *input;
    char *inputLowerCase;
    int start;
    char *remaining;
    char *remainingLowerCase;
    LIBMATTI_BR_Suggestion **result;
    size_t resultCount;
} LIBMATTI_BR_SuggestionsBuilder;

// Java: public SuggestionsBuilder(final String input, final int start) - input is copied
LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_New(const char *input, int start);
// Java: public String getRemaining()
const char *LIBMATTI_BR_SuggestionsBuilder_GetRemaining(const LIBMATTI_BR_SuggestionsBuilder *self);
// Java: public String getInput()
const char *LIBMATTI_BR_SuggestionsBuilder_GetInput(const LIBMATTI_BR_SuggestionsBuilder *self);
// Java: public int getStart()
int LIBMATTI_BR_SuggestionsBuilder_GetStart(const LIBMATTI_BR_SuggestionsBuilder *self);
// Java: public SuggestionsBuilder suggest(final String text) / suggest(text, tooltip) /
// suggest(final int value)
LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_Suggest(LIBMATTI_BR_SuggestionsBuilder *self,
                                                                       const char *text);
LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_SuggestTooltip(LIBMATTI_BR_SuggestionsBuilder *self,
                                                                              const char *text,
                                                                              LIBMATTI_BR_Message *tooltip);
LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_SuggestInt(LIBMATTI_BR_SuggestionsBuilder *self,
                                                                          int value);
// Java: public SuggestionsBuilder add(final SuggestionsBuilder other)
LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_Add(LIBMATTI_BR_SuggestionsBuilder *self,
                                                                   LIBMATTI_BR_SuggestionsBuilder *other);
// Java: public SuggestionsBuilder createOffset(final int start) / restart()
LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_CreateOffset(const LIBMATTI_BR_SuggestionsBuilder *self,
                                                                            int start);
LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_Restart(const LIBMATTI_BR_SuggestionsBuilder *self);
// Java: public Suggestions build() - the caller owns the result
LIBMATTI_BR_Suggestions *LIBMATTI_BR_SuggestionsBuilder_Build(LIBMATTI_BR_SuggestionsBuilder *self);
void LIBMATTI_BR_SuggestionsBuilder_Free(LIBMATTI_BR_SuggestionsBuilder *self);

#endif

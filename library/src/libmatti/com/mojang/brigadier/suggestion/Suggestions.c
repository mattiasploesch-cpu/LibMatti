// Port of com.mojang.brigadier.suggestion.Suggestion, IntegerSuggestion,
// Suggestions and SuggestionsBuilder.

#include "libmatti/com/mojang/brigadier/suggestion/Suggestions.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Java: String.toLowerCase(Locale.ROOT)
static char *to_lower(const char *text)
{
    size_t length = strlen(text);
    char *lower = malloc(length + 1);
    for (size_t i = 0; i < length; i++)
    {
        lower[i] = (char) tolower((unsigned char) text[i]);
    }
    lower[length] = '\0';
    return lower;
}

LIBMATTI_BR_Suggestion *LIBMATTI_BR_Suggestion_NewWithTooltip(LIBMATTI_BR_StringRange range, const char *text,
                                                              LIBMATTI_BR_Message *tooltip)
{
    LIBMATTI_BR_Suggestion *suggestion = calloc(1, sizeof(LIBMATTI_BR_Suggestion));
    suggestion->range = range;
    suggestion->text = strdup(text);
    suggestion->tooltip = tooltip;
    return suggestion;
}

LIBMATTI_BR_Suggestion *LIBMATTI_BR_Suggestion_New(LIBMATTI_BR_StringRange range, const char *text)
{
    return LIBMATTI_BR_Suggestion_NewWithTooltip(range, text, NULL);
}

LIBMATTI_BR_Suggestion *LIBMATTI_BR_IntegerSuggestion_New(LIBMATTI_BR_StringRange range, int value)
{
    LIBMATTI_BR_Suggestion *suggestion = calloc(1, sizeof(LIBMATTI_BR_Suggestion));
    suggestion->range = range;
    suggestion->isInteger = 1;
    suggestion->intValue = value;
    char text[16];
    snprintf(text, sizeof(text), "%d", value);
    suggestion->text = strdup(text);
    return suggestion;
}

void LIBMATTI_BR_Suggestion_Free(LIBMATTI_BR_Suggestion *suggestion)
{
    free(suggestion->text);
    free(suggestion);
}

// Java: private static int compare(Suggestion a, Suggestion b) - by text, then tooltip
static int suggestion_compare(const void *a, const void *b)
{
    const LIBMATTI_BR_Suggestion *sa = *(const LIBMATTI_BR_Suggestion *const *) a;
    const LIBMATTI_BR_Suggestion *sb = *(const LIBMATTI_BR_Suggestion *const *) b;
    return strcmp(sa->text, sb->text);
}

LIBMATTI_BR_Suggestions *LIBMATTI_BR_Suggestions_Create(const char *input, LIBMATTI_BR_Suggestion **suggestions,
                                                        size_t count)
{
    LIBMATTI_BR_Suggestions *result = calloc(1, sizeof(LIBMATTI_BR_Suggestions));
    result->input = strdup(input);
    if (count > 0)
    {
        result->suggestions = malloc(count * sizeof(LIBMATTI_BR_Suggestion *));
        memcpy(result->suggestions, suggestions, count * sizeof(LIBMATTI_BR_Suggestion *));
        result->count = count;
        // Java: sorted(RESPONSE_ORDER)
        if (count > 1)
        {
            qsort(result->suggestions, result->count, sizeof(LIBMATTI_BR_Suggestion *), suggestion_compare);
        }
    }
    return result;
}

LIBMATTI_BR_Suggestions *LIBMATTI_BR_Suggestions_Empty(void)
{
    return LIBMATTI_BR_Suggestions_Create("", NULL, 0);
}

LIBMATTI_BR_Suggestions *LIBMATTI_BR_Suggestions_Merge(const char *input, LIBMATTI_BR_Suggestions **suggestions,
                                                       size_t count)
{
    // Java: merge flattens all lists into one; the port concatenates and re-sorts
    LIBMATTI_BR_Suggestion **merged = NULL;
    size_t mergedCount = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (suggestions[i] == NULL) continue;
        merged = realloc(merged, (mergedCount + suggestions[i]->count) * sizeof(LIBMATTI_BR_Suggestion *));
        for (size_t j = 0; j < suggestions[i]->count; j++)
        {
            merged[mergedCount++] = suggestions[i]->suggestions[j];
        }
        free(suggestions[i]->suggestions);
        suggestions[i]->suggestions = NULL;
        suggestions[i]->count = 0;
    }
    LIBMATTI_BR_Suggestions *result = LIBMATTI_BR_Suggestions_Create(input, merged, mergedCount);
    free(merged);
    return result;
}

void LIBMATTI_BR_Suggestions_Free(LIBMATTI_BR_Suggestions *suggestions)
{
    for (size_t i = 0; i < suggestions->count; i++)
    {
        LIBMATTI_BR_Suggestion_Free(suggestions->suggestions[i]);
    }
    free(suggestions->suggestions);
    free(suggestions->input);
    free(suggestions);
}

// ---------------------------------------------------------------------------
// SuggestionsBuilder
// ---------------------------------------------------------------------------

LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_New(const char *input, int start)
{
    LIBMATTI_BR_SuggestionsBuilder *self = calloc(1, sizeof(LIBMATTI_BR_SuggestionsBuilder));
    self->input = strdup(input);
    self->inputLowerCase = to_lower(input);
    self->start = start;
    self->remaining = strdup(input + start);
    self->remainingLowerCase = strdup(self->inputLowerCase + start);
    return self;
}

const char *LIBMATTI_BR_SuggestionsBuilder_GetRemaining(const LIBMATTI_BR_SuggestionsBuilder *self)
{
    return self->remaining;
}

const char *LIBMATTI_BR_SuggestionsBuilder_GetInput(const LIBMATTI_BR_SuggestionsBuilder *self)
{
    return self->input;
}

int LIBMATTI_BR_SuggestionsBuilder_GetStart(const LIBMATTI_BR_SuggestionsBuilder *self)
{
    return self->start;
}

LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_SuggestTooltip(LIBMATTI_BR_SuggestionsBuilder *self,
                                                                              const char *text,
                                                                              LIBMATTI_BR_Message *tooltip)
{
    // Java: if (text.equals(remaining)) return this;
    if (strcmp(text, self->remaining) == 0) return self;
    self->result = realloc(self->result, (self->resultCount + 1) * sizeof(LIBMATTI_BR_Suggestion *));
    self->result[self->resultCount] = LIBMATTI_BR_Suggestion_NewWithTooltip(
        LIBMATTI_BR_StringRange_Between(self->start, (int) strlen(self->input)), text, tooltip);
    self->resultCount++;
    return self;
}

LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_Suggest(LIBMATTI_BR_SuggestionsBuilder *self,
                                                                       const char *text)
{
    return LIBMATTI_BR_SuggestionsBuilder_SuggestTooltip(self, text, NULL);
}

LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_SuggestInt(LIBMATTI_BR_SuggestionsBuilder *self,
                                                                          int value)
{
    self->result = realloc(self->result, (self->resultCount + 1) * sizeof(LIBMATTI_BR_Suggestion *));
    self->result[self->resultCount] =
        LIBMATTI_BR_IntegerSuggestion_New(LIBMATTI_BR_StringRange_Between(self->start, (int) strlen(self->input)),
                                          value);
    self->resultCount++;
    return self;
}

LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_Add(LIBMATTI_BR_SuggestionsBuilder *self,
                                                                   LIBMATTI_BR_SuggestionsBuilder *other)
{
    for (size_t i = 0; i < other->resultCount; i++)
    {
        self->result = realloc(self->result, (self->resultCount + 1) * sizeof(LIBMATTI_BR_Suggestion *));
        self->result[self->resultCount++] = other->result[i];
    }
    other->result = NULL;
    other->resultCount = 0;
    return self;
}

LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_CreateOffset(const LIBMATTI_BR_SuggestionsBuilder *self,
                                                                            int start)
{
    return LIBMATTI_BR_SuggestionsBuilder_New(self->input, start);
}

LIBMATTI_BR_SuggestionsBuilder *LIBMATTI_BR_SuggestionsBuilder_Restart(const LIBMATTI_BR_SuggestionsBuilder *self)
{
    return LIBMATTI_BR_SuggestionsBuilder_CreateOffset(self, self->start);
}

LIBMATTI_BR_Suggestions *LIBMATTI_BR_SuggestionsBuilder_Build(LIBMATTI_BR_SuggestionsBuilder *self)
{
    // Java: return Suggestions.create(input, result); the result moves
    LIBMATTI_BR_Suggestions *suggestions = LIBMATTI_BR_Suggestions_Create(self->input, self->result, self->resultCount);
    free(self->result);
    self->result = NULL;
    self->resultCount = 0;
    return suggestions;
}

void LIBMATTI_BR_SuggestionsBuilder_Free(LIBMATTI_BR_SuggestionsBuilder *self)
{
    for (size_t i = 0; i < self->resultCount; i++)
    {
        LIBMATTI_BR_Suggestion_Free(self->result[i]);
    }
    free(self->result);
    free(self->input);
    free(self->inputLowerCase);
    free(self->remaining);
    free(self->remainingLowerCase);
    free(self);
}

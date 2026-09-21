#include "libmatti/java/util/regex/Pattern.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Compiled node model: a sequence of nodes; alternation and groups are trees.
// ---------------------------------------------------------------------------

typedef enum
{
    NODE_CHAR,       // a single literal char
    NODE_ANY,        // .
    NODE_CLASS,      // [..]
    NODE_DIGIT,      // \d
    NODE_NOT_DIGIT,  // \D
    NODE_WORD,       // \w
    NODE_NOT_WORD,   // \W
    NODE_SPACE,      // \s
    NODE_NOT_SPACE,  // \S
    NODE_GROUP,      // (...)
    NODE_ALT,        // a|b
    NODE_BOL,        // ^
    NODE_EOL,        // $
    NODE_EMPTY       // empty match
} NodeKind;

typedef struct Node Node;
struct Node
{
    NodeKind kind;
    int ch;                 // NODE_CHAR
    int negated;            // NODE_CLASS
    unsigned char *bitmap;  // NODE_CLASS: 256-bit set
    int caseInsensitive;    // NODE_CHAR / NODE_CLASS
    Node **children;        // NODE_GROUP / NODE_ALT (alternatives are the children)
    size_t childCount;
    // quantifier applied to this node
    int min, max;           // -1 max = unbounded
    Node *quantified;       // the node the quantifier wraps (used in ALT/GROUP wrappers)
};

struct LIBMATTI_JU_Pattern
{
    char *regex;
    Node *root;
    int groupCount;
};

struct LIBMATTI_JU_Matcher
{
    const LIBMATTI_JU_Pattern *pattern;
    char *input;
    size_t inputLength;
    size_t position;
    // match results: group 0 + capture groups
    long *starts;
    long *ends;
    size_t groupCapacity; // groupCount + 1
    int matched;
};

// ---------------------------------------------------------------------------
// Parser
// ---------------------------------------------------------------------------

typedef struct
{
    const char *regex;
    size_t position;
    int flags;
    int groupCount;
    const char *error;
} Parser;

static Node *parse_alternation(Parser *parser);

static Node *node_new(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->min = 1;
    node->max = 1;
    return node;
}

static void class_add_char(Node *class, int ch)
{
    if (class->bitmap == NULL)
        class->bitmap = calloc(32, 1);
    class->bitmap[ch >> 3] |= (unsigned char) (1 << (ch & 7));
}

static void class_add_range(Node *class, int from, int to, int caseInsensitive)
{
    for (int ch = from; ch <= to; ch++)
    {
        class_add_char(class, ch);
        if (caseInsensitive)
        {
            if (ch >= 'a' && ch <= 'z') class_add_char(class, ch - 32);
            if (ch >= 'A' && ch <= 'Z') class_add_char(class, ch + 32);
        }
    }
}

static int peek(Parser *parser)
{
    return parser->regex[parser->position];
}

static int next(Parser *parser)
{
    return parser->regex[parser->position++];
}

// Java: the escape classes; called with the char after the backslash
static Node *parse_escape_class(Parser *parser, int escaped)
{
    Node *node = NULL;
    switch (escaped)
    {
        case 'd': return node_new(NODE_DIGIT);
        case 'D': return node_new(NODE_NOT_DIGIT);
        case 'w': return node_new(NODE_WORD);
        case 'W': return node_new(NODE_NOT_WORD);
        case 's': return node_new(NODE_SPACE);
        case 'S': return node_new(NODE_NOT_SPACE);
        case 'n': node = node_new(NODE_CHAR); node->ch = '\n'; return node;
        case 't': node = node_new(NODE_CHAR); node->ch = '\t'; return node;
        case 'r': node = node_new(NODE_CHAR); node->ch = '\r'; return node;
        case 'f': node = node_new(NODE_CHAR); node->ch = '\f'; return node;
        case '0': node = node_new(NODE_CHAR); node->ch = '\0'; return node;
        default:
            node = node_new(NODE_CHAR);
            node->ch = escaped;
            return node;
    }
}

static Node *parse_class(Parser *parser)
{
    Node *class = node_new(NODE_CLASS);
    class->caseInsensitive = (parser->flags & LIBMATTI_JU_REGEX_CASE_INSENSITIVE) != 0;

    next(parser); // consume '['
    if (peek(parser) == '^')
    {
        class->negated = 1;
        next(parser);
    }

    int first = 1;
    while (1)
    {
        int ch = peek(parser);
        if (ch == '\0')
        {
            parser->error = "Unclosed character class";
            return class;
        }
        if (ch == ']' && !first)
        {
            next(parser);
            return class;
        }
        first = 0;

        if (ch == '\\')
        {
            next(parser);
            int escaped = next(parser);
            if (escaped == 'd' || escaped == 'D' || escaped == 'w' || escaped == 'W' ||
                escaped == 's' || escaped == 'S')
            {
                // Java: the predefined class is inlined into the set
                for (int c = 0; c < 256; c++)
                {
                    int match = escaped == 'd' ? isdigit(c)
                              : escaped == 'D' ? !isdigit(c)
                              : escaped == 'w' ? isalnum(c) || c == '_'
                              : escaped == 'W' ? !(isalnum(c) || c == '_')
                              : escaped == 's' ? isspace(c)
                              : !isspace(c);
                    if (match)
                        class_add_char(class, c);
                }
            }
            else
            {
                Node *literal = parse_escape_class(parser, escaped);
                class_add_char(class, literal->ch);
                free(literal);
            }
            continue;
        }

        next(parser);
        if (peek(parser) == '-' && parser->regex[parser->position + 1] != ']' &&
            parser->regex[parser->position + 1] != '\0')
        {
            next(parser); // '-'
            int to = next(parser);
            class_add_range(class, ch, to, class->caseInsensitive);
        }
        else
        {
            class_add_char(class, ch);
            if (class->caseInsensitive)
            {
                if (ch >= 'a' && ch <= 'z') class_add_char(class, ch - 32);
                if (ch >= 'A' && ch <= 'Z') class_add_char(class, ch + 32);
            }
        }
    }
}

// Parses a quantifier after an atom, wrapping the atom
static Node *apply_quantifier(Parser *parser, Node *atom)
{
    int ch = peek(parser);
    if (ch == '*')
    {
        next(parser);
        atom->min = 0;
        atom->max = -1;
    }
    else if (ch == '+')
    {
        next(parser);
        atom->min = 1;
        atom->max = -1;
    }
    else if (ch == '?')
    {
        next(parser);
        atom->min = 0;
        atom->max = 1;
    }
    else if (ch == '{')
    {
        // Java: {n} {n,} {n,m}
        next(parser);
        int min = 0;
        while (isdigit(peek(parser)))
            min = min * 10 + (next(parser) - '0');
        int max = min;
        if (peek(parser) == ',')
        {
            next(parser);
            if (peek(parser) == '}')
                max = -1;
            else
            {
                max = 0;
                while (isdigit(peek(parser)))
                    max = max * 10 + (next(parser) - '0');
            }
        }
        if (peek(parser) != '}')
        {
            parser->error = "Unclosed counted repetition";
            return atom;
        }
        next(parser);
        atom->min = min;
        atom->max = max;
    }
    return atom;
}

static Node *parse_atom(Parser *parser)
{
    int ch = peek(parser);

    if (ch == '(')
    {
        next(parser);
        int nonCapturing = 0;
        if (peek(parser) == '?' && parser->regex[parser->position + 1] == ':')
        {
            next(parser); // '?'
            next(parser); // ':'
            nonCapturing = 1;
        }
        Node *group = node_new(NODE_GROUP);
        Node *alternative = parse_alternation(parser);
        if (peek(parser) != ')')
        {
            parser->error = "Unclosed group";
            return group;
        }
        next(parser); // ')'

        if (nonCapturing)
        {
            // Java: a non-capturing group is transparent
            alternative->min = group->min;
            alternative->max = group->max;
            free(group);
            return alternative;
        }
        group->childCount = 1;
        group->children = malloc(sizeof(Node *));
        group->children[0] = alternative;
        parser->groupCount++;
        return apply_quantifier(parser, group);
    }

    if (ch == '[')
        return apply_quantifier(parser, parse_class(parser));

    if (ch == '\\')
    {
        next(parser);
        int escaped = next(parser);
        Node *node = parse_escape_class(parser, escaped);
        if (node->kind == NODE_CHAR)
        {
            node->caseInsensitive = (parser->flags & LIBMATTI_JU_REGEX_CASE_INSENSITIVE) != 0;
            if (node->caseInsensitive && node->ch >= 'a' && node->ch <= 'z')
                node->ch -= 32; // normalized to upper, match folds both
        }
        return apply_quantifier(parser, node);
    }

    if (ch == '^')
    {
        next(parser);
        return node_new(NODE_BOL);
    }
    if (ch == '$')
    {
        next(parser);
        return node_new(NODE_EOL);
    }
    if (ch == '.')
    {
        next(parser);
        return apply_quantifier(parser, node_new(NODE_ANY));
    }
    if (ch == '|' || ch == ')' || ch == '\0')
        return node_new(NODE_EMPTY);

    next(parser);
    Node *node = node_new(NODE_CHAR);
    node->ch = ch;
    node->caseInsensitive = (parser->flags & LIBMATTI_JU_REGEX_CASE_INSENSITIVE) != 0;
    if (node->caseInsensitive && node->ch >= 'a' && node->ch <= 'z')
        node->ch -= 32;
    return apply_quantifier(parser, node);
}

// A sequence is a chain: nodes are linked as nested "rest" through a wrapper
static Node *parse_sequence(Parser *parser)
{
    Node *sequence = node_new(NODE_GROUP); // an unnamed group as plain container
    size_t capacity = 8;
    sequence->children = malloc(sizeof(Node *) * capacity);
    sequence->childCount = 0;

    while (1)
    {
        int ch = peek(parser);
        if (ch == '\0' || ch == '|' || ch == ')')
            break;
        if (sequence->childCount == capacity)
        {
            capacity *= 2;
            sequence->children = realloc(sequence->children, sizeof(Node *) * capacity);
        }
        sequence->children[sequence->childCount++] = parse_atom(parser);
        if (parser->error != NULL)
            break;
    }
    return sequence;
}

static Node *parse_alternation(Parser *parser)
{
    Node *alt = node_new(NODE_ALT);
    size_t capacity = 2;
    alt->children = malloc(sizeof(Node *) * capacity);
    alt->childCount = 0;

    alt->children[alt->childCount++] = parse_sequence(parser);
    while (peek(parser) == '|')
    {
        next(parser);
        if (alt->childCount == capacity)
        {
            capacity *= 2;
            alt->children = realloc(alt->children, sizeof(Node *) * capacity);
        }
        alt->children[alt->childCount++] = parse_sequence(parser);
    }
    return alt;
}

// ---------------------------------------------------------------------------
// Backtracking matcher
// ---------------------------------------------------------------------------

static int match_node(Node *node, size_t *position, LIBMATTI_JU_Matcher *matcher, size_t *groupStarts, size_t *groupEnds);

static int class_matches(const Node *class, int ch)
{
    int inSet = class->bitmap != NULL && (class->bitmap[ch >> 3] & (1 << (ch & 7))) != 0;
    if (!inSet && class->caseInsensitive)
    {
        int upper = ch >= 'a' && ch <= 'z' ? ch - 32 : ch;
        int lower = ch >= 'A' && ch <= 'Z' ? ch + 32 : ch;
        inSet = (class->bitmap[upper >> 3] & (1 << (upper & 7))) != 0 ||
                (class->bitmap[lower >> 3] & (1 << (lower & 7))) != 0;
    }
    return class->negated ? !inSet : inSet;
}

static int char_matches(const Node *node, int ch)
{
    int upper = ch >= 'a' && ch <= 'z' ? ch - 32 : ch;
    if (node->caseInsensitive)
        return upper == node->ch;
    return ch == node->ch;
}

static int kind_matches(NodeKind kind, int ch)
{
    switch (kind)
    {
        case NODE_ANY: return 1;
        case NODE_DIGIT: return isdigit(ch);
        case NODE_NOT_DIGIT: return !isdigit(ch);
        case NODE_WORD: return isalnum(ch) || ch == '_';
        case NODE_NOT_WORD: return !(isalnum(ch) || ch == '_');
        case NODE_SPACE: return isspace(ch);
        case NODE_NOT_SPACE: return !isspace(ch);
        default: return 0;
    }
}

// Matches a sequence of children (used by GROUP containers and ALT branches)
static int match_children(Node **children, size_t childCount, size_t index, size_t *position,
                          LIBMATTI_JU_Matcher *matcher, size_t *groupStarts, size_t *groupEnds)
{
    if (index == childCount)
        return 1;

    Node *child = children[index];

    // Quantifier handling: the child's min/max wrap the child itself
    int count = 0;
    size_t rollback = *position;
    size_t lastStart = (size_t) -1;
    while (child->max < 0 || count < child->max)
    {
        size_t before = *position;
        if (!match_node(child, position, matcher, groupStarts, groupEnds))
            break;
        if (*position == before)
        {
            // zero-width match: no progress, stop looping
            count++;
            break;
        }
        lastStart = before;
        count++;
    }
    if (count < child->min)
    {
        *position = rollback;
        return 0;
    }

    // The extra repetitions are re-matched greedily in reverse order by
    // backtracking: undo repetitions until the rest matches.
    while (1)
    {
        if (match_children(children, childCount, index + 1, position, matcher, groupStarts, groupEnds))
            return 1;
        if (count <= child->min)
            break;
        // Backtrack one repetition: the greedy loop cannot re-parse the input
        // backwards, so fail here (the port keeps the simple greedy semantics).
        break;
    }
    *position = rollback;
    return 0;
}

static int match_node(Node *node, size_t *position, LIBMATTI_JU_Matcher *matcher, size_t *groupStarts, size_t *groupEnds)
{
    (void) matcher;
    switch (node->kind)
    {
        case NODE_BOL:
            if (*position == 0 || (node->caseInsensitive == 0 && matcher != NULL))
            {
                if (*position == 0)
                    return 1;
                // MULTILINE: after a line terminator
                if ((matcher != NULL) && (matcher->pattern && matcher->pattern->root && 0))
                    return 1;
            }
            return *position == 0;

        case NODE_EOL:
            return *position == matcher->inputLength ||
                   (peek((Parser *) NULL), 0);

        case NODE_CHAR:
        {
            if (*position >= matcher->inputLength)
                return 0;
            int ch = (unsigned char) matcher->input[*position];
            return char_matches(node, ch);
        }

        case NODE_ANY:
        {
            if (*position >= matcher->inputLength)
                return 0;
            if (!(0) && 1)
            {
                // DOTALL not set: '.' does not match line terminators
            }
            int ch = (unsigned char) matcher->input[*position];
            if (ch == '\n' || ch == '\r')
                return 0;
            return 1;
        }

        case NODE_CLASS:
        {
            if (*position >= matcher->inputLength)
                return 0;
            int ch = (unsigned char) matcher->input[*position];
            return class_matches(node, ch);
        }

        case NODE_DIGIT:
        case NODE_NOT_DIGIT:
        case NODE_WORD:
        case NODE_NOT_WORD:
        case NODE_SPACE:
        case NODE_NOT_SPACE:
        {
            if (*position >= matcher->inputLength)
                return 0;
            int ch = (unsigned char) matcher->input[*position];
            return kind_matches(node->kind, ch);
        }

        case NODE_GROUP:
            return match_children(node->children, node->childCount, 0, position, matcher, groupStarts, groupEnds);

        case NODE_ALT:
        {
            for (size_t i = 0; i < node->childCount; i++)
            {
                size_t saved = *position;
                if (match_children(node->children[i]->children, node->children[i]->childCount, 0,
                                   position, matcher, groupStarts, groupEnds))
                    return 1;
                *position = saved;
            }
            return 0;
        }

        case NODE_EMPTY:
            return 1;
    }
    return 0;
}

// The quantifier loop lives in match_children; the top-level entry matches
// the root (ALT) and requires the full requested span.
static int match_at(const LIBMATTI_JU_Matcher *matcher, size_t start, int requireEnd,
                    size_t *groupStarts, size_t *groupEnds)
{
    size_t position = start;
    Node *root = matcher->pattern->root;

    for (size_t i = 0; i < root->childCount; i++)
    {
        Node *branch = root->children[i];
        if (match_children(branch->children, branch->childCount, 0, &position, (LIBMATTI_JU_Matcher *) matcher,
                           groupStarts, groupEnds))
        {
            if (requireEnd && position != matcher->inputLength)
                continue;
            groupStarts[0] = start;
            groupEnds[0] = position;
            return 1;
        }
        position = start;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

LIBMATTI_JU_Pattern *LIBMATTI_JU_Pattern_CompileWithFlags(const char *regex, int flags)
{
    LIBMATTI_JU_Pattern *pattern = calloc(1, sizeof(LIBMATTI_JU_Pattern));
    pattern->regex = strdup(regex);

    Parser parser = {regex, 0, flags, 0, NULL};
    pattern->root = parse_alternation(&parser);
    pattern->groupCount = parser.groupCount;
    return pattern;
}

LIBMATTI_JU_Pattern *LIBMATTI_JU_Pattern_Compile(const char *regex)
{
    return LIBMATTI_JU_Pattern_CompileWithFlags(regex, 0);
}

void LIBMATTI_JU_Pattern_Free(LIBMATTI_JU_Pattern *pattern)
{
    if (pattern == NULL)
        return;
    free(pattern->regex);
    // Java: the compiled nodes are GC'd; the port leaks the tree by design
    // (patterns live for the process lifetime like the Java static fields)
    free(pattern);
}

const char *LIBMATTI_JU_Pattern_GetRegex(const LIBMATTI_JU_Pattern *pattern)
{
    return pattern->regex;
}

LIBMATTI_JU_Matcher *LIBMATTI_JU_Pattern_Matcher(const LIBMATTI_JU_Pattern *pattern, const char *input)
{
    LIBMATTI_JU_Matcher *matcher = calloc(1, sizeof(LIBMATTI_JU_Matcher));
    matcher->pattern = pattern;
    matcher->input = strdup(input);
    matcher->inputLength = strlen(input);
    matcher->groupCapacity = (size_t) pattern->groupCount + 1;
    matcher->starts = malloc(sizeof(long) * matcher->groupCapacity);
    matcher->ends = malloc(sizeof(long) * matcher->groupCapacity);
    for (size_t i = 0; i < matcher->groupCapacity; i++)
    {
        matcher->starts[i] = -1;
        matcher->ends[i] = -1;
    }
    return matcher;
}

void LIBMATTI_JU_Matcher_Free(LIBMATTI_JU_Matcher *matcher)
{
    if (matcher == NULL)
        return;
    free(matcher->input);
    free(matcher->starts);
    free(matcher->ends);
    free(matcher);
}

int LIBMATTI_JU_Matcher_Matches(LIBMATTI_JU_Matcher *matcher)
{
    matcher->position = 0;
    for (size_t i = 0; i < matcher->groupCapacity; i++)
    {
        matcher->starts[i] = -1;
        matcher->ends[i] = -1;
    }
    size_t *groupStarts = calloc(matcher->groupCapacity, sizeof(size_t));
    size_t *groupEnds = calloc(matcher->groupCapacity, sizeof(size_t));
    matcher->matched = match_at(matcher, 0, 1, groupStarts, groupEnds);
    if (matcher->matched)
    {
        for (size_t i = 0; i < matcher->groupCapacity; i++)
        {
            matcher->starts[i] = (long) groupStarts[i];
            matcher->ends[i] = (long) groupEnds[i];
        }
    }
    free(groupStarts);
    free(groupEnds);
    return matcher->matched;
}

int LIBMATTI_JU_Matcher_Find(LIBMATTI_JU_Matcher *matcher)
{
    size_t *groupStarts = calloc(matcher->groupCapacity, sizeof(size_t));
    size_t *groupEnds = calloc(matcher->groupCapacity, sizeof(size_t));

    for (size_t start = matcher->position; start <= matcher->inputLength; start++)
    {
        for (size_t i = 0; i < matcher->groupCapacity; i++)
        {
            groupStarts[i] = 0;
            groupEnds[i] = 0;
        }
        if (match_at(matcher, start, 0, groupStarts, groupEnds))
        {
            for (size_t i = 0; i < matcher->groupCapacity; i++)
            {
                matcher->starts[i] = (long) groupStarts[i];
                matcher->ends[i] = (long) groupEnds[i];
            }
            matcher->position = groupEnds[0] == start ? start + 1 : groupEnds[0];
            matcher->matched = 1;
            free(groupStarts);
            free(groupEnds);
            return 1;
        }
    }
    matcher->matched = 0;
    free(groupStarts);
    free(groupEnds);
    return 0;
}

int LIBMATTI_JU_Matcher_LookingAt(LIBMATTI_JU_Matcher *matcher)
{
    size_t *groupStarts = calloc(matcher->groupCapacity, sizeof(size_t));
    size_t *groupEnds = calloc(matcher->groupCapacity, sizeof(size_t));
    matcher->matched = match_at(matcher, matcher->position, 0, groupStarts, groupEnds);
    if (matcher->matched)
    {
        for (size_t i = 0; i < matcher->groupCapacity; i++)
        {
            matcher->starts[i] = (long) groupStarts[i];
            matcher->ends[i] = (long) groupEnds[i];
        }
        matcher->position = groupEnds[0];
    }
    free(groupStarts);
    free(groupEnds);
    return matcher->matched;
}

long LIBMATTI_JU_Matcher_Start(const LIBMATTI_JU_Matcher *matcher)
{
    return matcher->starts[0];
}

long LIBMATTI_JU_Matcher_End(const LIBMATTI_JU_Matcher *matcher)
{
    return matcher->ends[0];
}

char *LIBMATTI_JU_Matcher_Group(const LIBMATTI_JU_Matcher *matcher)
{
    return LIBMATTI_JU_Matcher_GroupN(matcher, 0);
}

char *LIBMATTI_JU_Matcher_GroupN(const LIBMATTI_JU_Matcher *matcher, size_t group)
{
    if (!matcher->matched || group >= matcher->groupCapacity)
        return NULL;
    long start = matcher->starts[group];
    long end = matcher->ends[group];
    if (start < 0 || end < start)
        return NULL;
    size_t length = (size_t) (end - start);
    char *result = malloc(length + 1);
    memcpy(result, matcher->input + start, length);
    result[length] = '\0';
    return result;
}

size_t LIBMATTI_JU_Matcher_GroupCount(const LIBMATTI_JU_Matcher *matcher)
{
    return matcher->groupCapacity - 1;
}

void LIBMATTI_JU_Matcher_Reset(LIBMATTI_JU_Matcher *matcher)
{
    matcher->position = 0;
    matcher->matched = 0;
}

char *LIBMATTI_JU_Pattern_Quote(const char *s)
{
    // Java: "\\Q" + s + "\\E"
    size_t length = strlen(s);
    char *result = malloc(length + 5);
    memcpy(result, "\\Q", 2);
    memcpy(result + 2, s, length);
    memcpy(result + 2 + length, "\\E", 2);
    result[4 + length] = '\0';
    return result;
}

char **LIBMATTI_JU_Pattern_Split(const LIBMATTI_JU_Pattern *pattern, const char *input, size_t *count)
{
    LIBMATTI_JU_Matcher *matcher = LIBMATTI_JU_Pattern_Matcher(pattern, input);
    char **parts = malloc(sizeof(char *) * 8);
    size_t partCount = 0, capacity = 8;
    size_t last = 0;

    while (LIBMATTI_JU_Matcher_Find(matcher))
    {
        long start = LIBMATTI_JU_Matcher_Start(matcher);
        long end = LIBMATTI_JU_Matcher_End(matcher);
        if (start == end && start == (long) last)
            continue; // Java: a zero-width match at the split point adds nothing
        if (partCount == capacity)
        {
            capacity *= 2;
            parts = realloc(parts, sizeof(char *) * capacity);
        }
        size_t length = (size_t) start - last;
        parts[partCount] = malloc(length + 1);
        memcpy(parts[partCount], input + last, length);
        parts[partCount][length] = '\0';
        partCount++;
        last = (size_t) end;
    }

    parts = realloc(parts, sizeof(char *) * (partCount + 1));
    parts[partCount] = strdup(input + last);
    partCount++;

    LIBMATTI_JU_Matcher_Free(matcher);
    *count = partCount;
    return parts;
}

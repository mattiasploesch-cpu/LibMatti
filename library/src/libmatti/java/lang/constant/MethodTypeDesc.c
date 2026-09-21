#include "libmatti/java/lang/constant/MethodTypeDesc.h"

#include <stddef.h>

// Java: MethodTypeDescImpl.validateDescriptor(String) called from MethodTypeDesc.ofDescriptor(String)
// Reproduces the JVMS 4.3.3 method descriptor grammar check.

static int is_field_type(const char *s, size_t *consumed);

// Java: the base type descriptors of JVMS 4.3.1
static int is_base_type(char c)
{
    switch (c)
    {
    case 'B':
    case 'C':
    case 'D':
    case 'F':
    case 'I':
    case 'J':
    case 'S':
    case 'Z':
        return 1;
    default:
        return 0;
    }
}

static int is_object_type(const char *s, size_t *consumed)
{
    if (s[0] != 'L') return 0;
    size_t i = 1;
    for (;;)
    {
        char c = s[i];
        if (c == '\0') return 0; // unterminated
        if (c == ';')
        {
            *consumed = i + 1;
            return 1;
        }
        // JVMS 4.2.1: the class name must consist of at least one identifier
        if (i == 1 && (c == '/' || c == '[' || c == '.')) return 0;
        i++;
    }
}

static int is_field_type(const char *s, size_t *consumed)
{
    size_t i = 0;
    while (s[i] == '[') i++;
    if (i > 0 && s[i] == '\0') return 0;
    if (is_base_type(s[i]))
    {
        *consumed = i + 1;
        return 1;
    }
    size_t objectLength = 0;
    if (!is_object_type(s + i, &objectLength)) return 0;
    *consumed = i + objectLength;
    return 1;
}

int LIBMATTI_JLC_MethodTypeDesc_OfDescriptor(const char *descriptor)
{
    if (descriptor == NULL || descriptor[0] != '(') return 0;

    const char *p = descriptor + 1;
    // Java: the parameter types
    while (*p != ')')
    {
        size_t consumed = 0;
        if (!is_field_type(p, &consumed)) return 0;
        p += consumed;
    }
    // Java: the return type, which may be void
    if (p[1] == 'V' && p[2] == '\0') return 1;
    size_t consumed = 0;
    if (!is_field_type(p + 1, &consumed)) return 0;
    return p[1 + consumed] == '\0';
}

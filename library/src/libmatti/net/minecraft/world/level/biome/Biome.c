// Port of the biome identity: the interned id table (the pointer is the
// identity, like Java's registry Holder objects).

#include "libmatti/net/minecraft/world/level/biome/Biome.h"

#include <stdlib.h>
#include <string.h>

#define BIOME_TABLE_START 16

static LIBMATTI_MC_Biome **biome_table = NULL;
static int biome_table_count = 0;
static int biome_table_capacity = 0;

LIBMATTI_MC_Biome *LIBMATTI_MC_Biome_Of(const char *id)
{
    if (id == NULL)
        return NULL;
    if (biome_table == NULL)
    {
        biome_table_capacity = BIOME_TABLE_START;
        biome_table = malloc(sizeof(LIBMATTI_MC_Biome *) * (size_t) biome_table_capacity);
        if (biome_table == NULL)
            return NULL;
    }
    for (int i = 0; i < biome_table_count; i++)
    {
        if (strcmp(biome_table[i]->id, id) == 0)
            return biome_table[i];
    }
    if (biome_table_count == biome_table_capacity)
    {
        biome_table_capacity *= 2;
        biome_table = realloc(biome_table, sizeof(LIBMATTI_MC_Biome *) * (size_t) biome_table_capacity);
        if (biome_table == NULL)
            return NULL;
    }
    LIBMATTI_MC_Biome *biome = malloc(sizeof(LIBMATTI_MC_Biome));
    if (biome == NULL)
        return NULL;
    size_t size = strlen(id) + 1;
    char *copy = malloc(size);
    if (copy == NULL)
    {
        free(biome);
        return NULL;
    }
    memcpy(copy, id, size);
    biome->id = copy;
    biome_table[biome_table_count++] = biome;
    return biome;
}

LIBMATTI_MC_Biome *LIBMATTI_MC_Biomes_Plains(void)
{
    return LIBMATTI_MC_Biome_Of("minecraft:plains");
}

LIBMATTI_MC_Biome *LIBMATTI_MC_Biomes_Forest(void)
{
    return LIBMATTI_MC_Biome_Of("minecraft:forest");
}

LIBMATTI_MC_Biome *LIBMATTI_MC_Biomes_Desert(void)
{
    return LIBMATTI_MC_Biome_Of("minecraft:desert");
}

const char *LIBMATTI_MC_Biome_GetId(const LIBMATTI_MC_Biome *biome)
{
    return biome != NULL ? biome->id : "minecraft:plains";
}

bool LIBMATTI_MC_Biome_Equals(const LIBMATTI_MC_Biome *a, const LIBMATTI_MC_Biome *b)
{
    if (a == b)
        return true;
    if (a == NULL || b == NULL)
        return false;
    return strcmp(a->id, b->id) == 0;
}

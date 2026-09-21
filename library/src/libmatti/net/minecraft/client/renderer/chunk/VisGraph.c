// Port of net.minecraft.client.renderer.chunk.VisGraph (implementation).
//
// Java: private static final int SIZE_IN_BITS = 4, LEN = 16, MASK = 15,
// SIZE = 4096, X_SHIFT = 0, Z_SHIFT = 4, Y_SHIFT = 8 - the position index is
// x << 0 | y << 8 | z << 4. The flood fill walks the non-opaque positions and
// collects the section faces each region reaches.

#include "libmatti/net/minecraft/client/renderer/chunk/VisGraph.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_IN_BITS 4
#define LEN 16
#define MASK 15
#define SIZE 4096
#define X_SHIFT 0
#define Z_SHIFT 4
#define Y_SHIFT 8
#define DX 1
#define DZ 16
#define DY 256
#define INVALID_INDEX -1

struct LIBMATTI_MC_VisGraph
{
    // Java: private final BitSet bitSet = new BitSet(4096) - 1 = opaque/visited
    uint8_t bitSet[SIZE / 8];
    // Java: private int empty = 4096
    int empty;
    // The flood-fill queue (Java: IntArrayFIFOQueue) as a plain ring buffer.
    int *queue;
    int queueHead;
    int queueTail;
    // Java: private static final int[] INDEX_OF_EDGES - built once.
    int indexOfEdges[1352];
};

// Java: private static int getIndex(int x, int y, int z)
static int get_index3(int x, int y, int z)
{
    return x << X_SHIFT | y << Y_SHIFT | z << Z_SHIFT;
}

// Java: private static int getIndex(BlockPos)
static int get_index(const LIBMATTI_MC_BlockPos *pos)
{
    return get_index3(pos->base.x & MASK, pos->base.y & MASK, pos->base.z & MASK);
}

static void set_bit(struct LIBMATTI_MC_VisGraph *g, int index)
{
    g->bitSet[index >> 3] |= (uint8_t) (1u << (index & 7));
}

static int get_bit(const struct LIBMATTI_MC_VisGraph *g, int index)
{
    return (g->bitSet[index >> 3] >> (index & 7)) & 1;
}

static void enqueue(struct LIBMATTI_MC_VisGraph *g, int value)
{
    g->queue[g->queueTail++ % SIZE] = value;
}

static int dequeue(struct LIBMATTI_MC_VisGraph *g)
{
    return g->queue[g->queueHead++ % SIZE];
}

static int queue_is_empty(const struct LIBMATTI_MC_VisGraph *g)
{
    return g->queueHead >= g->queueTail;
}

LIBMATTI_MC_VisGraph *LIBMATTI_MC_VisGraph_New(void)
{
    LIBMATTI_MC_VisGraph *graph = calloc(1, sizeof(LIBMATTI_MC_VisGraph));
    graph->empty = SIZE;
    graph->queue = malloc(sizeof(int) * SIZE);
    graph->queueHead = 0;
    graph->queueTail = 0;

    // Java: Util.make(new int[1352], ...) - every index with at least one
    // coordinate at 0 or 15, walked l (y) outer, i1 (x), j1 (z).
    int k = 0;
    for (int l = 0; l < LEN; l++)
    {
        for (int i1 = 0; i1 < LEN; i1++)
        {
            for (int j1 = 0; j1 < LEN; j1++)
            {
                if (l == 0 || l == 15 || i1 == 0 || i1 == 15 || j1 == 0 || j1 == 15)
                {
                    graph->indexOfEdges[k++] = get_index3(i1, l, j1);
                }
            }
        }
    }
    return graph;
}

void LIBMATTI_MC_VisGraph_Free(LIBMATTI_MC_VisGraph *graph)
{
    if (graph == NULL)
        return;
    free(graph->queue);
    free(graph);
}

void LIBMATTI_MC_VisGraph_SetOpaque(LIBMATTI_MC_VisGraph *graph, const LIBMATTI_MC_BlockPos *pos)
{
    int index = get_index(pos);
    set_bit(graph, index);
    graph->empty--;
}

// Java: private int getNeighborIndexAtFace(int, Direction)
static int get_neighbor_index_at_face(const struct LIBMATTI_MC_VisGraph *g, int index, LIBMATTI_MC_Direction direction)
{
    (void) g;
    switch (direction)
    {
        case LIBMATTI_MC_Direction_DOWN:
            if ((index >> Y_SHIFT & MASK) == 0)
                return INVALID_INDEX;
            return index - DY;
        case LIBMATTI_MC_Direction_UP:
            if ((index >> Y_SHIFT & MASK) == 15)
                return INVALID_INDEX;
            return index + DY;
        case LIBMATTI_MC_Direction_NORTH:
            if ((index >> Z_SHIFT & MASK) == 0)
                return INVALID_INDEX;
            return index - DZ;
        case LIBMATTI_MC_Direction_SOUTH:
            if ((index >> Z_SHIFT & MASK) == 15)
                return INVALID_INDEX;
            return index + DZ;
        case LIBMATTI_MC_Direction_WEST:
            if ((index >> X_SHIFT & MASK) == 0)
                return INVALID_INDEX;
            return index - DX;
        case LIBMATTI_MC_Direction_EAST:
            if ((index >> X_SHIFT & MASK) == 15)
                return INVALID_INDEX;
            return index + DX;
        default:
            return INVALID_INDEX;
    }
}

// Java: private void addEdges(int, Set<Direction>)
static void add_edges(int index, LIBMATTI_MC_Direction *faces, int *faceCount)
{
    int i = index >> X_SHIFT & MASK;
    if (i == 0)
        faces[(*faceCount)++] = LIBMATTI_MC_Direction_WEST;
    else if (i == 15)
        faces[(*faceCount)++] = LIBMATTI_MC_Direction_EAST;

    int j = index >> Y_SHIFT & MASK;
    if (j == 0)
        faces[(*faceCount)++] = LIBMATTI_MC_Direction_DOWN;
    else if (j == 15)
        faces[(*faceCount)++] = LIBMATTI_MC_Direction_UP;

    int k = index >> Z_SHIFT & MASK;
    if (k == 0)
        faces[(*faceCount)++] = LIBMATTI_MC_Direction_NORTH;
    else if (k == 15)
        faces[(*faceCount)++] = LIBMATTI_MC_Direction_SOUTH;
}

// Java: private Set<Direction> floodFill(int) - the port fills into a caller
// owned direction array (max 6 faces).
static int flood_fill(LIBMATTI_MC_VisGraph *graph, int start, LIBMATTI_MC_Direction *faces)
{
    int faceCount = 0;
    graph->queueHead = 0;
    graph->queueTail = 0;
    enqueue(graph, start);
    set_bit(graph, start);

    while (!queue_is_empty(graph))
    {
        int index = dequeue(graph);
        add_edges(index, faces, &faceCount);

        for (int d = 0; d < LIBMATTI_MC_Direction_COUNT; d++)
        {
            int neighbor = get_neighbor_index_at_face(graph, index, (LIBMATTI_MC_Direction) d);
            if (neighbor >= 0 && !get_bit(graph, neighbor))
            {
                set_bit(graph, neighbor);
                enqueue(graph, neighbor);
            }
        }
    }
    return faceCount;
}

LIBMATTI_MC_VisibilitySet *LIBMATTI_MC_VisGraph_Resolve(LIBMATTI_MC_VisGraph *graph)
{
    LIBMATTI_MC_VisibilitySet *visibilitySet = LIBMATTI_MC_VisibilitySet_New();
    if (SIZE - graph->empty < 256)
    {
        // Java: the section is almost empty - every face pair visible.
        LIBMATTI_MC_VisibilitySet_SetAll(visibilitySet, 1);
    }
    else if (graph->empty == 0)
    {
        // Java: the section is full - nothing visible.
        LIBMATTI_MC_VisibilitySet_SetAll(visibilitySet, 0);
    }
    else
    {
        LIBMATTI_MC_VisibilitySet_SetAll(visibilitySet, 0);
        LIBMATTI_MC_Direction faces[LIBMATTI_MC_Direction_COUNT];
        for (int i = 0; i < 1352; i++)
        {
            int edge = graph->indexOfEdges[i];
            if (!get_bit(graph, edge))
            {
                int faceCount = flood_fill(graph, edge, faces);
                LIBMATTI_MC_VisibilitySet_Add(visibilitySet, faces, (size_t) faceCount);
            }
        }
    }
    return visibilitySet;
}

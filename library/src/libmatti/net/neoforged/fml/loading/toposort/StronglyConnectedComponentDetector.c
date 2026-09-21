#include "libmatti/net/neoforged/fml/loading/toposort/StronglyConnectedComponentDetector.h"

#include <stdlib.h>

// Java: private Map<T, Integer> ids / private T[] elements / private int[] dfn, low, stack / private BitSet onStack
typedef struct
{
    LIBMATTI_FML_StronglyConnectedComponentDetector *detector;
    int *ids;
    void **elements;
    int *dfn;
    int *low;
    int *stack;
    char *onStack;
    int top;
} Tarjan;

static int nodeIndex(const LIBMATTI_FML_Graph *graph, const void *node)
{
    for (size_t i = 0; i < graph->nodeCount; i++)
        if (graph->nodes[i] == node)
            return (int) i;
    return -1;
}

static void addComponent(LIBMATTI_FML_StronglyConnectedComponentDetector *detector, void **component, size_t size)
{
    detector->components = realloc(detector->components, sizeof(void **) * (detector->componentCount + 1));
    detector->componentSizes = realloc(detector->componentSizes, sizeof(size_t) * (detector->componentCount + 1));
    detector->components[detector->componentCount] = component;
    detector->componentSizes[detector->componentCount] = size;
    detector->componentCount++;
}

// Java: private void dfs(int now, int depth)
static void dfs(Tarjan *tarjan, int now, int depth)
{
    tarjan->dfn[now] = depth;
    tarjan->low[now] = depth;
    tarjan->top++;
    tarjan->stack[tarjan->top] = now;
    tarjan->onStack[now] = 1;

    size_t successorCount = 0;
    void **successors = LIBMATTI_FML_Graph_Successors(tarjan->detector->graph, tarjan->elements[now],
                                                      &successorCount);
    for (size_t i = 0; i < successorCount; i++)
    {
        int to = nodeIndex(tarjan->detector->graph, successors[i]);
        if (tarjan->dfn[to] != 0)
        {
            if (tarjan->low[now] > tarjan->dfn[to]) tarjan->low[now] = tarjan->dfn[to];
        }
        else
        {
            dfs(tarjan, to, depth + 1);
            if (tarjan->low[now] > tarjan->low[to]) tarjan->low[now] = tarjan->low[to];
        }
    }

    if (tarjan->dfn[now] == tarjan->low[now])
    {
        void **component = NULL;
        size_t componentSize = 0;
        while (tarjan->top >= 0)
        {
            int t = tarjan->stack[tarjan->top];
            component = realloc(component, sizeof(void *) * (componentSize + 1));
            component[componentSize++] = tarjan->elements[t];
            tarjan->onStack[t] = 0;
            tarjan->top--;
            if (t == now) break;
        }
        addComponent(tarjan->detector, component, componentSize);
    }
}

// Java: private void calculate()
static void calculate(LIBMATTI_FML_StronglyConnectedComponentDetector *detector)
{
    detector->components = NULL;
    detector->componentSizes = NULL;
    detector->componentCount = 0;

    LIBMATTI_FML_Graph *graph = detector->graph;
    int n = (int) graph->nodeCount;

    Tarjan tarjan = {0};
    tarjan.detector = detector;
    tarjan.ids = malloc(sizeof(int) * (n > 0 ? n : 1));
    tarjan.elements = calloc(n > 0 ? n : 1, sizeof(void *));
    for (int i = 0; i < n; i++)
    {
        tarjan.ids[i] = i;
        tarjan.elements[i] = graph->nodes[i];
    }

    tarjan.dfn = calloc(n > 0 ? n : 1, sizeof(int));
    tarjan.low = calloc(n > 0 ? n : 1, sizeof(int));
    tarjan.stack = malloc(sizeof(int) * (n > 0 ? n : 1));
    tarjan.onStack = calloc(n > 0 ? n : 1, sizeof(char));
    tarjan.top = -1;

    for (int i = 0; i < n; i++)
        if (tarjan.dfn[i] == 0)
            dfs(&tarjan, i, 1);

    free(tarjan.ids);
    free(tarjan.elements);
    free(tarjan.dfn);
    free(tarjan.low);
    free(tarjan.stack);
    free(tarjan.onStack);
}

// Java: public StronglyConnectedComponentDetector(Graph<T> graph)
LIBMATTI_FML_StronglyConnectedComponentDetector *LIBMATTI_FML_StronglyConnectedComponentDetector_New(
    LIBMATTI_FML_Graph *graph)
{
    LIBMATTI_FML_StronglyConnectedComponentDetector *detector =
        calloc(1, sizeof(LIBMATTI_FML_StronglyConnectedComponentDetector));
    detector->graph = graph;
    return detector;
}

// Java: public Set<Set<T>> getComponents()
void ***LIBMATTI_FML_StronglyConnectedComponentDetector_GetComponents(
    LIBMATTI_FML_StronglyConnectedComponentDetector *detector, size_t **componentSizes, size_t *componentCount)
{
    if (detector->components == NULL)
        calculate(detector);

    *componentSizes = detector->componentSizes;
    *componentCount = detector->componentCount;
    return detector->components;
}

void LIBMATTI_FML_StronglyConnectedComponentDetector_Free(LIBMATTI_FML_StronglyConnectedComponentDetector *detector)
{
    if (detector == NULL) return;
    for (size_t i = 0; i < detector->componentCount; i++) free(detector->components[i]);
    free(detector->components);
    free(detector->componentSizes);
    free(detector);
}

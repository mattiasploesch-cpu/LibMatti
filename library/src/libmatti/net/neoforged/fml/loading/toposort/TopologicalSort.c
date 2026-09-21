#include "libmatti/net/neoforged/fml/loading/toposort/TopologicalSort.h"

#include "libmatti/cpw/modlauncher/LogManager.h"
#include "libmatti/net/neoforged/fml/loading/toposort/StronglyConnectedComponentDetector.h"

#include <stdlib.h>
#include <string.h>

static LIBMATTI_ML_Logger *LOGGER(void)
{
    return LIBMATTI_ML_LogManager_GetLogger();
}

// Java: Queue<T> queue - ArrayDeque (FIFO) without a comparator, PriorityQueue (smallest first) with one
static void queue_add(void **queue, size_t *count, void *node, int (*comparator)(void *, void *, void *),
                      void *userdata)
{
    if (comparator == NULL)
    {
        queue[(*count)++] = node;
        return;
    }

    // Java: PriorityQueue.add - insert at the position the comparator selects
    size_t index = *count;
    while (index > 0 && comparator(queue[index - 1], node, userdata) > 0)
    {
        queue[index] = queue[index - 1];
        index--;
    }
    queue[index] = node;
    (*count)++;
}

static void *queue_remove(void **queue, size_t *count, int (*comparator)(void *, void *, void *), void *userdata)
{
    (void) comparator;
    (void) userdata;
    void *node = queue[0];
    memmove(queue, queue + 1, sizeof(void *) * (--(*count)));
    return node;
}

// Java: private static <T> void throwCyclePresentException(Set<Set<T>> components)
static const char *nodeName(void *node, void *userdata)
{
    const char *(*name)(void *) = userdata;
    return name != NULL ? name(node) : "";
}

// Java: public static <T> List<T> topologicalSort(Graph<T> graph, @Nullable Comparator<? super T> comparator)
void **LIBMATTI_FML_TopologicalSort_TopologicalSort(LIBMATTI_FML_Graph *graph,
                                                    int (*comparator)(void *a, void *b, void *userdata),
                                                    void *userdata, size_t *count,
                                                    LIBMATTI_FML_CyclePresentException **cycles)
{
    *count = 0;
    *cycles = NULL;

    // Java: Preconditions.checkArgument(graph.isDirected(), "Cannot topologically sort an undirected graph!");
    if (!LIBMATTI_FML_Graph_IsDirected(graph))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Cannot topologically sort an undirected graph!");
        return NULL;
    }
    // Java: Preconditions.checkArgument(!graph.allowsSelfLoops(), "Cannot topologically sort a graph with self loops!");
    if (LIBMATTI_FML_Graph_AllowsSelfLoops(graph))
    {
        LIBMATTI_ML_Logger_Error(LOGGER(), NULL, "Cannot topologically sort a graph with self loops!");
        return NULL;
    }

    size_t nodeCount = LIBMATTI_FML_Graph_NodeCount(graph);
    void **queue = malloc(sizeof(void *) * (nodeCount > 0 ? nodeCount : 1));
    size_t queueCount = 0;
    int *degrees = calloc(nodeCount > 0 ? nodeCount : 1, sizeof(int));
    void **results = malloc(sizeof(void *) * (nodeCount > 0 ? nodeCount : 1));

    for (size_t i = 0; i < nodeCount; i++)
    {
        size_t degree = LIBMATTI_FML_Graph_InDegree(graph, graph->nodes[i]);
        if (degree == 0)
            queue_add(queue, &queueCount, graph->nodes[i], comparator, userdata);
        else
            degrees[i] = (int) degree;
    }

    while (queueCount > 0)
    {
        void *current = queue_remove(queue, &queueCount, comparator, userdata);
        results[(*count)++] = current;

        size_t successorCount = 0;
        void **successors = LIBMATTI_FML_Graph_Successors(graph, current, &successorCount);
        for (size_t i = 0; i < successorCount; i++)
        {
            int index = -1;
            for (size_t j = 0; j < nodeCount; j++)
                if (graph->nodes[j] == successors[i])
                    index = (int) j;
            if (index < 0) continue;

            int updated = --degrees[index];
            if (updated == 0)
            {
                queue_add(queue, &queueCount, successors[i], comparator, userdata);
                degrees[index] = 0;
            }
        }
    }

    int hasDegrees = 0;
    for (size_t i = 0; i < nodeCount; i++)
    {
        if (degrees[i] == 0) continue;
        // Java: only the nodes that were added to the map are checked (degree > 0)
        hasDegrees = 1;
        break;
    }

    free(queue);

    if (!hasDegrees)
    {
        free(degrees);
        return results;
    }

    free(results);

    // Java: Set<Set<T>> components = new StronglyConnectedComponentDetector<>(graph).getComponents(); components.removeIf(set -> set.size() < 2);
    LIBMATTI_FML_StronglyConnectedComponentDetector *detector =
        LIBMATTI_FML_StronglyConnectedComponentDetector_New(graph);
    size_t *componentSizes = NULL;
    size_t componentCount = 0;
    void ***components = LIBMATTI_FML_StronglyConnectedComponentDetector_GetComponents(detector, &componentSizes,
                                                                                       &componentCount);

    void ***cycleNodes = NULL;
    size_t *cycleSizes = malloc(sizeof(size_t) * (componentCount > 0 ? componentCount : 1));
    size_t cycleCount = 0;
    for (size_t i = 0; i < componentCount; i++)
    {
        if (componentSizes[i] < 2) continue;
        cycleNodes = realloc(cycleNodes, sizeof(void **) * (cycleCount + 1));
        cycleNodes[cycleCount] = components[i];
        cycleSizes[cycleCount] = componentSizes[i];
        cycleCount++;
    }

    *cycles = LIBMATTI_FML_CyclePresentException_New(cycleNodes, cycleSizes, cycleCount, nodeName, NULL);

    free(degrees);
    LIBMATTI_FML_StronglyConnectedComponentDetector_Free(detector);
    return NULL;
}

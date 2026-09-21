#include "libmatti/net/neoforged/fml/loading/toposort/Graph.h"

#include <stdlib.h>

// Java: MutableGraph.addNode(node) - Guava keeps a set of nodes
static int nodeIndex(const LIBMATTI_FML_Graph *graph, const void *node)
{
    for (size_t i = 0; i < graph->nodeCount; i++)
        if (graph->nodes[i] == node)
            return (int) i;
    return -1;
}

// Java: GraphBuilder.directed().build()
LIBMATTI_FML_Graph *LIBMATTI_FML_Graph_New(int directed, int allowsSelfLoops)
{
    LIBMATTI_FML_Graph *graph = calloc(1, sizeof(LIBMATTI_FML_Graph));
    graph->directed = directed;
    graph->allowsSelfLoops = allowsSelfLoops;
    return graph;
}

void LIBMATTI_FML_Graph_Free(LIBMATTI_FML_Graph *graph)
{
    if (graph == NULL) return;
    for (size_t i = 0; i < graph->nodeCount; i++) free(graph->successors[i]);
    free(graph->nodes);
    free(graph->successors);
    free(graph->successorCounts);
    free(graph);
}

// Java: MutableGraph.addNode(node)
void LIBMATTI_FML_Graph_AddNode(LIBMATTI_FML_Graph *graph, void *node)
{
    if (nodeIndex(graph, node) >= 0) return;

    graph->nodes = realloc(graph->nodes, sizeof(void *) * (graph->nodeCount + 1));
    graph->successors = realloc(graph->successors, sizeof(void **) * (graph->nodeCount + 1));
    graph->successorCounts = realloc(graph->successorCounts, sizeof(size_t) * (graph->nodeCount + 1));
    graph->nodes[graph->nodeCount] = node;
    graph->successors[graph->nodeCount] = NULL;
    graph->successorCounts[graph->nodeCount] = 0;
    graph->nodeCount++;
}

// Java: MutableGraph.putEdge(nodeU, nodeV)
void LIBMATTI_FML_Graph_PutEdge(LIBMATTI_FML_Graph *graph, void *from, void *to)
{
    LIBMATTI_FML_Graph_AddNode(graph, from);
    LIBMATTI_FML_Graph_AddNode(graph, to);

    int index = nodeIndex(graph, from);
    for (size_t i = 0; i < graph->successorCounts[index]; i++)
        if (graph->successors[index][i] == to)
            return; // Guava's graph is a set of edges

    graph->successors[index] = realloc(graph->successors[index],
                                       sizeof(void *) * (graph->successorCounts[index] + 1));
    graph->successors[index][graph->successorCounts[index]++] = to;
}

// Java: graph.isDirected()
int LIBMATTI_FML_Graph_IsDirected(const LIBMATTI_FML_Graph *graph)
{
    return graph->directed;
}

// Java: graph.allowsSelfLoops()
int LIBMATTI_FML_Graph_AllowsSelfLoops(const LIBMATTI_FML_Graph *graph)
{
    return graph->allowsSelfLoops;
}

// Java: graph.nodes()
void **LIBMATTI_FML_Graph_Nodes(const LIBMATTI_FML_Graph *graph, size_t *count)
{
    *count = graph->nodeCount;
    return graph->nodes;
}

// Java: graph.successors(node)
void **LIBMATTI_FML_Graph_Successors(const LIBMATTI_FML_Graph *graph, const void *node, size_t *count)
{
    int index = nodeIndex(graph, node);
    *count = index >= 0 ? graph->successorCounts[index] : 0;
    return index >= 0 ? graph->successors[index] : NULL;
}

// Java: graph.predecessors(node)
void **LIBMATTI_FML_Graph_Predecessors(const LIBMATTI_FML_Graph *graph, const void *node, size_t *count)
{
    void **result = NULL;
    *count = 0;
    for (size_t i = 0; i < graph->nodeCount; i++)
    {
        for (size_t j = 0; j < graph->successorCounts[i]; j++)
        {
            if (graph->successors[i][j] != node) continue;
            result = realloc(result, sizeof(void *) * (*count + 1));
            result[(*count)++] = graph->nodes[i];
            break;
        }
    }
    return result;
}

// Java: graph.inDegree(node)
size_t LIBMATTI_FML_Graph_InDegree(const LIBMATTI_FML_Graph *graph, const void *node)
{
    size_t degree = 0;
    for (size_t i = 0; i < graph->nodeCount; i++)
        for (size_t j = 0; j < graph->successorCounts[i]; j++)
            if (graph->successors[i][j] == node)
                degree++;
    return degree;
}

// Java: graph.nodes().size()
size_t LIBMATTI_FML_Graph_NodeCount(const LIBMATTI_FML_Graph *graph)
{
    return graph->nodeCount;
}

// Java: com.google.common.graph.Graphs.reachableNodes(graph, node)
void **LIBMATTI_FML_Graph_ReachableNodes(const LIBMATTI_FML_Graph *graph, const void *node, size_t *count)
{
    void **result = malloc(sizeof(void *) * (graph->nodeCount > 0 ? graph->nodeCount : 1));
    *count = 0;
    if (nodeIndex(graph, node) < 0)
    {
        free(result);
        return NULL;
    }

    // breadth-first over the successors
    result[(*count)++] = (void *) node;
    for (size_t i = 0; i < *count; i++)
    {
        size_t successorCount = 0;
        void **successors = LIBMATTI_FML_Graph_Successors(graph, result[i], &successorCount);
        for (size_t s = 0; s < successorCount; s++)
        {
            int seen = 0;
            for (size_t r = 0; r < *count; r++)
                if (result[r] == successors[s])
                {
                    seen = 1;
                    break;
                }
            if (seen) continue;
            result[(*count)++] = successors[s];
        }
    }

    return result;
}

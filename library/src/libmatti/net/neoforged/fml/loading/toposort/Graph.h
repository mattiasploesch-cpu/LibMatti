// Stands in for com.google.common.graph.GraphBuilder / MutableGraph / Graph, which are external.
// Nodes are identified by pointer, which is what the sorter needs (Java compares them by identity too).

#ifndef MATTICRAFT_FML_LOADING_TOPOSORT_GRAPH_H
#define MATTICRAFT_FML_LOADING_TOPOSORT_GRAPH_H

#include <stddef.h>

typedef struct LIBMATTI_FML_Graph LIBMATTI_FML_Graph;

struct LIBMATTI_FML_Graph
{
    void **nodes;
    size_t nodeCount;
    void ***successors; // per node: the successors in edge insertion order
    size_t *successorCounts;
    int directed; // Java: graph.isDirected()
    int allowsSelfLoops; // Java: graph.allowsSelfLoops()
};

// Java: GraphBuilder.directed().build()
LIBMATTI_FML_Graph *LIBMATTI_FML_Graph_New(int directed, int allowsSelfLoops);
void LIBMATTI_FML_Graph_Free(LIBMATTI_FML_Graph *graph);

// Java: MutableGraph.addNode(node)
void LIBMATTI_FML_Graph_AddNode(LIBMATTI_FML_Graph *graph, void *node);
// Java: MutableGraph.putEdge(nodeU, nodeV)
void LIBMATTI_FML_Graph_PutEdge(LIBMATTI_FML_Graph *graph, void *from, void *to);

// Java: graph.isDirected()
int LIBMATTI_FML_Graph_IsDirected(const LIBMATTI_FML_Graph *graph);
// Java: graph.allowsSelfLoops()
int LIBMATTI_FML_Graph_AllowsSelfLoops(const LIBMATTI_FML_Graph *graph);
// Java: graph.nodes()
void **LIBMATTI_FML_Graph_Nodes(const LIBMATTI_FML_Graph *graph, size_t *count);
// Java: graph.successors(node)
void **LIBMATTI_FML_Graph_Successors(const LIBMATTI_FML_Graph *graph, const void *node, size_t *count);
// Java: graph.predecessors(node) - the nodes with an edge into node
void **LIBMATTI_FML_Graph_Predecessors(const LIBMATTI_FML_Graph *graph, const void *node, size_t *count);
// Java: graph.inDegree(node)
size_t LIBMATTI_FML_Graph_InDegree(const LIBMATTI_FML_Graph *graph, const void *node);
// Java: graph.nodes().size()
size_t LIBMATTI_FML_Graph_NodeCount(const LIBMATTI_FML_Graph *graph);

// Java: com.google.common.graph.Graphs.reachableNodes(graph, node) - the node itself included.
// Caller frees the array.
void **LIBMATTI_FML_Graph_ReachableNodes(const LIBMATTI_FML_Graph *graph, const void *node, size_t *count);

#endif //MATTICRAFT_FML_LOADING_TOPOSORT_GRAPH_H

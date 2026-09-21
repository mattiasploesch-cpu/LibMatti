// Port of net.neoforged.fml.loading.toposort.StronglyConnectedComponentDetector.
// Splits the graph into strongly connected components lazily with Tarjan's algorithm.

#ifndef MATTICRAFT_FML_LOADING_TOPOSORT_STRONGLYCONNECTEDCOMPONENTDETECTOR_H
#define MATTICRAFT_FML_LOADING_TOPOSORT_STRONGLYCONNECTEDCOMPONENTDETECTOR_H

#include "libmatti/net/neoforged/fml/loading/toposort/Graph.h"

#include <stddef.h>

typedef struct LIBMATTI_FML_StronglyConnectedComponentDetector LIBMATTI_FML_StronglyConnectedComponentDetector;

struct LIBMATTI_FML_StronglyConnectedComponentDetector
{
    LIBMATTI_FML_Graph *graph;
    // Java: private Set<Set<T>> components - NULL until calculate() ran
    void ***components;
    size_t *componentSizes;
    size_t componentCount;
};

// Java: public StronglyConnectedComponentDetector(Graph<T> graph)
LIBMATTI_FML_StronglyConnectedComponentDetector *LIBMATTI_FML_StronglyConnectedComponentDetector_New(
    LIBMATTI_FML_Graph *graph);

// Java: public Set<Set<T>> getComponents()
void ***LIBMATTI_FML_StronglyConnectedComponentDetector_GetComponents(
    LIBMATTI_FML_StronglyConnectedComponentDetector *detector, size_t **componentSizes, size_t *componentCount);

void LIBMATTI_FML_StronglyConnectedComponentDetector_Free(LIBMATTI_FML_StronglyConnectedComponentDetector *detector);

#endif //MATTICRAFT_FML_LOADING_TOPOSORT_STRONGLYCONNECTEDCOMPONENTDETECTOR_H

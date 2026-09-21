// Port of net.neoforged.fml.loading.toposort.TopologicalSort.

#ifndef MATTICRAFT_FML_LOADING_TOPOSORT_TOPOLOGICALSORT_H
#define MATTICRAFT_FML_LOADING_TOPOSORT_TOPOLOGICALSORT_H

#include "libmatti/net/neoforged/fml/loading/toposort/CyclePresentException.h"
#include "libmatti/net/neoforged/fml/loading/toposort/Graph.h"

#include <stddef.h>

// Java: public static <T> List<T> topologicalSort(Graph<T> graph, @Nullable Comparator<? super T> comparator)
// Returns the ordered nodes and sets *count, or NULL with *cycles set when the graph contains cycles.
void **LIBMATTI_FML_TopologicalSort_TopologicalSort(LIBMATTI_FML_Graph *graph,
                                                    int (*comparator)(void *a, void *b, void *userdata),
                                                    void *userdata, size_t *count,
                                                    LIBMATTI_FML_CyclePresentException **cycles);

#endif //MATTICRAFT_FML_LOADING_TOPOSORT_TOPOLOGICALSORT_H

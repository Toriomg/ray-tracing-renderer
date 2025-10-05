#pragma once

#include "dataStructs/aabb.hpp"

struct BVHNode {
    AABB box;
    
    // Si es un nodo interno, 'first_primitive_offset' es el índice del hijo izquierdo.
    // Si es un nodo hoja, 'first_primitive_offset' es el índice de la primera primitiva.
    uint32_t first_primitive_offset; 
    
    // Si es un nodo interno, 'primitive_count' es 0.
    // Si es un nodo hoja, 'primitive_count' > 0.
    uint32_t primitive_count;
};
#include "ants.hpp"
#include <iostream>

int main() {
    Anthill anthill;

    // Execute sequential pipeline phases
    anthill.load_from_file("assets/fourmiliere_3D.txt"); // 1. Map parsing and file data ingest
    anthill.bfs_algo();                                  // 2. Shortest distance vector mapping
    anthill.simulate();                                  // 3. Core pathfinding engine execution

    return 0;
}
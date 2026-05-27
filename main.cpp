#include "ants.hpp"
#include <iostream>

int main() {
    Anthill anthill;

    anthill.load_from_file("fourmiliere_3D.txt");
    anthill.bfs_algo();
    anthill.simulate();

    return 0;
}
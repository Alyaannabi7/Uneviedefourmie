#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <queue>

class Ants;
class Room;

class Room {
public:
    int max_capacity;
    std::vector<Ants*> current_ants; // Dynamic container to support capacity > 1
    std::string name;
    std::vector<std::string> tunnels_near; // Adjacency list for graph connections

    bool is_full();
    bool is_empty();
};

class Ants {
public:
    int id;
    std::string current_room;
};

class Anthill {
public:
    std::vector<Ants> list_ants;
    std::map<std::string, Room> rooms_total; // Main graph storage (Room Name -> Room Object)
    std::map<std::string, int> distances;    // Stores BFS short path scores to reach "Sd"

    void load_from_file(std::string filename);
    void bfs_algo();
    void simulate();
};
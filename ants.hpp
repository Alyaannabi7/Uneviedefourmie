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
    std::vector<Ants*> current_ants; // supports capacity > 1
    std::string name;
    std::vector<std::string> tunnels_near;

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
    std::map<std::string, Room> rooms_total;
    std::map<std::string, int> distances;

    void load_from_file(std::string filename);
    void bfs_algo();
    void simulate();
};
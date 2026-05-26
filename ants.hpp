#include <iostream>
#include <vector>

class Room{
 public:
 int max_capacity;
 Ants* pcurrent_room=nullptr;
 std::string name;
};

class Ants{
    public:
    int id;
    std::string current_room;

};

class Anthill{
    public:
    std::vector<Room> total_rooms;
    std::vector<Ants> total_ants;

};
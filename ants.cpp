#include <iostream>
#include "ants.hpp"
#include <fstream>
#include <string>
#include <map>
#include <queue>
#include <algorithm>

using namespace std;

bool Room::is_empty() {
    return current_ants.empty();
}

bool Room::is_full() {
    return (int)current_ants.size() >= max_capacity;
}

void Anthill::load_from_file(const string filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    int total_ants = 0;

    // 1. Secure parsing for the total number of ants
    while (getline(file, line)) {
        // Handle format variants like "f=10" or "F = 10"
        size_t equal_pos = line.find('=');
        if (equal_pos != string::npos) {
            line = line.substr(equal_pos + 1);
        }

        // Clean up hidden spaces and carriage returns
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(remove(line.begin(), line.end(), '\n'), line.end());

        if (!line.empty()) {
            try {
                total_ants = stoi(line);
                break; 
            } catch (const invalid_argument& e) {
                continue; // Ignore non-numeric lines before the population count
            }
        }
    }

    if (total_ants <= 0) {
        cerr << "Error: Invalid or missing ant population count at the beginning of the file." << endl;
        return;
    }

    // Initialize the ants and place them in the starting room ("Sv")
    for (int i = 1; i <= total_ants; i++) {
        Ants f;
        f.id = i;
        f.current_room = "Sv";
        list_ants.push_back(f);
    }

    // 2. Explicitly initialize "Sv" and "Sd" with infinite capacity
    Room sv, sd;
    sv.name = "Sv"; sv.max_capacity = total_ants + 1;
    sd.name = "Sd"; sd.max_capacity = total_ants + 1;
    rooms_total["Sv"] = sv;
    rooms_total["Sd"] = sd;

    // 3. Parse the rest of the file (Rooms and Tunnels)
    while (getline(file, line)) {
        if (line.empty()) continue;

        size_t dash = line.find("-");
        if (dash != string::npos) {
            // --- TUNNEL PARSING ---
            string roomA = line.substr(0, dash);
            string roomB = line.substr(dash + 1);

            // Clean whitespaces from room names
            roomA.erase(remove(roomA.begin(), roomA.end(), ' '), roomA.end());
            roomA.erase(remove(roomA.begin(), roomA.end(), '\r'), roomA.end());
            roomA.erase(remove(roomA.begin(), roomA.end(), '\n'), roomA.end());

            roomB.erase(remove(roomB.begin(), roomB.end(), ' '), roomB.end());
            roomB.erase(remove(roomB.begin(), roomB.end(), '\r'), roomB.end());
            roomB.erase(remove(roomB.begin(), roomB.end(), '\n'), roomB.end());

            // Safeguard: If a room in a tunnel wasn't declared yet, create it with default capacity (1)
            if (rooms_total.find(roomA) == rooms_total.end()) {
                Room r; r.name = roomA; r.max_capacity = 1;
                rooms_total[roomA] = r;
            }
            if (rooms_total.find(roomB) == rooms_total.end()) {
                Room r; r.name = roomB; r.max_capacity = 1;
                rooms_total[roomB] = r;
            }

            // Establish bidirectional edge connections
            rooms_total[roomA].tunnels_near.push_back(roomB);
            rooms_total[roomB].tunnels_near.push_back(roomA);

        } else {
            // --- ROOM PARSING ---
            size_t open  = line.find("{");
            size_t close = line.find("}");
            string name;
            int capacity = 1; // Default capacity as per project specifications

            if (open != string::npos) {
                name = line.substr(0, open);
                capacity = stoi(line.substr(open + 1, close - open - 1));
            } else {
                name = line;
            }

            // Clean whitespaces from the room name
            name.erase(remove(name.begin(), name.end(), ' '), name.end());
            name.erase(remove(name.begin(), name.end(), '\r'), name.end());
            name.erase(remove(name.begin(), name.end(), '\n'), name.end());

            if (name == "Sv" || name == "Sd" || name.empty()) continue;

            Room r;
            r.name = name;
            r.max_capacity = capacity;
            rooms_total[name] = r;
        }
    }
}

// Breadth-First Search (BFS) starting from "Sd" to map the shortest path scores
void Anthill::bfs_algo() {
    // Initialize all distances to -1 (unvisited/unreachable status)
    for (auto& p : rooms_total) distances[p.first] = -1;

    queue<string> q;
    q.push("Sd");
    distances["Sd"] = 0;

    while (!q.empty()) {
        string cur = q.front(); q.pop();
        for (const string& nb : rooms_total[cur].tunnels_near) {
            // If the neighbor hasn't been visited yet, assign its distance score
            if (distances[nb] == -1) {
                distances[nb] = distances[cur] + 1;
                q.push(nb);
            }
        }
    }
}

// Main Simulation Engine handling synchronous ant traffic flow
void Anthill::simulate() {
    // Put all ants inside the starting Vestibule ("Sv")
    for (Ants& a : list_ants) {
        rooms_total["Sv"].current_ants.push_back(&a);
    }

    int turn_count = 1; // Tracks simulation cycles for output formatting

    while (true) {
        // Global termination check: verify if the whole colony has reached "Sd"
        bool all_done = true;
        for (Ants& a : list_ants) {
            if (a.current_room != "Sd") { 
                all_done = false; 
                break; 
            }
        }
        if (all_done) break; // Exit main loop if everyone is safe

        bool moved = false;
        
        // Temporary structure to stack up valid routing intents for the current cycle
        struct Movement {
            Ants* ant;
            string from;
            string to;
        };
        vector<Movement> movements_this_turn;

        // Track virtual traffic occupancy during decision-making to prevent overcrowding
        map<string, int> temporary_occupancy;
        for (auto const& [name, room] : rooms_total) {
            temporary_occupancy[name] = room.current_ants.size();
        }

        // Compute pathfinding strategies for each ant
        for (Ants& a : list_ants) {
            if (a.current_room == "Sd") continue; // Skip ants that already reached destination

            string cur = a.current_room;
            int dist_cur = distances[cur];
            string best = "";

            // Evaluate adjacent tunnels
            for (const string& nb : rooms_total[cur].tunnels_near) {
                if (distances[nb] == -1) continue; // Skip dead ends/unreachable rooms
                if (distances[nb] >= dist_cur) continue; // Route constraint: must get closer to target

                // Capacity constraint check: verify if target room has an available slot
                bool ok = (nb == "Sd") || (temporary_occupancy[nb] < rooms_total[nb].max_capacity);

                if (ok) { 
                    best = nb; 
                    break; // Optimal path choice found for this turn, look no further
                }
            }

            // Log movement intention if a valid destination room is cleared
            if (!best.empty()) {
                movements_this_turn.push_back({&a, cur, best});
                temporary_occupancy[cur]--;
                temporary_occupancy[best]++;
            }
        }

        // --- SYNCHRONOUS ROUTING EXECUTION AND OUTPUT DISPLAY ---
        if (!movements_this_turn.empty()) {
            // Print out current turn header as requested by project standard format
            cout << "+++ E +++\n" << turn_count << "\n";
            
            // Commit all safe routing moves calculated during this step
            for (const Movement& m : movements_this_turn) {
                // 1. Remove ant pointer from its previous room
                auto& vec_from = rooms_total[m.from].current_ants;
                vec_from.erase(remove(vec_from.begin(), vec_from.end(), m.ant), vec_from.end());

                // 2. Relocate ant structure properties to the destination room
                m.ant->current_room = m.to;
                if (m.to != "Sd") {
                    rooms_total[m.to].current_ants.push_back(m.ant);
                }

                // 3. Print out structural shift using project specifications: f[id] - [from] - [to]
                cout << "f" << m.ant->id << " - " << m.from << " - " << m.to << "\n";
                moved = true;
            }
            turn_count++;
        }

        if (!moved) {
            break; // Anti-deadlock safety trigger to prevent freezing if no paths can evolve
        }
    }
}
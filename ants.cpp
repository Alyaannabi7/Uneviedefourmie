#include <iostream>
#include "ants.hpp"
#include <fstream>
#include <string>
#include <map>
#include <queue>

using namespace std;

 bool Room::is_empty(){
    return pcurrent_ants==nullptr;
}

 void Anthill::load_from_file(const string filename){

     ifstream file(filename);
     string first_line;
     string lines;

     if(file.is_open()){
            getline(file, first_line);
            int total_ants=stoi(first_line);

            for(int i=1; i<=total_ants; i++){
                Ants f;
                f.id=i;
                f.current_room="Sv";
                list_ants.push_back(f);
          }
          
          while(getline(file, lines)){
            if(lines.find("-")!=string::npos){
                std::string::size_type pos_crochet = lines.find("-");
                string roomA=lines.substr(0,pos_crochet);
                string roomB=lines.substr(pos_crochet+1);

                rooms_total[roomA].tunnels_near.push_back(roomB);
                rooms_total[roomB].tunnels_near.push_back(roomA);                
            }

            else{
                std::string::size_type crochet_pos = lines.find("{");
                std::string::size_type crochet_pos_end = lines.find("}");
                string name_room;
                int capacity_default=1;

                if (crochet_pos != string::npos) {
                name_room = lines.substr(0, crochet_pos);

                int number_int= crochet_pos_end - (crochet_pos + 1);
                string max_room_size = lines.substr(crochet_pos + 1, number_int);
                capacity_default = stoi(max_room_size);
                }
                else{
                    name_room = lines;
               }

               Room new_room;
               new_room.name = name_room;
               new_room.max_capacity = capacity_default;
               new_room.pcurrent_ants = nullptr; 
               rooms_total[name_room] = new_room;
          }
     }
}
}

  void Anthill::bfs_algo(){
    std::map<std::string, bool> visited;

    for (Room& r : total_rooms) {
        visited[r.name] = false;
    }
    std::queue<std::string> file;

    file.push("Sd");
    visited["Sd"] = true;
    distances["Sd"] = 0; 

    while (!file.empty()) {
        std::string actual = file.front();
        file.pop();
    
        for (std::string near_room : rooms_total[actual].tunnels_near) {
            
            if (!visited[near_room]) {
                visited[near_room] = true;
                distances[near_room] = distances[actual] + 1;
                
                file.push(near_room);
            }
        }
    }
}

void Anthill::simulate() {
    bool all_arrived = false;
    while (!all_arrived) {
        all_arrived = true; 
        bool moved_this_turn = false;

        for (Ants& a : list_ants) {
            if (a.current_room == "Sd") {
                continue; 
            }

            all_arrived = false;

            std::string current_pos = a.current_room;
            for (std::string neighbor : rooms_total[current_pos].tunnels_near) {
                
                if (distances[neighbor] < distances[current_pos]) {
                    
                    if (rooms_total[neighbor].is_empty() || neighbor == "Sd") { 

                        if (current_pos != "Sv") {
                            rooms_total[current_pos].pcurrent_ants = nullptr;
                        }

                        a.current_room = neighbor;
                        
                        if (neighbor != "Sd") {
                            rooms_total[neighbor].pcurrent_ants= &a;
                        }

                        std::cout << "L" << a.id << "-" << neighbor << " ";
                        
                        moved_this_turn = true;
                        break;
                    }
                }
            }
        }
        if (moved_this_turn) {
            std::cout << std::endl;
        }
    }
}

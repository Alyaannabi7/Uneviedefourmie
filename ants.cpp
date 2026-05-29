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
        cerr << "Erreur: impossible d'ouvrir " << filename << endl;
        return;
    }

    string line;
    int total_ants = 0;

    // 1. Recherche sécurisée du nombre de fourmis
    while (getline(file, line)) {
        size_t equal_pos = line.find('=');
        if (equal_pos != string::npos) {
            line = line.substr(equal_pos + 1);
        }

        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(remove(line.begin(), line.end(), '\n'), line.end());

        if (!line.empty()) {
            try {
                total_ants = stoi(line);
                break; 
            } catch (const invalid_argument& e) {
                continue;
            }
        }
    }

    if (total_ants <= 0) {
        cerr << "Erreur: Nombre de fourmis invalide ou introuvable au début du fichier." << endl;
        return;
    }

    // Création des fourmis
    for (int i = 1; i <= total_ants; i++) {
        Ants f;
        f.id = i;
        f.current_room = "Sv";
        list_ants.push_back(f);
    }

    // 2. Initialisation forcée de Sv et Sd avec une capacité infinie
    Room sv, sd;
    sv.name = "Sv"; sv.max_capacity = total_ants + 1;
    sd.name = "Sd"; sd.max_capacity = total_ants + 1;
    rooms_total["Sv"] = sv;
    rooms_total["Sd"] = sd;

    // 3. Lecture du reste du fichier (Salles et Tunnels)
    while (getline(file, line)) {
        if (line.empty()) continue;

        size_t dash = line.find("-");
        if (dash != string::npos) {
            // --- C'est un TUNNEL ---
            string roomA = line.substr(0, dash);
            string roomB = line.substr(dash + 1);

            // Nettoyage strict des espaces invisibles autour des noms
            roomA.erase(remove(roomA.begin(), roomA.end(), ' '), roomA.end());
            roomA.erase(remove(roomA.begin(), roomA.end(), '\r'), roomA.end());
            roomA.erase(remove(roomA.begin(), roomA.end(), '\n'), roomA.end());

            roomB.erase(remove(roomB.begin(), roomB.end(), ' '), roomB.end());
            roomB.erase(remove(roomB.begin(), roomB.end(), '\r'), roomB.end());
            roomB.erase(remove(roomB.begin(), roomB.end(), '\n'), roomB.end());

            // Sécurité : Si une pièce du tunnel n'a pas été déclarée avant, on la crée
            if (rooms_total.find(roomA) == rooms_total.end()) {
                Room r; r.name = roomA; r.max_capacity = 1;
                rooms_total[roomA] = r;
            }
            if (rooms_total.find(roomB) == rooms_total.end()) {
                Room r; r.name = roomB; r.max_capacity = 1;
                rooms_total[roomB] = r;
            }

            // Ajout des voisins réciproques
            rooms_total[roomA].tunnels_near.push_back(roomB);
            rooms_total[roomB].tunnels_near.push_back(roomA);

        } else {
            // --- C'est une SALLE ---
            size_t open  = line.find("{");
            size_t close = line.find("}");
            string name;
            int capacity = 1;

            if (open != string::npos) {
                name = line.substr(0, open);
                capacity = stoi(line.substr(open + 1, close - open - 1));
            } else {
                name = line;
            }

            // Nettoyage du nom de la salle
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

    // Le reste de la fonction (Initialisation de Sv, Sd et boucle while) reste EXACTEMENT LE MÊME...

// BFS depuis Sd pour avoir la distance de chaque salle vers Sd
void Anthill::bfs_algo() {
    for (auto& p : rooms_total) distances[p.first] = -1;

    queue<string> q;
    q.push("Sd");
    distances["Sd"] = 0;

    while (!q.empty()) {
        string cur = q.front(); q.pop();
        for (const string& nb : rooms_total[cur].tunnels_near) {
            if (distances[nb] == -1) {
                distances[nb] = distances[cur] + 1;
                q.push(nb);
            }
        }
    }
}
void Anthill::simulate() {
    // Placer toutes les fourmis initialement dans Sv
    for (Ants& a : list_ants) {
        rooms_total["Sv"].current_ants.push_back(&a);
    }

    int turn_count = 1; // Compteur pour les étapes +++ E +++

    while (true) {
        // Vérifier si toutes les fourmis sont arrivées
        bool all_done = true;
        for (Ants& a : list_ants) {
            if (a.current_room != "Sd") { 
                all_done = false; 
                break; 
            }
        }
        if (all_done) break;

        bool moved = false;
        
        // Structure temporaire pour stocker les mouvements du tour
        struct Movement {
            Ants* ant;
            string from;
            string to;
        };
        vector<Movement> movements_this_turn;

        // Map temporaire pour suivre l'occupation
        map<string, int> temporary_occupancy;
        for (auto const& [name, room] : rooms_total) {
            temporary_occupancy[name] = room.current_ants.size();
        }

        for (Ants& a : list_ants) {
            if (a.current_room == "Sd") continue;

            string cur = a.current_room;
            int dist_cur = distances[cur];
            string best = "";

            for (const string& nb : rooms_total[cur].tunnels_near) {
                if (distances[nb] == -1) continue;
                if (distances[nb] >= dist_cur) continue; 

                bool ok = (nb == "Sd") || (temporary_occupancy[nb] < rooms_total[nb].max_capacity);

                if (ok) { 
                    best = nb; 
                    break; 
                }
            }

            if (!best.empty()) {
                movements_this_turn.push_back({&a, cur, best});
                temporary_occupancy[cur]--;
                temporary_occupancy[best]++;
            }
        }

        // --- AFFICHAGE STRICT DU TOUR SELON LE SUJET ---
        if (!movements_this_turn.empty()) {
            // Affichage de la balise du tour (ex: +++ E +++ \n 1)
            cout << "+++ E +++\n" << turn_count << "\n";
            
            // On applique et on affiche chaque mouvement
            for (const Movement& m : movements_this_turn) {
                // 1. On l'enlève de l'ancienne pièce
                auto& vec_from = rooms_total[m.from].current_ants;
                vec_from.erase(remove(vec_from.begin(), vec_from.end(), m.ant), vec_from.end());

                // 2. On la met dans la nouvelle
                m.ant->current_room = m.to;
                if (m.to != "Sd") {
                    rooms_total[m.to].current_ants.push_back(m.ant);
                }

                // 3. Affichage au format exact : f[id] - [from] - [to]
                cout << "f" << m.ant->id << " - " << m.from << " - " << m.to << "\n";
                moved = true;
            }
            turn_count++;
        }

        if (!moved) {
            break; // Sécurité anti-blocage
        }
    }
}
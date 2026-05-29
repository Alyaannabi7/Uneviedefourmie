# 🐜 Projet : Une Vie de Fourmie (Simulation de Flux de Trafic)

Projet d'algorithmique et de Programmation Orientée Objet (POO) réalisé dans le cadre du cursus de Développement Logiciel à **La Plateforme_**.

---

## 📌 1. La Problématique Principale

L'objectif de ce projet est de concevoir un moteur de simulation performant en **C++** capable de guider une population de fourmis ($f$) à travers un réseau de galeries (représenté sous la forme d'un graphe bidirectionnel). La colonie doit voyager depuis un point d'entrée unique, le **Vestibule (`Sv`)**, jusqu'à sa destination finale, le **Dortoir (`Sd`)**.

La simulation se déroule selon un **cycle d'horloge synchrone au tour par tour**. Les principaux défis à relever sont :
* **Contraintes de Capacité Strictes :** Chaque pièce intermédiaire possède une capacité maximale d'accueil (`max_capacity`). Une pièce ne peut plus accepter de nouvelles fourmis si elle est pleine, ce qui oblige le moteur à réguler activement le flux de trafic pour éviter tout encombrement.
* **Optimisation du Temps :** Les fourmis doivent atteindre leur destination en un **minimum absolu de tours/étapes**, ce qui nécessite des choix de cheminement optimaux.
* **Blocages Structurels et Pièges :** Le moteur doit rester stable et efficace face à n'importe quelle configuration de fichier valide, même celles contenant des chemins asymétriques, des goulots d'étranglement massifs, des boucles fermées (cycles) ou des pièces totalement isolées.

---

## 🏗️ 2. Architecture du Projet & Conception POO

Le projet respecte strictement les principes de la Programmation Orientée Objet afin de garantir une séparation claire des responsabilités. Il est divisé en trois classes fondamentales :

### A. La Classe `Ants`
Représente une fourmi individuelle au sein de la colonie.
* **Attributs :**
  * `int id` : Un identifiant unique pour le suivi et l'affichage.
  * `std::string current_room` : Stocke le nom exact de la pièce dans laquelle la fourmi se trouve actuellement.

### B. La Classe `Room`
Représente une pièce physique (un nœud) à l'intérieur de la fourmilière.
* **Attributs :**
  * `std::string name` : Le nom de la pièce (ex: `"S1"`, `"Sv"`).
  * `int max_capacity` : Le nombre maximum de fourmis autorisées simultanément à l'intérieur.
  * `std::vector<Ants*> current_ants` : Un conteneur dynamique stockant des pointeurs vers les fourmis actuellement présentes, permettant de gérer pleinement les capacités supérieures à 1.
  * `std::vector<std::string> tunnels_near` : Une liste d'adjacence stockant les noms de toutes les pièces voisines connectées.
* **Méthodes :**
  * `is_empty()` : Renvoie `true` si la pièce ne contient aucune fourmi.
  * `is_full()` : Renvoie `true` si le nombre de fourmis présentes a atteint `max_capacity`.

### C. La Classe `Anthill`
La classe maîtresse (l'Architecte) qui orchestre tout le cycle de vie de la simulation.
* **Attributs :**
  * `std::vector<Ants> list_ants` : Le registre central contenant toutes les fourmis.
  * `std::map<std::string, Room> rooms_total` : Le dictionnaire principal du graphe reliant le nom des pièces à leurs objets `Room` réels.
  * `std::map<std::string, int> distances` : Relie chaque pièce à sa distance la plus courte absolue pour atteindre la destination (`Sd`).
* **Méthodes :**
  * `load_from_file(string filename)` : Analyse les fichiers texte d'entrée et construit le graphe.
  * `bfs_algo()` : Calcule la matrice des distances les plus courtes.
  * `simulate()` : Exécute la boucle principale du moteur synchrone tour par tour.

---

## 🚀 3. Notre Démarche Algorithmique Étape par Étape

La simulation progresse de manière séquentielle à travers trois phases architecturales distinctes :

### Phase 1 : Lecture des Données et Construction du Graphe (Parsing)
La méthode `load_from_file` lit le fichier de configuration ligne par ligne.
1. Elle capture d'abord le nombre total de fourmis, instancie les objets `Ants` et les place dans `"Sv"`.
2. Elle initialise les pièces virtuelles `"Sv"` et `"Sd"` en leur attribuant une capacité infinie.
3. Elle analyse les lignes restantes en séparant dynamiquement les déclarations de pièces (en stockant leurs capacités personnalisées, ou `1` par défaut) des liaisons de tunnels, qui viennent alimenter les listes d'adjacence bidirectionnelles.

### Phase 2 : Cartographie Spatiale via un BFS Inversé
Pour empêcher les fourmis d'errer au hasard, de faire marche arrière ou de se retrouver piégées dans des boucles de rétroaction, nous avons implémenté un algorithme de **Parcours en Largeur (BFS - Breadth-First Search)**.
* Le BFS démarre ses calculs directement depuis le nœud de destination (`Sd`) avec un score de départ de `0`, puis se propage à l'envers, couche par couche, vers l'entrée.
* Chaque pièce découverte reçoit un score numérique représentant sa distance exacte par rapport à la sortie. Les sous-réseaux isolés ou les impasses ne reçoivent jamais de score (restant marqués à `-1`), ce qui les rend totalement invisibles pour les fourmis.

### Phase 3 : Le Moteur de Trafic Synchrone
La boucle `simulate()` pilote la logique du tour par tour. À chaque cycle, le moteur parcourt le registre des fourmis. Une fourmi est autorisée à avancer vers une pièce voisine si et seulement si :
1. Le score de distance BFS de la pièce cible est strictement inférieur au score de la pièce actuelle (garantissant une progression constante vers la sortie).
2. La pièce cible dispose d'une place libre (`occupation < max_capacity`).

---

## 🛠️ 4. Problèmes Techniques Rencontrés & Solutions Apportées

Au cours du développement, nous avons été confrontés à plusieurs cas limites et bugs structurels complexes qui ont nécessité un débogage approfondi et une optimisation du code :

### A. Échec du Parsing : Espaces Invisibles et Retours Chariot
* **Le Problème :** Lors de nos premiers tests sur des fichiers générés sous Windows, notre moteur indiquait que `Sv` possédait `0 voisin`, ce qui interrompait immédiatement la simulation. Le parseur découpait des lignes comme `Sv - S1` sans nettoyer les espaces environnants, créant des clés de map distinctes pour `"Sv "` et `"Sv"`. De plus, les caractères de retour chariot Windows (`\r`) corrompaient les chaînes lors des conversions avec `std::stoi()`.
* **Le Solution :** Nous avons renforcé notre routine `load_from_file` en intégrant des étapes de nettoyage strictes grâce au **formalisme Erase-Remove** (`std::remove`). Cela supprime tous les espaces (`' '`), les sauts de ligne (`'\n'`) et les retours chariot (`\r`) avant d'enregistrer les entités du graphe dans notre table des symboles globale.

### B. Corruption de Mémoire : Modification de Conteneur en Pleine Itération
* **Le Problème :** Dans notre première tentative de routage multi-fourmis, nous avons essayé de supprimer instantanément le pointeur d'une fourmi du vecteur `current_ants` d'une pièce dès qu'elle décidait de bouger. Modifier la taille d'un vecteur et décaler ses éléments *pendant que l'on boucle sur la population de fourmis* a corrompu les itérateurs mémoire, provoquant des erreurs de segmentation (*segmentation faults*) et des sauts de données imprévisibles.
* **La Solution :** Nous avons dissocié la **prise de décision** de l'**exécution**. Dans `simulate()`, nous avons introduit un système transactionnel utilisant une structure de suivi temporaire :
  ```cpp
  struct Movement {
      Ants* ant;
      string from;
      string to;
  };
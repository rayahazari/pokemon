#include <bits/stdc++.h>
using namespace std;

const int LEVEL = 100;
const int CRITICAL = 1;
const string POKEDEX_FILE = "pokedex.csv";
const string POKEMON_MOVES_FILE = "pokemon_moves_status.csv";
const set <string> OHKO = {"fissure", "horn-drill", "guillotine", "sheer-cold"};

unsigned seed = chrono::system_clock::now().time_since_epoch().count();
std::mt19937 gen(seed);

int getRandomInt(int min_val, int max_val) {
    std::uniform_int_distribution<int> distrib(min_val, max_val);
    return distrib(gen);
}

enum class StatusCondition {
    NONE, BURN, PARALYZE, POISON, SLEEP, FREEZE
};

class Pokemon;

class Move {
public:
    string name, type, category;
    int power, accuracy, pp, priority;
    string statToChange, statTarget;
    int statChangeAmount;

    Move() : name(""), type(""), category(""), power(0), accuracy(0), pp(0), priority(0) {}

    Move (string &nameOfMove, string &typeOfMove, string &categoryOfMove, int powerOfMove, int accuracyOfMove, int maxPP, string &statToChange, int statAmount, string &statChangeTarget) : name(nameOfMove), type(typeOfMove), category(categoryOfMove), power(powerOfMove), accuracy(accuracyOfMove), pp(maxPP), statToChange(statToChange), statChangeAmount(statAmount), statTarget(statChangeTarget) {
        priority = 0;
        if (nameOfMove == "quick-attack") priority++;
    } 

    int calculateDamage(Pokemon &attacker, Pokemon &defender);
    double getRandomForDamageCalculation();
    double effectiveness (Pokemon &defendingPokemon);
    double isStab(Pokemon &attackingPokemon);
    Move getStruggle();
};

class Pokemon {
public:
    int pokedexID;
    string name;
    string type1, type2;
    int att, spA, def, spD, spe, hp;
    int current_health;
    map <string, double> weaknesses;
    vector <Move> moveSet;
    StatusCondition currentStatus = StatusCondition::NONE;
    int statusTurns = 0; 

    int attStage = 0, defStage = 0, spAStage = 0, spDStage = 0, speStage = 0;

    Pokemon() {
        pokedexID = 0;
        name = "";
        type1 = "";
        type2 = "";
        att = spA = def = spD = spe = hp = 0;
        current_health = 0;
    }

    Pokemon (int pokedex_number, string &name, string &firstType, string &secondType, int base_attack, int base_spA, int base_defense, int base_spD, int base_speed, int baseHP, map<string, double> &weakness_chart, vector <Move> &movePool) {
        pokedexID = pokedex_number;
        this->name = name;
        type1 = firstType;
        type2 = secondType;
        att = 2 * base_attack + 98;
        spA = 2 * base_spA + 98;
        def = 2 * base_defense + 98;
        spD = 2 * base_spD + 98;
        spe = 2 * base_speed + 98;
        hp = 2 * baseHP + 203;
        current_health = hp;
        weaknesses = weakness_chart;
        moveSet = movePool;
    }

    void display() {
        cout << "Pokedex ID : " << pokedexID << "\t\t";
        cout << "Name : " << name << "\t\t";
        cout << "Types : " << type1 << " " << type2 << "\t\t";
        cout << "Moves: ";
        for (Move &move : moveSet) {
            cout << move.name << " ";
        }
        cout << "\n";
    }

    void displayMoves() {
        for (int i=0; i<moveSet.size(); i++) cout << i+1 << ") " << moveSet[i].name << "\t";
        cout << "\n";
    }

    void drawHealthBar() {
        int barWidth = 50;
        float hpPercent = current_health*1.0f/hp;

        if (hpPercent < 0.0f) hpPercent = 0.0f;

        int filledBlocks = hpPercent * barWidth;

        string green = "\033[32m", red = "\033[31m", yellow = "\033[33m";
        string colorCode = green, defaultColor = "\033[0m";
        if (hpPercent <= 0.2f) colorCode = red;
        else if (hpPercent <= 0.5f) colorCode = yellow;

        cout << name << " HP: ["<< current_health << "/" << hp << "]\t[";;
        for (int i=0; i<barWidth; i++) {
            if (i < filledBlocks) {
                cout << colorCode << "\xDB" << defaultColor;
            }
            else {
                cout << " ";
            }
        }
        cout << "]\n";
        
    }

    Move& getRandomMove() {
        if (isOutOfUsableMoves()) return moveSet[0];
        int idx = getRandomInt(0, moveSet.size()-1);
        while (moveSet[idx].pp <= 0) {
            idx = getRandomInt(0, moveSet.size()-1);
        }
        return moveSet[idx];
    }

    bool applyStatus(StatusCondition newStatus) {
        if (currentStatus != StatusCondition::NONE) return false;

        currentStatus = newStatus;
        if (newStatus == StatusCondition::SLEEP) statusTurns = getRandomInt(1, 3);
        return true;
    }

    void applyStatChange (const string &statName, int amount) {
        if (current_health <= 0) return;

        if (statName == "attack") {
            attStage+=amount;
            if (attStage > 6) attStage = 6;
            else if (attStage < -6) attStage = -6;
        }
        else if (statName == "defense") {
            defStage+=amount;
            if (defStage > 6) defStage = 6;
            else if (defStage < -6) defStage = -6;
        }
        else if (statName == "special_defense") {
            spDStage+=amount;
            if (spDStage > 6) spDStage = 6;
            else if (spDStage < -6) spDStage = -6;
        }
        else if (statName == "special_attack") {
            spAStage+=amount;
            if (spAStage > 6) spAStage = 6;
            else if (spAStage < -6) spAStage = -6;
        }
        else if (statName == "speed") {
            speStage+=amount;
            if (speStage > 6) speStage = 6;
            else if (speStage < -6) speStage = -6;
        }
        else if (statName == "burn") {
            if (applyStatus(StatusCondition::BURN)) {
                cout << name << " was burnt!\n";
            }
            else cout << "It won't have any effect!\n";
        }
        else if (statName == "poison") {
            if (applyStatus(StatusCondition::POISON)) {
                cout << name << " was poisoned!\n";
            }
            else cout << "It won't have any effect!\n";
        }
        else if (statName == "paralyze") {
            if (applyStatus(StatusCondition::PARALYZE)) {
                cout << name << " was paralyzed!\n";
            }
            else cout << "It won't have any effect!\n";
        }
        else if (statName == "sleep") {
            if (applyStatus(StatusCondition::SLEEP)) {
                cout << name << " went to sleep!\n";
            }
            else cout << "It won't have any effect!\n";
        }
    }
    
    bool isFainted() {
        return current_health <= 0;
    }

    double getStageMultiplier(int stage) {
        if (stage > 0) return (2.0 + stage)/2.0;
        if (stage < 0) return 2.0/(2.0 - stage);
        return 1.0;
    }

    int getEffectiveAttack() {
        int calculatedAttack = att * getStageMultiplier(attStage);
        if (currentStatus == StatusCondition::BURN) calculatedAttack/=2;
        return calculatedAttack;
    }

    int getEffectiveDefense() {
        return def * getStageMultiplier(defStage);
    }

    int getEffectiveSpecialAttack() {
        return spA * getStageMultiplier(spAStage);
    }

    int getEffectiveSpecialDefense() {
        return spD * getStageMultiplier(spDStage);
    }
     
    int getEffectiveSpeed() {
        int calculatedSpeed = spe * getStageMultiplier(speStage);
        if (currentStatus == StatusCondition::PARALYZE) calculatedSpeed/=2;
        return calculatedSpeed;
    }

    bool isOutOfUsableMoves() {
        for (int i=0; i<(int)moveSet.size(); i++) {
            if (moveSet[i].pp>0) return false;
        }
        return true;
    }
};

int Move::calculateDamage(Pokemon &attacker, Pokemon &defender) {
    int damageDealt = 0;  
    if (power == 0 && OHKO.find(name)==OHKO.end()) return 0;
    else if (OHKO.find(name)!=OHKO.end()) {
        if (effectiveness(defender)) {
            damageDealt = defender.current_health;
            cout << "It's a one-hit KO!\n";
        }
        else return 0;
    }
    if (category == "physical"){
        double damagef = ((((2 * LEVEL * CRITICAL/5.0 + 2.0) * power * attacker.getEffectiveAttack()*1.0 / defender.getEffectiveDefense() * 1.0)/50.0) + 2) * isStab(attacker) * effectiveness(defender) * getRandomForDamageCalculation();

        damageDealt = int(floor(damagef));
    }
    else if (category == "special") {
        double damagef = ((((2 * LEVEL * CRITICAL/5.0 + 2.0) * power * attacker.getEffectiveSpecialAttack()*1.0 / defender.getEffectiveSpecialDefense() * 1.0)/50.0) + 2) * isStab(attacker) * effectiveness(defender) * getRandomForDamageCalculation();

        damageDealt = int(floor(damagef));
    }
    else {
        
    }
    return damageDealt;
}

double Move::getRandomForDamageCalculation() {
    int MIN_VAL_OF_SEED = 217;
    int MAX_VAL_OF_SEED = 255;
    return getRandomInt(MIN_VAL_OF_SEED, MAX_VAL_OF_SEED)*1.0/MAX_VAL_OF_SEED*1.0;
}

double Move::effectiveness (Pokemon &defendingPokemon) {
    return defendingPokemon.weaknesses[type];
}

double Move::isStab(Pokemon &attackingPokemon) {
    if (type == attackingPokemon.type1 || type == attackingPokemon.type2) return 1.5;
    else return 1.0;
}

Move Move::getStruggle() {
    string moveName = "struggle", moveType = "normal", moveCategory = "physical", s1 = "none", s2 = "none";
    int power = 50, acc = 100000, pp = 100000, statchange = 0;
    Move move = Move(moveName, moveType, moveCategory, power, acc, pp, s1, statchange, s2);
    return move;
}

class TurnManager {
public:

    void applyEndOfTurnStatus(Pokemon &currPokemon) {
        if (currPokemon.current_health <= 0) return;

        if (currPokemon.currentStatus == StatusCondition::BURN) {
            int burnDamage = max(1, currPokemon.hp/16);
            currPokemon.current_health -= min(burnDamage, currPokemon.current_health);
            cout << currPokemon.name << " was hurt by its burn!\n";
        }
        else if (currPokemon.currentStatus == StatusCondition::POISON) {
            int poisonDamage = max(1, currPokemon.hp/8);
            currPokemon.current_health -= min(poisonDamage, currPokemon.current_health);
            
            cout << currPokemon.name << " was hurt by poison!\n";
        }
    }

	string determine_first_attacker(Pokemon &user_pokemon, Move &user_move, Pokemon &opponent_pokemon, Move &opponent_move) {
    	if(user_move.priority > opponent_move.priority) return "user";
        else if(user_move.priority < opponent_move.priority) return "opponent";
        else {
        	if(user_pokemon.getEffectiveSpeed() > opponent_pokemon.getEffectiveSpeed()) return "user";
            else if(user_pokemon.spe < opponent_pokemon.spe) return "opponent";
        }
        if (getRandomInt(1,2) == 1) return "user";
        else return "opponent";
    }

    void printStatChanges(Pokemon &currPokemon, Move &move) {
        if (move.statChangeAmount == 1) {
            cout << currPokemon.name << "'s " << move.statToChange << " rose!\n";
        }
        else if (move.statChangeAmount == 2) {
            cout << currPokemon.name << "'s " << move.statToChange << " rose sharply!\n";
        }
        else if (move.statChangeAmount == -1) {
            cout << currPokemon.name << "'s " << move.statToChange << " fell!\n";
        }
        else if (move.statChangeAmount == -2) {
            cout << currPokemon.name << "'s " << move.statToChange << " fell sharply!\n";
        }
    }

    void executeAttack(Pokemon &attacker, Move &move, Pokemon &defender) {
        bool struggling = attacker.isOutOfUsableMoves();
        Move originalMove = move;

        if (struggling) {
            cout << attacker.name << " is out of usable moves!\n";
            move = move.getStruggle();
        }
        

        if (attacker.currentStatus == StatusCondition::SLEEP) {
            if  (attacker.statusTurns > 0) {
                cout << attacker.name << " is fast asleep!\n";
                attacker.statusTurns--;
                return;
            }
            else {
                cout << attacker.name << " woke up!\n";
                attacker.applyStatus(StatusCondition::NONE);
            }
        }

        if (attacker.currentStatus == StatusCondition::PARALYZE) {
            if (getRandomInt(1, 100) <= 25) {
                cout << attacker.name << " is paralyzed! It can't move!\n";
                return;
            }
        }

        if (!struggling) {
            move.pp--;
        }

    	cout << attacker.name << " used " << move.name << "!\n";
        if (getRandomInt(1, 100) > move.accuracy && move.statToChange=="none") {
            cout << "It missed!\n";
        }
        else {
            int damage = min(move.calculateDamage(attacker, defender), defender.current_health);
            if(damage > 0) {
                if (move.effectiveness(defender) > 1) cout << "It's super effective!\n";
                else if (move.effectiveness(defender) < 1) cout << "It's not very effective...\n";
                defender.current_health -= damage;
                if(defender.current_health < 0) defender.current_health = 0;
                cout << defender.name << " took " << damage << " damage! ";
            }
            else {
                if (move.effectiveness(defender) == 0) cout << defender.name << " is immune to " << move.name << "!\n";
            }

            if (move.statToChange != "none") {
                if (move.statTarget == "self") {
                    attacker.applyStatChange(move.statToChange, move.statChangeAmount);
                    printStatChanges(attacker, move);
                }
                else if (move.statTarget == "enemy" && defender.current_health > 0) {
                    defender.applyStatChange(move.statToChange, move.statChangeAmount);
                    printStatChanges(defender, move);
                }
            }
        }
        
        cout << defender.name << " has " << defender.current_health << " HP remaining.\n";

        if (struggling) {
            if (attacker.current_health > 0) {
                int recoil = min((int)(ceil(attacker.hp / 4.0)), attacker.current_health);
                cout << attacker.name << " is hurt by recoil. It lost " << recoil << " HP.\n";
                attacker.current_health -= recoil;
            }
            move = originalMove;
        }
    }
    
    void executeTurn(Pokemon &user_pokemon, Move &user_move, Pokemon &opponent_pokemon, Move &opponent_move) {
    	string first = determine_first_attacker(user_pokemon, user_move, opponent_pokemon, opponent_move);
        Pokemon *firstAttacker, *secondAttacker;
        Move *firstMove, *secondMove;
        
        if(first == "user") {
            firstAttacker = &user_pokemon;
            firstMove = &user_move;
            secondAttacker = &opponent_pokemon;
            secondMove = &opponent_move;
        } 
        else {
            firstAttacker = &opponent_pokemon;
            firstMove = &opponent_move;
            secondAttacker = &user_pokemon;
            secondMove = &user_move;
        }
        
        executeAttack(*firstAttacker, *firstMove, *secondAttacker);

        if(secondAttacker->current_health <= 0) return;
        
        executeAttack(*secondAttacker, *secondMove, *firstAttacker);

        applyEndOfTurnStatus(*firstAttacker);
        applyEndOfTurnStatus(*secondAttacker);

        cout << firstAttacker->name << " has " << firstAttacker->current_health << " HP remaining.\n";
        cout << secondAttacker->name << " has " << secondAttacker->current_health << " HP remaining.\n";

        cout << "\n";
    }
};

class Battle {
public:
    TurnManager turnManager; 
    
    bool isPartyDefeated(vector<Pokemon>& party) {
        for(Pokemon& p : party) {
            if(p.current_health > 0)
            return false;
        }
        return true;
    }

    void displayParty(vector <Pokemon> &party) {
        for (int i = 0; i < party.size(); i++) {
            cout << i + 1 << ") " << party[i].name << " (HP: " << party[i].current_health << "/" << party[i].hp << ")\n";

            cout << "Moves:\t";
            party[i].displayMoves();
        }
    }

    void handleSwitch(vector<Pokemon> &user_party, int &user_idx, vector<Pokemon> &opponent_party, int &opponent_idx, Move &opponent_move) {
        cout << "Select Pokemon to switch to (Enter number): ";
        displayParty(user_party);

        char switch_choice;
        cin >> switch_choice;
        while(!(switch_choice - '0' >= 1 && switch_choice - '0' <= user_party.size()) || user_party[(switch_choice - '0')-1].current_health<=0) {
            if(!(switch_choice - '0' >= 1 && switch_choice - '0' <= user_party.size())) {
                cout << "Invalid! Enter again: ";
                cin >> switch_choice;
            }
            else if (user_party[(switch_choice-'0')-1].isFainted()){
                cout << user_party[(switch_choice-'0')-1].name << " has fainted! Enter again: ";
                cin >> switch_choice;
            }
        }

        if (switch_choice - '0' - 1 == user_idx) return;
        

        cout << user_party[user_idx].name << ", return. ";
        user_idx = (switch_choice-'0')-1;
        cout << user_party[user_idx].name << ", I choose you!\n";

        turnManager.executeAttack(opponent_party[opponent_idx], opponent_move, user_party[user_idx]);
    }

    void handleAttack(vector<Pokemon> &user_party, int &user_idx, vector<Pokemon> &opponent_party, int &opponent_idx, Move &opponent_move) {

        cout << "Choose a move for " << user_party[user_idx].name << " (Enter number):\n";   
        user_party[user_idx].displayMoves();
        char user_moveID; 
        cin >> user_moveID;

        while (true) {
            if (!(user_moveID - '0' >= 1 && user_moveID - '0' <= user_party[user_idx].moveSet.size())) {
                cout << "Invalid! Enter again: ";
                cin >> user_moveID;
            } else if (user_party[user_idx].moveSet[(user_moveID - '0') - 1].pp <= 0 && !user_party[user_idx].isOutOfUsableMoves()) {
                cout << "That move has no PP left! Select another move: ";
                cin >> user_moveID;
            } else {
                break;
            }
        }

        Move user_move = user_party[user_idx].moveSet[(user_moveID - '0')-1];

        turnManager.executeTurn(user_party[user_idx], user_move, opponent_party[opponent_idx], opponent_move);
    }
    
    void mainBattle(vector<Pokemon> &user_party, vector<Pokemon> &opponent_party) {
    	int user_idx = 0;
        int opponent_idx = 0;
        
        cout << "BATTLE START!\n";
        cout << "Your Pokemon : ";
        displayParty(user_party);
        
        int turn_count=0;
        while (!isPartyDefeated(user_party) && !isPartyDefeated(opponent_party)) {
            turn_count++;
            
            Pokemon &user_pokemon = user_party[user_idx];
            Pokemon &opponent_pokemon = opponent_party[opponent_idx];

            cout << "Turn " << turn_count << "\n";
            // cout << user_pokemon.name << " (HP: " << user_pokemon.current_health << "/" << user_pokemon.hp << ") V/S " << opponent_pokemon.name << " (HP: " << opponent_pokemon.current_health << "/" << opponent_pokemon.hp << ")\n";
            user_pokemon.drawHealthBar();
            cout << "                V/S                    \n";
            opponent_pokemon.drawHealthBar();

            cout << "What do you want to do?\n";
            cout << "F : Fight\n";
            cout << "S : Switch\n";

            char choice;
            cin >> choice;
            while (choice!='S' && choice!='s' && choice!='F' && choice!='f') {
                cout << "Invalid choice! Enter again: ";
                cin >> choice;
            }

            Move opponent_move = opponent_pokemon.getRandomMove();
            
            if (choice == 'S' || choice == 's') {
                handleSwitch(user_party, user_idx, opponent_party, opponent_idx, opponent_move);
            }
            else {
                handleAttack(user_party, user_idx, opponent_party, opponent_idx, opponent_move);
            }

            if (opponent_party[opponent_idx].current_health <= 0) {
                cout << "Opponent's " << opponent_party[opponent_idx].name << " fainted!\n";
                for (int i = 0; i < opponent_party.size(); i++) {
                    if (opponent_party[i].current_health > 0) {
                        opponent_idx = i;
                        cout << "Opponent sent out " << opponent_party[opponent_idx].name << "!\n";
                        break;
                    }
                }
            }

            if (user_party[user_idx].current_health <= 0) {
                cout << "Your " << user_party[user_idx].name << " fainted!\n";
                
                if (!isPartyDefeated(user_party)) {
                    cout << "Select next Pokemon to send out:\n";
                    for (int i = 0; i < user_party.size(); i++) {
                        user_party[i].drawHealthBar();
                        cout << "\n";
                    }
    
                    char next_choice;
                    cin >> next_choice;
                    while(!(next_choice - '0' >= 1 && next_choice - '0' <= user_party.size()) || user_party[(next_choice - '0')-1].current_health==0) {
                        if(!(next_choice - '0' >= 1 && next_choice - '0' <= user_party.size())) {
                            cout << "Invalid! Enter again: ";
                            cin >> next_choice;
                        }
                        else if (user_party[(next_choice-'0')-1].isFainted()){
                            cout << user_party[(next_choice-'0')-1].name << " has fainted! Enter again: ";
                            cin >> next_choice;
                        }
                    }
                    
                    user_idx = (next_choice - '0') - 1;

                    cout << "You sent out " << user_party[user_idx].name << "!\n";
                }
            }
        }
        
        if(isPartyDefeated(user_party)) {
            cout << "You have no Pokemon left! OPPONENT WINS!\n";
        } else {
            cout << "You defeated the opponent! YOU WIN!\n";
        }
        cout << "BATTLE OVER!\n";
    }
};

vector <string> parse_csv_line(const string& line) {
    vector <string> result;
    string current;
    bool in_quotes = false;
    for(char c : line) {
        if(c == '"') {
            in_quotes=!in_quotes;
        }
        else if(c == ',' && !in_quotes) {
            result.push_back(current);
            current.clear();
        }
        else
        current+=c;
    }
    result.push_back(current);
    return result;
}

void shufflePossibleMoves(vector <Move> &allPossibleMoves) {
    shuffle(allPossibleMoves.begin(), allPossibleMoves.end(), gen);
}

Pokemon readPokemonData(int pokedexNumber) {

    ifstream pokedex_file(POKEDEX_FILE);
    ifstream pokemon_moves_file(POKEMON_MOVES_FILE);

    if (!pokedex_file.is_open()) {
        cerr << "Could not open " << POKEDEX_FILE << " !";
        return Pokemon();
    }
    if (!pokemon_moves_file.is_open()) {
        cerr << "Could not open " << POKEMON_MOVES_FILE << " !";
        return Pokemon();
    }
    string pokedex_line, moveset_line;
    getline(pokedex_file, pokedex_line);
    getline(pokemon_moves_file, moveset_line);

    int currentCounter = 0;

    while (currentCounter != pokedexNumber) {
        currentCounter++;
        getline(pokedex_file, pokedex_line);
        getline(pokemon_moves_file, moveset_line);
    }
    
    vector <string> row = parse_csv_line(pokedex_line);
    
    
    int current_id = stoi(row[0]);

    string name = row[1];
    string type1 = row[2];
    string type2 = row[3];

    int hp = stoi(row[5]);
    int att = stoi(row[6]);
    int spa = stoi(row[7]);
    int def = stoi(row[8]);
    int spd = stoi(row[9]);
    int spe = stoi(row[10]);

    map <string, double> weaknesses;
    weaknesses["bug"] = stod(row[12]);
    weaknesses["dark"] = stod(row[13]);
    weaknesses["dragon"] = stod(row[14]);
    weaknesses["electric"] = stod(row[15]);
    weaknesses["fairy"] = stod(row[16]);
    weaknesses["fighting"] = stod(row[17]);
    weaknesses["fire"] = stod(row[18]);
    weaknesses["flying"] = stod(row[19]);
    weaknesses["ghost"] = stod(row[20]);
    weaknesses["grass"] = stod(row[21]);
    weaknesses["ground"] = stod(row[22]);
    weaknesses["ice"] = stod(row[23]);
    weaknesses["normal"] = stod(row[24]);
    weaknesses["poison"] = stod(row[25]);
    weaknesses["psychic"] = stod(row[26]);
    weaknesses["rock"] = stod(row[27]);
    weaknesses["steel"] = stod(row[28]);
    weaknesses["water"] = stod(row[29]);

    pokedex_file.close();

    row.clear();
    row = parse_csv_line(moveset_line);

    string moveName="", moveType="", categoryMove="", statToChange = "", statTarget = "";
    int power=0, accuracy=100, powerPoints=0, statChangeAmount = 0;

    vector <Move> possibleMoves;

    for (int col = 3; col < row.size(); col += 9) {
        moveName = row[col];
        
        if (moveName.empty()) continue; 

        moveType = row[col+1];
        categoryMove = row[col+2];
        
        power = row[col+3].empty() ? 0 : stoi(row[col+3]);
        accuracy = row[col+4].empty() ? 0 : stoi(row[col+4]);
        powerPoints = row[col+5].empty() ? 0 : stoi(row[col+5]);

        string statToChange = row[col+6];
        int statAmount = row[col+7].empty() ? 0 : stoi(row[col+7]);
        string statTarget = row[col+8];

        possibleMoves.push_back(Move(moveName, moveType, categoryMove, power, accuracy, powerPoints, statToChange, statAmount, statTarget));
    }

    auto getMoveScore = [&](Move& m) {
        double score = m.power * (m.accuracy / 100.0);
        if (m.power == 0) {
            if (m.statToChange == "sleep" || m.statToChange == "paralyze" || m.statToChange == "burn" || m.statToChange == "poison") score = 60.0;
            else if (m.statToChange != "none") score = 40.0;
        }
        if (m.type == type1 || m.type == type2) score *= 1.5;
        return score;
    };

    sort(possibleMoves.begin(), possibleMoves.end(), [&](Move& a, Move& b) {
        return getMoveScore(a) > getMoveScore(b);
    });

    int movesLearnt = min(4, static_cast<int>(possibleMoves.size()));

    vector <Move> pokemonMoves;
    for (int i=0; i<movesLearnt; i++) {
        Move selectedMove = possibleMoves[i];
        pokemonMoves.push_back(selectedMove);
    }

    return Pokemon(pokedexNumber, name, type1, type2, att, spa, def, spd, spe, hp, weaknesses, pokemonMoves);     
}

Pokemon getRandomPokemon() {
    return readPokemonData(getRandomInt(1, 151));
}

int main() {
    ifstream pokedex_file(POKEDEX_FILE);
    ifstream pokemon_moves_file(POKEMON_MOVES_FILE);

    if (!pokedex_file.is_open()) {
        cerr << "Could not open " << POKEDEX_FILE << " !";
        return -1;
    }

    if (!pokemon_moves_file.is_open()) {
        cerr << "Could not open " << POKEMON_MOVES_FILE << " !";
        return -1;
    }

    vector <Pokemon> p1, p2;

    Battle *battle;
    for (int i=0; i<6; i++) {
        p1.push_back(getRandomPokemon());
        p2.push_back(getRandomPokemon());
    }
    battle->mainBattle(p1, p2);
}

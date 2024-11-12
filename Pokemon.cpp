#include <string>
#include "Move.cpp"
#include <vector>
class Pokemon{
    public:
        std::string Name;
        int id;
        std::vector<std::string> Type;
        int Moves;

    Pokemon(std::string N, int I, std::vector<std::string> T, int M){
        Name = N;
        id = I;
        Type = T;
        Moves = M;
    }

    void attack(){
        //attack opp pokmemon using one of the 4 move attributes
    };

    void defend(){
        // defend from opp pokemon using one ofthe 4 move attributes
    };
};
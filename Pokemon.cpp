#include <string>
#include "Move.cpp"
#include <vector>
class Pokemon{
    public:
        std::string Name;
        int id;
        int height;
        std::vector<Move> Moves;

    Pokemon(std::string N, int I, int H,  std::vector<Move> M){
        Name = N;
        id = I;
        height = H;
        Moves = M;
    }
};
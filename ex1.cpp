#include <iostream>
#include <vector>
#include <bitset>

using namespace std;

class CellularAutomaton {
private:
    vector<int> state;
    int rule;

    int get_next_cell(int left, int center, int right) {
        int index = (left << 2) | (center << 1) | right;
        return (rule >> index) & 1;
    }

public:
    CellularAutomaton(int r) : rule(r) {}

    void init_state(const vector<int>& initial) {
        state = initial;
    }

    void evolve() {
        int n = state.size();
        vector<int> new_state(n);
        
        for (int i = 0; i < n; i++) {
            int left = (i == 0) ? 0 : state[i - 1];
            int center = state[i];
            int right = (i == n - 1) ? 0 : state[i + 1];
            
            new_state[i] = get_next_cell(left, center, right);
        }
        
        state = new_state;
    }

    void print() {
        for (int cell : state) {
            cout << (cell ? "█" : " ");
        }
        cout << endl;
    }

    vector<int> get_state() {
        return state;
    }
};

int main() {
    CellularAutomaton ca(30);
    
    vector<int> initial = {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0};
    ca.init_state(initial);
    
    cout << "Rule 30:" << endl;
    for (int i = 0; i < 15; i++) {
        ca.print();
        ca.evolve();
    }
    
    cout << "\nRule 90:" << endl;
    CellularAutomaton ca90(90);
    ca90.init_state(initial);
    for (int i = 0; i < 15; i++) {
        ca90.print();
        ca90.evolve();
    }
    
    cout << "\nRule 110:" << endl;
    CellularAutomaton ca110(110);
    ca110.init_state(initial);
    for (int i = 0; i < 15; i++) {
        ca110.print();
        ca110.evolve();
    }
    
    return 0;
}
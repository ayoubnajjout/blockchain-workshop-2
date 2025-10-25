#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

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

string ac_hash(const string& input, uint32_t rule, size_t steps) {
    vector<int> bits;
    
    for (char c : input) {
        for (int i = 7; i >= 0; i--) {
            bits.push_back((c >> i) & 1);
        }
    }
    
    while (bits.size() < 256) {
        bits.push_back(0);
    }
    
    if (bits.size() > 512) {
        vector<int> folded(512, 0);
        for (size_t i = 0; i < bits.size(); i++) {
            folded[i % 512] ^= bits[i];
        }
        bits = folded;
    }
    
    CellularAutomaton ca(rule);
    ca.init_state(bits);
    
    for (size_t i = 0; i < steps; i++) {
        ca.evolve();
    }
    
    vector<int> final_state = ca.get_state();
    vector<int> hash_bits(256);
    
    for (int i = 0; i < 256; i++) {
        hash_bits[i] = final_state[i % final_state.size()] ^ 
                       final_state[(i * 3) % final_state.size()];
    }
    
    stringstream ss;
    for (int i = 0; i < 256; i += 8) {
        int byte = 0;
        for (int j = 0; j < 8; j++) {
            byte = (byte << 1) | hash_bits[i + j];
        }
        ss << hex << setw(2) << setfill('0') << byte;
    }
    
    return ss.str();
}

int main() {
    string input1 = "Hello, World!";
    string input2 = "Hello, World?";
    string input3 = "Completely different text";
    
    string hash1 = ac_hash(input1, 30, 100);
    string hash2 = ac_hash(input2, 30, 100);
    string hash3 = ac_hash(input3, 30, 100);
    
    cout << "Input 1: \"" << input1 << "\"" << endl;
    cout << "Hash 1:  " << hash1 << endl << endl;
    
    cout << "Input 2: \"" << input2 << "\"" << endl;
    cout << "Hash 2:  " << hash2 << endl << endl;
    
    cout << "Input 3: \"" << input3 << "\"" << endl;
    cout << "Hash 3:  " << hash3 << endl << endl;
    
    cout << "Hash 1 == Hash 2? " << (hash1 == hash2 ? "YES" : "NO") << endl;
    cout << "Hash 1 == Hash 3? " << (hash1 == hash3 ? "YES" : "NO") << endl;
    cout << "Hash 2 == Hash 3? " << (hash2 == hash3 ? "YES" : "NO") << endl;
    
    return 0;
}
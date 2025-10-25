#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <openssl/sha.h>

using namespace std;

enum HashMode {
    SHA256_MODE,
    AC_HASH_MODE
};

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

string sha256_hash(const string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input.c_str(), input.size(), hash);
    
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

class Block {
public:
    int index;
    string data;
    string previous_hash;
    time_t timestamp;
    int nonce;
    string hash;

    Block(int idx, string d, string prev_hash) {
        index = idx;
        data = d;
        previous_hash = prev_hash;
        timestamp = time(nullptr);
        nonce = 0;
        hash = "";
    }

    string calculate_hash(HashMode mode) {
        stringstream ss;
        ss << index << data << previous_hash << timestamp << nonce;
        
        if (mode == SHA256_MODE) {
            return sha256_hash(ss.str());
        } else {
            return ac_hash(ss.str(), 30, 100);
        }
    }

    void mine_block(int difficulty, HashMode mode) {
        string target(difficulty, '0');
        
        do {
            nonce++;
            hash = calculate_hash(mode);
        } while (hash.substr(0, difficulty) != target);
        
        cout << "Block mined: " << hash << endl;
    }
};

class Blockchain {
private:
    vector<Block> chain;
    int difficulty;
    HashMode hash_mode;

public:
    Blockchain(int diff, HashMode mode) {
        difficulty = diff;
        hash_mode = mode;
        chain.push_back(create_genesis_block());
    }

    Block create_genesis_block() {
        Block genesis(0, "Genesis Block", "0");
        genesis.hash = genesis.calculate_hash(hash_mode);
        return genesis;
    }

    Block get_last_block() {
        return chain.back();
    }

    void add_block(Block new_block) {
        new_block.previous_hash = get_last_block().hash;
        new_block.mine_block(difficulty, hash_mode);
        chain.push_back(new_block);
    }

    bool is_chain_valid() {
        for (size_t i = 1; i < chain.size(); i++) {
            Block current = chain[i];
            Block previous = chain[i - 1];

            if (current.hash != current.calculate_hash(hash_mode)) {
                return false;
            }

            if (current.previous_hash != previous.hash) {
                return false;
            }
        }
        return true;
    }

    void print_chain() {
        for (Block& block : chain) {
            cout << "Block #" << block.index << endl;
            cout << "Data: " << block.data << endl;
            cout << "Hash: " << block.hash << endl;
            cout << "Previous Hash: " << block.previous_hash << endl;
            cout << "Nonce: " << block.nonce << endl << endl;
        }
    }
};

int main() {
    cout << "=== Blockchain with SHA256 ===" << endl;
    Blockchain blockchain_sha(4, SHA256_MODE);
    blockchain_sha.add_block(Block(1, "Transaction 1", ""));
    blockchain_sha.add_block(Block(2, "Transaction 2", ""));
    cout << "Chain valid: " << (blockchain_sha.is_chain_valid() ? "YES" : "NO") << endl << endl;

    cout << "=== Blockchain with AC_HASH ===" << endl;
    Blockchain blockchain_ac(4, AC_HASH_MODE);
    blockchain_ac.add_block(Block(1, "Transaction 1", ""));
    blockchain_ac.add_block(Block(2, "Transaction 2", ""));
    cout << "Chain valid: " << (blockchain_ac.is_chain_valid() ? "YES" : "NO") << endl << endl;

    blockchain_ac.print_chain();

    return 0;
}
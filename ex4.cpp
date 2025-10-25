#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

using namespace std;
using namespace std::chrono;

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

uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

string sha256_hash(const string& input) {
    uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                     0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    
    uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    
    vector<uint8_t> data(input.begin(), input.end());
    uint64_t bit_len = data.size() * 8;
    
    data.push_back(0x80);
    while ((data.size() % 64) != 56) {
        data.push_back(0x00);
    }
    
    for (int i = 7; i >= 0; i--) {
        data.push_back((bit_len >> (i * 8)) & 0xff);
    }
    
    for (size_t chunk = 0; chunk < data.size(); chunk += 64) {
        uint32_t w[64] = {0};
        
        for (int i = 0; i < 16; i++) {
            w[i] = (data[chunk + i * 4] << 24) | (data[chunk + i * 4 + 1] << 16) |
                   (data[chunk + i * 4 + 2] << 8) | data[chunk + i * 4 + 3];
        }
        
        for (int i = 16; i < 64; i++) {
            uint32_t s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
            uint32_t s1 = rotr(w[i-2], 17) ^ rotr(w[i-2], 19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];
        
        for (int i = 0; i < 64; i++) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = hh + S1 + ch + k[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;
            
            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }
    
    stringstream ss;
    for (int i = 0; i < 8; i++) {
        ss << hex << setw(8) << setfill('0') << h[i];
    }
    return ss.str();
}

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

    int mine_block(int difficulty, HashMode mode) {
        string target(difficulty, '0');
        int iterations = 0;
        
        do {
            nonce++;
            iterations++;
            hash = calculate_hash(mode);
        } while (hash.substr(0, difficulty) != target);
        
        return iterations;
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
};

struct BenchmarkResult {
    double avg_time_ms;
    double avg_iterations;
};

BenchmarkResult benchmark_mining(HashMode mode, int difficulty, int num_blocks) {
    Blockchain blockchain(difficulty, mode);
    
    double total_time = 0;
    double total_iterations = 0;
    
    for (int i = 0; i < num_blocks; i++) {
        stringstream ss;
        ss << "Transaction " << (i + 1);
        Block block(i + 1, ss.str(), "");
        block.previous_hash = blockchain.get_last_block().hash;
        
        auto start = high_resolution_clock::now();
        int iterations = block.mine_block(difficulty, mode);
        auto end = high_resolution_clock::now();
        
        double duration = duration_cast<milliseconds>(end - start).count();
        total_time += duration;
        total_iterations += iterations;
    }
    
    return {total_time / num_blocks, total_iterations / num_blocks};
}

void print_table(const vector<tuple<int, BenchmarkResult, BenchmarkResult>>& results) {
    cout << "+------------+------------------+------------------+------------------+------------------+" << endl;
    cout << "| Difficulty |  SHA256 Time(ms) | SHA256 Iterations|  AC_HASH Time(ms)| AC_HASH Iterations|" << endl;
    cout << "+------------+------------------+------------------+------------------+------------------+" << endl;
    
    for (const auto& result : results) {
        int diff = get<0>(result);
        BenchmarkResult sha_res = get<1>(result);
        BenchmarkResult ac_res = get<2>(result);
        
        cout << "| " << setw(10) << diff << " | ";
        cout << setw(16) << fixed << setprecision(2) << sha_res.avg_time_ms << " | ";
        cout << setw(16) << fixed << setprecision(0) << sha_res.avg_iterations << " | ";
        cout << setw(16) << fixed << setprecision(2) << ac_res.avg_time_ms << " | ";
        cout << setw(17) << fixed << setprecision(0) << ac_res.avg_iterations << " |" << endl;
    }
    
    cout << "+------------+------------------+------------------+------------------+------------------+" << endl;
}

int main() {
    const int NUM_BLOCKS = 10;
    vector<int> difficulties = {3, 4};
    vector<tuple<int, BenchmarkResult, BenchmarkResult>> results;
    
    cout << "Benchmarking mining performance..." << endl << endl;
    
    for (int difficulty : difficulties) {
        cout << "Testing difficulty " << difficulty << "..." << endl;
        
        BenchmarkResult sha_result = benchmark_mining(SHA256_MODE, difficulty, NUM_BLOCKS);
        cout << "SHA256 completed" << endl;
        
        BenchmarkResult ac_result = benchmark_mining(AC_HASH_MODE, difficulty, NUM_BLOCKS);
        cout << "AC_HASH completed" << endl << endl;
        
        results.push_back(make_tuple(difficulty, sha_result, ac_result));
    }
    
    cout << "\n=== BENCHMARK RESULTS (Average over " << NUM_BLOCKS << " blocks) ===" << endl << endl;
    print_table(results);
    
    return 0;
}
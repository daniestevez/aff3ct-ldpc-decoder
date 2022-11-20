#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <aff3ct.hpp>
using namespace aff3ct;

struct params {
    int K = 1024;           // number of information bits
    int N = 2048 + 512;     // codeword size
    int puncturing = 512;   // number of punctured bits
    int n_ite = 200;        // number of decoder iterations
    tools::Sparse_matrix H; // parity check matrix
    std::vector<uint32_t> info_bits_pos;
    // The maximum check-node degree for the AR4JA code used is 6
    Update_rule_SPA<> update_rule = tools::Update_rule_SPA<>(6);
};
void init_params(params& p, const char* alist_path);

struct modules {
    std::unique_ptr<module::Modem_BPSK<>> modem;
    std::unique_ptr<module::Decoder_LDPC_BP_flooding<>> decoder;
};
void init_modules(const params& p, modules& m);

struct buffers {
    std::vector<float> sigma;
    std::vector<float> noisy_symbols;
    std::vector<float> LLRs;
    std::vector<int> dec_bits;
    std::vector<uint8_t> dec_bits_packed;
};
void init_buffers(const params& p, buffers& b);

void usage(char** argv)
{
    std::cerr << "usage: " << argv[0]
              << " <alist_file> <input_frames.f32> <output_frames.u8>" << std::endl;
}


int main(int argc, char** argv)
{
    if (argc != 4) {
        usage(argv);
        return 1;
    }

    params p;
    init_params(p, argv[1]);
    modules m;
    init_modules(p, m);
    buffers b;
    init_buffers(p, b);

    FILE* input_frames = fopen(argv[2], "rb");
    if (input_frames == nullptr) {
        std::cerr << "could not open input file" << std::endl;
        return 1;
    }
    FILE* output_frames = fopen(argv[3], "wb");
    if (output_frames == nullptr) {
        std::cerr << "could not open output file" << std::endl;
        return 1;
    }

    std::fill(b.noisy_symbols.begin(), b.noisy_symbols.end(), 0.0);
    std::fill(b.sigma.begin(), b.sigma.end(), 0.5);

    size_t total_frames = 0;
    size_t failed_frames = 0;

    while (fread(&b.noisy_symbols[0],
                 (p.N - p.puncturing) * sizeof(float),
                 1,
                 input_frames) == 1) {
        ++total_frames;
        for (int j = 0; j < p.N - p.puncturing; ++j) {
            b.noisy_symbols[j] = -b.noisy_symbols[j];
        }
        m.modem->demodulate(b.sigma, b.noisy_symbols, b.LLRs);
        int ret = m.decoder->decode_siho(b.LLRs, b.dec_bits);
        if (ret) {
            std::cerr << "failed to decode frame" << std::endl;
            ++failed_frames;
            continue;
        }
        uint8_t x = 0;
        for (int j = 0; j < p.K; ++j) {
            x = (x << 1) | b.dec_bits[j];
            if (j % 8 == 7) {
                b.dec_bits_packed[j / 8] = x;
            }
        }
        if (fwrite(&b.dec_bits_packed[0], b.dec_bits_packed.size(), 1, output_frames) !=
            1) {
            std::cerr << "could not write to output file" << std::endl;
            return 1;
        }
    }

    fclose(input_frames);
    fclose(output_frames);

    std::cout << "decoded " << (total_frames - failed_frames) << "/" << total_frames
              << " frames (" << failed_frames << " failed)" << std::endl;

    return 0;
}

void init_params(params& p, const char* alist_path)
{
    std::ifstream alist_file;
    alist_file.open(alist_path, std::ifstream::in);
    p.H = tools::AList::read(alist_file);

    for (unsigned j = 0; j < p.K; ++j) {
        p.info_bits_pos.push_back(j);
    }
}

void init_modules(const params& p, modules& m)
{
    m.modem = std::unique_ptr<module::Modem_BPSK<>>(new module::Modem_BPSK<>(p.N));
    m.decoder = std::unique_ptr<module::Decoder_LDPC_BP_flooding<>>(
        new module::Decoder_LDPC_BP_flooding<>(
            p.K, p.N, p.n_ite, p.H, p.info_bits_pos, p.update_rule));
};

void init_buffers(const params& p, buffers& b)
{
    b.sigma = std::vector<float>(1);
    b.noisy_symbols = std::vector<float>(p.N);
    b.LLRs = std::vector<float>(p.N);
    b.dec_bits = std::vector<int>(p.K);
    b.dec_bits_packed = std::vector<uint8_t>(p.K / 8);
}

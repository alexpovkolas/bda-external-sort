#include <iostream>
#include <vector>
#include <fstream>
#include <math.h>
#include <algorithm>

using namespace std;

//#define __PROFILE__

#ifdef __PROFILE__

#include <random>
#include <chrono>

#endif


typedef unsigned long long item;

#ifdef __PROFILE__

void gen_test(item n) {

    ofstream file("input.bin", ios::binary);
    file.write((char *)&n, 8);

    vector<item> items(n);
    for (item i = n; i > 0; i--){
        items[i - 1] = i;
    }
    auto rng = default_random_engine {};
    //shuffle(begin(items), end(items), rng);

    for (item i = n; i > 0; i--){
        file.write((char *)&items[i-1], 8);
    }

    file.close();
}

#endif

struct input_chunk {
    ifstream *in;
    item *block;
    item file_size;
    item file_offset;
    item buffer_offset;
    item block_size;
    item chunk_count;
    item read_size;
    item item_size = 8;

    input_chunk(const string& file, char* block, item block_size) {
        in = new ifstream(file, ios::binary);
        this->block = (item *)block;
        in->read((char *)&(file_size), item_size);
        file_offset = item_size;
        this->block_size = block_size;
        buffer_offset = 0;
        chunk_count = 0;

        in->seekg(file_offset);
        read_size = get_next_read_size();
        in->read(block, read_size);
        file_offset += read_size;
        chunk_count++;
    }

    item get_next_read_size() {
        return file_size * item_size - chunk_count * block_size >= block_size ? block_size : file_size * item_size - chunk_count * block_size;
    }

    inline bool hasNext() {
        if (buffer_offset * item_size < read_size) {
            return true;
        } else if (file_offset >= (file_size + 1) * item_size) {
            return false;
        } else {
            read_size = get_next_read_size();
            in->seekg(file_offset);
            in->read((char*)block, read_size);
            file_offset += read_size;
            chunk_count++;
            buffer_offset = 0;

            return true;
        }
    }

    inline item next(){
        return block[buffer_offset];
    }

    inline void move(){
        buffer_offset++;
    }
};

struct output_chunk {

    ofstream *out;
    item *block;
    item block_size;
    item file_offset;
    item buffer_offset;
    item item_size = 8;
    item total_size;

    output_chunk(const string& file, char* block, item block_size, item total_size) {
        out = new ofstream(file, ios::binary);
        this->block = (item*)block;
        this->block_size = block_size;
        out->write((char *)&total_size, item_size);
        file_offset = item_size;
        buffer_offset = 0;
        this->total_size = total_size;
    }

    inline void push(item next){
#ifdef __PROFILE__
        cout << "Push: "<< next<< endl;
#endif
        if (buffer_offset * item_size >= block_size) {
            out->seekp(file_offset);
            out->write((char*)block, block_size);
            buffer_offset = 0;
            file_offset += block_size;
        }

        block[buffer_offset++] = next;
    }

    void close() {
        if ((total_size + 1) * item_size > file_offset) {
            out->seekp(file_offset);
            out->write((char*)block, buffer_offset * item_size);
            out->close();
        }

    }
};

void external_merge(vector<string> &files, char *buffer, item memory_size, const string &out_file, item item_size) {
    vector<input_chunk> inputs;

    item block_size = floor((double)memory_size / (files.size() + 1));
    block_size = floor(block_size / 8) * 8;

    item total_size = 0;
    for (int i = 0; i < files.size(); ++i) {
        inputs.push_back(input_chunk(files[i], buffer + block_size * i, block_size));
        total_size += inputs[i].file_size;
    }

    output_chunk output(out_file, buffer + block_size * files.size(), block_size, total_size);


    while (!inputs.empty()) {

        auto min = inputs.begin();
        item min_value = LONG_LONG_MAX;
        bool found_min = false;
        for (auto input = inputs.begin(); input != inputs.end() && !inputs.empty();) {
            if (input->hasNext()) {
                item value = input->next();
#ifdef __PROFILE__
                cout << value << endl;
#endif
                if (value <= min_value) {
                    min_value = value;
                    min = input;
                    found_min = true;
                }
                ++input;
            } else {
                inputs.erase(input);
            }
        }

        if (found_min) {
            output.push(min_value);
            min->move();
        }

    }

    output.close();
}

void partitial_sort(ifstream &input, char *block, item block_size, vector<string> &files, item item_size) {
    item size = 0;
    input.read((char *)&size, item_size);
    size *= item_size;

    item size_offset = item_size;

    item blocks_count = ceil((double)size / block_size);
    for (int i = 0; i < blocks_count; ++i) {
        item offset = size_offset + block_size * i;
        item bytes_to_read = size - block_size * i > block_size ? block_size : size - block_size * i;
        input.seekg(offset);
        input.read(block, bytes_to_read);

        sort((item*)block, (item*)block + bytes_to_read / item_size);
        string out_file = "output_" + to_string(i);
        ofstream file(out_file, ios::binary);
        item items_count = floor(bytes_to_read / item_size);
        file.write((char *)&(items_count), 8);
        file.write(block, bytes_to_read);
        file.close();

        files.push_back(out_file);
    }
}

int main() {
#ifdef __PROFILE__
    gen_test(5);
#endif
    //item memory_size = 500000 - 1000;
    item memory_size = 30000*8;
    char *buffer = new char[memory_size];
    vector<string> files;
    ifstream in("input.bin", ios::binary);

    partitial_sort(in, buffer, memory_size, files, 8);
    external_merge(files, buffer, memory_size, "output.bin", 8);

    return 0;
}
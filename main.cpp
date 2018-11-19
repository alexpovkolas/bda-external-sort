#include <iostream>
#include <vector>
#include <fstream>
#include <math.h>
#include <algorithm>

using namespace std;

#define __PROFILE__

#ifdef __PROFILE__


#include <chrono>

#endif


typedef unsigned long long item;

#ifdef __PROFILE__

void gen_test(item n) {

    ofstream file("input.bin", ios::binary);
    file.write((char *)&n, 8);

    for (item i = n; i > 0; i--){
        file.write((char *)&i, 8);
    }

    file.close();
}

#endif

struct input_chunk {
    ifstream *in;
    item *block;
    size_t file_size;
    size_t file_offset;
    size_t buffer_offset;
    size_t block_size;
    size_t chunk_count;
    size_t read_size;

    input_chunk(const string& file, char* block, size_t block_size) {
        in = new ifstream(file, ios::binary);
        this->block = (item *)block;
        in->read((char *)&(file_size), 8);
        file_offset = 8;
        block_size = block_size;
        chunk_count = 0;

        in->seekg(file_offset);
        read_size = get_next_read_size();
        in->read(block, read_size);
        file_offset += read_size;
        chunk_count++;
    }

    size_t get_next_read_size() {
        return file_size - chunk_count * block_size >= block_size ? block_size : file_size - chunk_count * block_size;
    }

    inline bool hasNext() {
        if (buffer_offset < read_size) {
            return true;
        } else if (file_offset == file_size + 8) {
            return false;
        } else {
            read_size = get_next_read_size();
            file_offset += read_size;
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

    inline item move(){
        return buffer_offset++;
    }
};

struct output_chunk {

    ofstream *out;
    item *block;
    size_t size;
    size_t block_size;
    size_t file_offset;
    size_t buffer_offset;

    output_chunk(const string& file, char* block, size_t block_size, size_t total_size) {
        out = new ofstream(file, ios::binary);
        this->block = (item*)block;
        this->block_size = block_size;
        out->write((char *)&total_size, 8);
        file_offset = 8;
        buffer_offset = 0;
    }

    inline void push(item next){
        if (buffer_offset < block_size) {
            block[buffer_offset++] = next;
        } else {
            out->seekp(file_offset);
            out->write((char*)buffer_offset, block_size);
            buffer_offset = 0;
            file_offset += block_size;
        }
    }

    void close() {
        out->seekp(file_offset);
        out->write((char*)buffer_offset, buffer_offset);
        out->close();
    }
};

void external_merge(vector<string> &files, char *buffer, size_t memory_size, const string &out_file) {
    vector<input_chunk> inputs;

    size_t block_size = floor((double)memory_size / (files.size() + 1));

    size_t total_size = 0;
    for (int i = 0; i < files.size(); ++i) {
        inputs.push_back(input_chunk(files[i], buffer + block_size * i, block_size));
        total_size += inputs[i].file_size;
    }

    output_chunk output(out_file, buffer + block_size * files.size(), block_size, total_size);


    while (!inputs.empty()) {
        item min_value;
        auto min = inputs.begin();
        for (auto input = inputs.begin(); input != inputs.end();) {
            if (input->hasNext()) {
                item value = input->next();
                if (value < min_value) {
                    min_value = value;
                    min = input;
                }
                ++input;
            } else {
                inputs.erase(input);
            }
        }

        output.push(min_value);
        min->move();
    }

    output.close();
}

void partitial_sort(ifstream &input, char *block, size_t block_size, vector<string> &files) {
    item size = 0;
    input.read((char *)&size, 8);

    size_t size_offset = 8;

    size_t blocks_count = ceil((double)size / block_size);
    for (int i = 0; i < blocks_count; ++i) {
        size_t offset = size_offset + block_size * i;
        size_t bytes_to_read = size - block_size * i > block_size ? block_size : size - block_size * i;
        input.seekg(offset);
        input.read(block, bytes_to_read);

        sort(block, block + bytes_to_read);
        string out_file = "output_" + to_string(i);
        ofstream file(out_file, ios::binary);
        file.write((char *)&bytes_to_read, 8);
        file.write(block, bytes_to_read);
        file.close();

        files.push_back(out_file);
    }
}

int main() {
#ifdef __PROFILE__
    gen_test(100);
#endif
    //size_t memory_size = 500000 - 1000;
    size_t memory_size = 30;
    char *buffer = new char[memory_size];
    vector<string> files;
    ifstream in("input.bin", ios::binary);

    partitial_sort(in, buffer, memory_size, files);
    external_merge(files, buffer, memory_size, "output.bin");

    return 0;
}
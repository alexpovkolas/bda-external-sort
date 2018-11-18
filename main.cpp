#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

#define __PROFILE__

#ifdef __PROFILE__

#include <algorithm>
#include <chrono>

#endif


#ifdef __PROFILE__

void gen_test(int n, int m) {

    ofstream file("input.bin", ios::binary);
    file.write((char *)&n, 4);
    file.write((char *)&m, 4);

    for (int i = 1; i <= n * m; i++){
        file.write((char *)&i, 1);
    }

    file.close();
}

bool compare_files(const string& filename1, const string& filename2)
{
    ifstream file1(filename1, ifstream::ate | ifstream::binary);
    std::ifstream file2(filename2, std::ifstream::ate | std::ifstream::binary);
    const ifstream::pos_type fileSize = file1.tellg();

    if (fileSize != file2.tellg()) {
        return false; //different file size
    }

    file1.seekg(0);
    file2.seekg(0);

    istreambuf_iterator<char> begin1(file1);
    istreambuf_iterator<char> begin2(file2);

    return equal(begin1, istreambuf_iterator<char>(),begin2); //Second argument is end-of-range iterator
}

#endif
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
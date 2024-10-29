#include <iostream>
#include <string>
#include <string_view>

void reverse_complement(const char* seq, char* rc, int length) {
    static char trans[256];
    trans['A'] = 'T';
    trans['T'] = 'A';
    trans['C'] = 'G';
    trans['G'] = 'C';

    for (int i = 0; i < length; ++i)
        rc[i] = trans[seq[length - 1 - i]];
    rc[length] = '\0';  // 确保字符串以空字符结尾
}

std::string reverse_complement(std::string_view seq) {
    std::string rc;
    for (auto it = seq.rbegin(); it != seq.rend(); ++it) {
        switch (*it) {
            case 'A':
                rc += 'T';
                break;
            case 'T':
                rc += 'A';
                break;
            case 'C':
                rc += 'G';
                break;
            case 'G':
                rc += 'C';
                break;
        }
    }
    return rc;
}
int main() {
    char seq[] = "TTTCTGCTTAGTTTTGTTGTTGCATCTTCAAACAGCAGCGTTT";
    char rc[100];
    std::string rc2;

    reverse_complement(seq, rc, 43);
    rc2 = reverse_complement("TTTCTGCTTAGTTTTGTTGTTGCATCTTCAAACAGCAGCGTTT");

    std::cout << (rc == rc2) << std::endl;
    std::cout << rc << std::endl;
    std::cout << rc2 << std::endl;
    return 0;
}
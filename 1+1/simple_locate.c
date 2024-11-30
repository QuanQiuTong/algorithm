// 简单找一下可能的起始位置
#include <stdio.h>
#include <windows.h>

void simple_locate(const char* pFileData, DWORD fileSizeLow) {
    const char* p = pFileData;
    const char* end = pFileData + fileSizeLow;

    const char* s="TTCATCCCAGC";

    size_t len_s = strlen(s); // 子串的长度
    size_t len_output = 40; // 每次输出的字符数

    // 遍历主字符串，查找所有子串匹配位置
    for (size_t i = 0; i < fileSizeLow - len_s; i++) {
        //printf("#");
        // 检查当前位置是否匹配目标子串
        if (strncmp(&pFileData[i], s, len_s) == 0) {
            printf("Found match at index %zu: ", i);
            // 输出从当前位置开始的50个字符（考虑边界）
            for (size_t j = i; j < i + len_output && j < fileSizeLow; j++) {
                putchar(pFileData[j]);
            }
            printf("\n");
        }
    }
}

int main() {
    const char* input_filename = "chr6.fasta";
    HANDLE hFile = CreateFile(input_filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        perror("Cannot open file");
        return 1;
    }

    DWORD fileSizeHigh;
    DWORD fileSizeLow = GetFileSize(hFile, &fileSizeHigh);
    if (fileSizeLow == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
        perror("Cannot get file size");
        CloseHandle(hFile);
        return 1;
    }
    if (fileSizeHigh != 0) {
        fprintf(stderr, "File size is too large\n");
        CloseHandle(hFile);
        return 1;
    }

    HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMapFile == NULL) {
        perror("Cannot create file mapping");
        CloseHandle(hFile);
        return 1;
    }

    char* pFileData = (char*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (pFileData == NULL) {
        perror("Cannot map view of file");
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // 此处可以直接操作内存中的文件数据，进行分割和处理
    printf("length = %lu\n",fileSizeLow);
    simple_locate(pFileData, fileSizeLow);

    // 处理完成后，释放资源
    UnmapViewOfFile(pFileData);
    CloseHandle(hMapFile);
    CloseHandle(hFile);

    return 0;
}
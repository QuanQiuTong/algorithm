// 简单找一下可能的起始位置
#include <stdio.h>
#include <windows.h>

void extract_substring(const char* pFileData, DWORD fileSizeLow) {
    FILE* fp = fopen("chr6_substring_2.fasta", "w");
    if (fp == NULL) {
        perror("Cannot open file for writing");
        return;
    }

    const char* p = pFileData+160600000;
    const char* end = p+60000;

    fwrite(p, 1, 60000, fp);
    
    fclose(fp);
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
    extract_substring(pFileData, fileSizeLow);

    // 处理完成后，释放资源
    UnmapViewOfFile(pFileData);
    CloseHandle(hMapFile);
    CloseHandle(hFile);

    return 0;
}
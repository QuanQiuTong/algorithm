#include <stdio.h>
#include <windows.h>

void purify(const char* pFileData, DWORD fileSizeLow) {
    FILE* fp = fopen("chr6.fasta", "w");
    if (fp == NULL) {
        perror("Cannot open file for writing");
        return;
    }

    const char* p = pFileData;
    const char* end = pFileData + fileSizeLow;
    while (p < end && *p != '\n' && *p != '\r') {
        ++p;
    }

    while (p < end) {
        const char* q = p + 1;
        while (q < end && *q != '\n' && *q != '\r') {
            ++q;
        }
        fwrite(p, 1, q - p, fp);  // 不包括换行符

        p = q;
        while (p < end && (*p == '\n' || *p == '\r')) {
            ++p;
        }
    }

    fclose(fp);
}

int main() {
    const char* input_filename = "../chr/chr6  AC_CM000668.2  gi_568336018  LN_170805979  rl_Chromosome  M5_5691468a67c7e7a7b5f2a3a683792c29  AS_GRCh38.fasta";
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
    purify(pFileData, fileSizeLow);

    // 处理完成后，释放资源
    UnmapViewOfFile(pFileData);
    CloseHandle(hMapFile);
    CloseHandle(hFile);

    return 0;
}
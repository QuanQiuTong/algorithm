// divide fasta into chromosomes

#include <stdio.h>
#include <windows.h>

char *get_chr_name(char *p, char *q)
{
    char *chr = p + 1;
    char *filename = (char *)malloc(6 + q - chr + 6) + 6;
    if (filename == NULL)
    {
        perror("Memory allocation failed");
        exit(1);
    }
    memcpy(filename, chr, q - chr);
    // remove illegal characters in filename
    for (int i = 0; i < q - chr; i++)
    {
        if (filename[i] == '/' || filename[i] == '\\' || filename[i] == ':' || filename[i] == '*' || filename[i] == '?' || filename[i] == '"' || filename[i] == '<' || filename[i] == '>' || filename[i] == '|')
            filename[i] = '_';
    }
    {
        filename[-6] = '.';
        filename[-5] = '/';
        filename[-4] = 'c';
        filename[-3] = 'h';
        filename[-2] = 'r';
        filename[-1] = '/';
        filename[q - chr] = '.';
        filename[q - chr + 1] = 'f';
        filename[q - chr + 2] = 'a';
        filename[q - chr + 3] = 's';
        filename[q - chr + 4] = 't';
        filename[q - chr + 5] = 'a';
        filename[q - chr + 6] = '\0';
    }
    return filename - 6;
}

int devide_fasta(char *pFileData, DWORD fileSizeLow)
{
    char *p = pFileData;
    char *end = pFileData + fileSizeLow;
    char *q = p;
    char *chr;
    char *filename = NULL;
    char *lastp = p;
    while (p < end)
    {
        if (*p == '>')
        {
            if (filename != NULL)
            {
                FILE *fp = fopen(filename, "w");
                if (fp == NULL)
                {
                    fprintf(stderr, "Cannot open file %s\n", filename);
                    return 1;
                }
                fwrite(lastp, 1, p - lastp, fp);
                fclose(fp);

                free(filename);
            }
            q = p + 1;
            while (q < end && *q != '\n')
            {
                q++;
            }
            filename = get_chr_name(p, q);
            lastp = p;
            p = q;
        }
        else
        {
            p++;
        }
    }
    return 0;
}

int main()
{
    const char *input_filename = "GCA_000001405.15_GRCh38_no_alt_analysis_set.fasta";
    HANDLE hFile = CreateFile(input_filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        perror("Cannot open file");
        return 1;
    }

    DWORD fileSizeHigh;
    DWORD fileSizeLow = GetFileSize(hFile, &fileSizeHigh);
    if (fileSizeLow == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
    {
        perror("Cannot get file size");
        CloseHandle(hFile);
        return 1;
    }
    if (fileSizeHigh != 0)
    {
        fprintf(stderr, "File size is too large\n");
        CloseHandle(hFile);
        return 1;
    }

    HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMapFile == NULL)
    {
        perror("Cannot create file mapping");
        CloseHandle(hFile);
        return 1;
    }

    char *pFileData = (char *)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (pFileData == NULL)
    {
        perror("Cannot map view of file");
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // 此处可以直接操作内存中的文件数据，进行分割和处理
    system("mkdir chr");
    int v = devide_fasta(pFileData, fileSizeLow);
    if (v == 1)
    {
        perror("Cannot divide fasta");
        UnmapViewOfFile(pFileData);
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // 处理完成后，释放资源
    UnmapViewOfFile(pFileData);
    CloseHandle(hMapFile);
    CloseHandle(hFile);

    return 0;
}
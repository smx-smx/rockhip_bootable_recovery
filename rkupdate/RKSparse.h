/*************************************************************************
	> File Name: RKSparse.h
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Mon 21 Aug 2017 09:50:25 AM CST
 ************************************************************************/

#ifndef _RKSPARSE_H
#define _RKSPARSE_H
#include "DefineHeader.h"
#include "RKComm.h"
#define LBA_TRANSFER_SIZE 16*1024
class RKSparse{
public:
    sparse_header m_header;
    chunk_header m_chunk;
    FILE *m_pFile;
    const char *fileName;
    RKSparse(const char *filePath);
    ~RKSparse();
    bool IsSparseImage();
    long long GetSparseImageSize();
    bool SparseFile_Download(DWORD dwPos, CRKComm *pComm);
    //bool SparseFile_Check();
    void display();
private:
    bool readfile(long long dwOffset, DWORD dwSize, PBYTE lpBuffer);
    //bool writefile();
};
#endif

#ifndef __XMI_XMI_H
#define __XMI_XMI_H

std::vector<unsigned char*> TranscodeXmiToMid(const unsigned char* pXmiData, size_t iXmiLength, std::vector<size_t>& midLengths);

#endif

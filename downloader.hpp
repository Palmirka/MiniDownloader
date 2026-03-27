#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <curl/curl.h>

#include "threadPool.hpp"

class MiniDownloader 
{
    std::string url;
    std::string fileName;
    bool partialDownloadAllowed;
    size_t partSize;
    ThreadPool* threadPool;

    bool partialDownload(int part);
    void concatFiles(int part);

public:
    MiniDownloader(std::string url, std::string fileName);
    void run();
    ~MiniDownloader();
};

void createDirectory(std::string filename);
size_t saveToFile(char* data, size_t size, size_t nmemb, void* file);
std::string prepareFileName(std::string fileName, int part);
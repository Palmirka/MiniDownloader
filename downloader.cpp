#include "downloader.hpp"

MiniDownloader::MiniDownloader (std::string url, std::string fileName) : 
    url(url), 
    fileName(fileName), 
    partialDownloadAllowed(false),
    partSize(2 * 1024 * 1024),
    threadPool(new ThreadPool((size_t)4))
{
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, saveToFile);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
}

bool MiniDownloader::partialDownload(int part)
{
    std::string partialFileName = prepareFileName(fileName, part);
    std::string range = std::to_string(part * partSize) + "-" + std::to_string((part + 1) * partSize -1);
     
    FILE* file = fopen(partialFileName.c_str(), "wb");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str());
    CURLcode result = curl_easy_perform(curl);
    fclose(file);

    curl_off_t receivedBytes = 0;
    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &receivedBytes);

    return receivedBytes < partSize;
}

void MiniDownloader::concatFiles(int part)
{
    char buffer[4096];
    size_t bytesRead = 0;
    FILE* file = fopen(fileName.c_str(), "wb");
    for (int i = 0; i < part; i++)
    {
        std::string partialFileName = prepareFileName(fileName, i);
        FILE* partialFile = fopen(partialFileName.c_str(), "rb");
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), partialFile)) > 0) 
        {
            fwrite(buffer, 1, bytesRead, file);
        }
        fclose(partialFile);
        remove(partialFileName.c_str());
    }
    fclose(file);
}

void MiniDownloader::run()
{ 
    createDirectory(fileName);
    int part = 0;
    curl_off_t receivedBytes = 0;
    while (!threadPool->end) 
    {
        threadPool->emplace([&, part] () {
            return partialDownload(part);
        });
        part++;
    }
    threadPool->join();
    concatFiles(part);
}

void createDirectory(std::string fileName)
{
    std::string directory = fileName.substr(0, fileName.find('.'));
    std::filesystem::create_directory("Download");
    std::filesystem::current_path(std::string("Download/"));
    std::filesystem::create_directory(directory);
    std::filesystem::current_path(directory);
}

size_t saveToFile(char* data, size_t size, size_t nmemb, void* file) 
{
    return fwrite(data, size, nmemb, (FILE*)file);
}

std::string prepareFileName(std::string fileName, int part)
{
    std::string suffix = "(" + std::to_string(part) + ")";
    auto dotIdx = fileName.find('.');
    if (dotIdx != std::string::npos)
    {
        return fileName.substr(0, dotIdx) + suffix + fileName.substr(dotIdx);
    }
    return fileName + suffix;
}

MiniDownloader::~MiniDownloader()
{
    delete threadPool;
}
#include <iostream>
#include "downloader.hpp"

// g++ main.cpp downloader.cpp threadPool.cpp -o test -I C:\Users\kama\Downloads\curl-8.19.0_4-win64-mingw\include -L C:\Users\kama\Downloads\curl-8.19.0_4-win64-mingw\lib -lcurl
// ./test https://www.stats.govt.nz/assets/Uploads/Annual-enterprise-survey/Annual-enterprise-survey-2024-financial-year-provisional/Download-data/annual-enterprise-survey-2024-financial-year-provisional.csv xd.csv
int main(int argc, char** argv)
{
    if(argc != 3){
        std::cout << "Incorrect number of arguments" << std::endl;
        return 0;
    }
    MiniDownloader md = MiniDownloader(argv[1], argv[2]);
    md.run();
    return 0;
}
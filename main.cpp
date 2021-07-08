#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include<CRC.h>

#include <iomanip>  // Includes ::std::hex
#include <iostream> // Includes ::std::cout
#include <cstdint>  // Includes ::std::uint32_t
#include<vector>
#include<cstring>

void generateRandomMessage(std::vector<unsigned int>& data, int size){
    for(int i = 0; i < size; i++){
        data[i] = rand();
    }
}

void addRandomNoise(const std::vector<unsigned int>& in, std::vector<unsigned int>& out){
    for(int i = 0; i < in.size(); i++){
        out[i] = in[i] -  rand() % 10 == 0 ? rand() : 0;
    }
}

uint32_t compute32Xor(const std::vector<unsigned int>& data){
    uint32_t checksum = data[0];
        for(int i = 1; i < data.size(); i++){
        checksum  ^= data[i];
    }
    return checksum;
}

uint32_t compute8Xor(const std::vector<unsigned int>& data){
    uint8_t* p = (uint8_t*)data.data();
    uint8_t checksum = p[0];
    for(int i = 1; i < data.size() * 4; i++){
        checksum ^= p[i];
    }
    return checksum & 0x7E;
}

int main(int argc, const char** argv){

    int samples = 5000000;
    int nrCollision = 0;

    std::vector<unsigned int> data(10);
    std::vector<unsigned int> noise(10);
    srand(time(NULL));

    for(int i = 0; i < samples; i++){
        generateRandomMessage(data, 10);
        addRandomNoise(data, noise);

        std::uint32_t originalCRC = CRC::Calculate(data.data(), data.size() * sizeof(unsigned int), CRC::CRC_8());
        std::uint32_t noiseCRC = CRC::Calculate(noise.data(), noise.size()  * sizeof(unsigned int), CRC::CRC_8());	

    //    std::uint32_t originalCRC =  compute8Xor(data);
    //     std::uint32_t noiseCRC = compute8Xor(noise);

        if(!std::strncmp((const char*)data.data(), (const char*)noise.data(), data.size() * 4) && originalCRC == noiseCRC){

            nrCollision++;
        }

	
        // std::cout << std::hex << originalCRC << std::endl;
        // std::cout << std::hex << noiseCRC << std::endl;
    }
    printf("CRC8 collision %d, %f!\n", nrCollision, (float)nrCollision / (float)samples);
}
#include <iostream>
#include <fstream> 
#include <vector>
#include <algorithm>

enum Axis{
    x,
    y,
    z
};

struct Image{
    
    int dimX;
    int dimY;
    int dimZ;
    std::vector<unsigned int> data;

    Image(){
        dimX = 0;
        dimY = 0;
        dimZ = 0;
        data = std::vector<unsigned int>();
    }
    Image( int idimX , int idimY , int idimZ){
        dimX = idimX;
        dimY = idimY;
        dimZ = idimZ;
        data = std::vector<unsigned int>(dimX * dimY * dimZ);
    }

    unsigned int getValue(int i, int j, int k) const{
        return data[k * (dimY * dimX) + j * dimX + i];
    }
    
    unsigned int getMaxValue() const {
        unsigned int maxVal = 0;
        for (const auto& val : data) {
            if (val > maxVal) {
                maxVal = val;
            }
        }
        return maxVal;
    }

    unsigned int getMinValue() const {
        unsigned int minVal = 0;
        for (const auto& val : data) {
            if (val < minVal) {
                minVal = val;
            }
        }
        return minVal;
    }
};

Image read_img_file(const char* filename, int dimX, int dimY, int dimZ) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Open broke " << filename << std::endl;
        return Image();
    }

    Image img(dimX, dimY, dimZ);
    int totalSize = dimX * dimY * dimZ;
    img.data.resize(totalSize);

    std::vector<uint8_t> buffer(totalSize * 2);
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    for (size_t i = 0; i < buffer.size(); i += 2) {
        img.data[i/2] = (unsigned short) (buffer[i]) * 256 + buffer[i + 1];
    }

    file.close();
    std::cout << "Image data successfully read from " << filename << std::endl;
    return img;
}

void saveHistogramData(Image & im, std::string name, int sX, int sY, int sZ){

    const char * nom_image = name.c_str();
    FILE *f_data;

    if( (f_data = fopen(nom_image, "wb")) == NULL){
        printf("\n[Export Histogramme] Pas d'acces en ecriture sur le fichier %s \n", nom_image);
        exit(EXIT_FAILURE);
    }

    int min = im.getMinValue();
    int max = im.getMaxValue();

    std::vector< unsigned int > count(max - min, 0);
    std::cout << "SIZE " << max - min << std::endl;

    for(int i = 0 ; i < sX * sY * sZ ; ++i){

        auto v = (uint16_t)im.data[i];

        count[v-min]++;
    }


    for(int i = 0 ; i < count.size() ; ++i){
        
        fprintf(f_data,
            "%d %d\n",
            i-min,
            count[i-min]
        );
    }

    fclose(f_data);
}
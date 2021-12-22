#pragma once

class Texture
{
    int width, height, nbChannels;
    unsigned char* pixels = nullptr; 

public:
    Texture(char const* path);
    Texture(int width, int height, int nbChannels);
    ~Texture();

    int getWidth() { return width; }
    int getHeight() { return height; }
    int getNbChannels() { return nbChannels; }
    unsigned char* getPixels() { return pixels; }
};
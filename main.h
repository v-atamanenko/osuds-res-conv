#include <stdio.h>
#include <string>
#include <sys/stat.h>

#include <SDL.h>
#include <SDL_image.h>

// Reading files one byte per read.
#define BUFFERSIZE 1

// Screen size, any number since nothing is actually rendered on the screen.
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// Textures with RGB4 palette are 2 bits per pixel
struct pxInByte_2 {
	unsigned int pixel1 : 2;
    unsigned int pixel2 : 2;
    unsigned int pixel3 : 2;
    unsigned int pixel4 : 2;
};

// Textures with RGB16 palette are 4 bits per pixel
struct pxInByte_4 {
	unsigned int pixel1 : 4;
	unsigned int pixel2 : 4;
};

// Start up SDL/SDL_image, create window and renderer.
bool init();

// Shut down SDL/SDL_image.
void close();

SDL_Texture* load(std::string path, SDL_Color* pal, size_t pals, int custom_width);
void save(SDL_Texture *tex, const char *filename);

long GetFileSize(const std::string& filename);

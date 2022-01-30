#include "main.h"
#include "palette.cpp"

// SDL globals.
SDL_Window* gWindow = nullptr;
SDL_Renderer *gRenderer = nullptr;

long GetFileSize(const std::string& filename) {
    struct stat stat_buf{};
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL failed to initialize: %s\n", SDL_GetError());
        return false;
    }

    gWindow = SDL_CreateWindow("osuds-res-conv", SDL_WINDOWPOS_UNDEFINED,
    						   SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
    						   SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
        fprintf(stderr, "Window failed to create: %s\n", SDL_GetError());
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
    if ( gRenderer == nullptr ) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "SDL_image failed to initialize: %s\n", IMG_GetError());
        return false;
    }

    return true;
}

void close() {
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;

    IMG_Quit();
    SDL_Quit();
}

uint8_t * indexed4ToNormal24Bit(pxInByte_2* pixels_indexed, SDL_Color* pal, int w, int h, long fsize) {
	// Since SDL doesn't have the built-in capabality to work with RGB4/RGB16
    // indexed images, here we manually loop over the read pixel data and 
    // map the numbers in the texture to the provided palette to build a normal
    // RGB24 surface.
    int bypp = 3; 		 // bytes per pixel
    int bipp = bypp * 8; // bits per pixel
    int pipb = 4;        // pixels per byte

    size_t pixels_24bit_size = w * h * bypp * sizeof(uint8_t);
    auto* pixels_24bit = static_cast<uint8_t *>(malloc(pixels_24bit_size));
    memset(pixels_24bit, 0, pixels_24bit_size);

    for (int u = 0; u<fsize; u++) {
    	int x=0; // index of the pixel inside pixels_indexed[u]
    	unsigned int offset = ( u * bypp * pipb ) + (x * bypp);

    	pixels_24bit[ offset + 0 ] = pal[pixels_indexed[u].pixel1].r;
        pixels_24bit[ offset + 1 ] = pal[pixels_indexed[u].pixel1].g;
        pixels_24bit[ offset + 2 ] = pal[pixels_indexed[u].pixel1].b;
        x++;

        offset = ( u * bypp * pipb ) + (x * bypp);
    	pixels_24bit[ offset + 0 ] = pal[pixels_indexed[u].pixel2].r;
        pixels_24bit[ offset + 1 ] = pal[pixels_indexed[u].pixel2].g;
        pixels_24bit[ offset + 2 ] = pal[pixels_indexed[u].pixel2].b;
        x++;

        offset = ( u * bypp * pipb ) + (x * bypp);
        pixels_24bit[ offset + 0 ] = pal[pixels_indexed[u].pixel3].r;
        pixels_24bit[ offset + 1 ] = pal[pixels_indexed[u].pixel3].g;
        pixels_24bit[ offset + 2 ] = pal[pixels_indexed[u].pixel3].b;
        x++;

        offset = ( u * bypp * pipb ) + (x * bypp);
        pixels_24bit[ offset + 0 ] = pal[pixels_indexed[u].pixel4].r;
        pixels_24bit[ offset + 1 ] = pal[pixels_indexed[u].pixel4].g;
        pixels_24bit[ offset + 2 ] = pal[pixels_indexed[u].pixel4].b;
	}
	free(pixels_indexed);
    return pixels_24bit;
}

uint8_t * indexed16ToNormal24Bit(pxInByte_4* pixels_indexed, SDL_Color* pal, int w, int h, long fsize) {
    // Since SDL doesn't have the built-in capabality to work with RGB4/RGB16
    // indexed images, here we manually loop over the read pixel data and
    // map the numbers in the texture to the provided palette to build a normal
    // RGB24 surface.
    int bypp = 3; 		 // bytes per pixel
    int bipp = bypp * 8; // bits per pixel
    int pipb = 2;        // pixels per byte

    size_t pixels_24bit_size = w * h * bypp * sizeof(uint8_t);
    auto* pixels_24bit = static_cast<uint8_t *>(malloc(pixels_24bit_size));
    memset(pixels_24bit, 0, pixels_24bit_size);

    for (int u = 0; u<fsize; u++) {
        int x=0; // index of the pixel inside pixels_indexed[u]
        unsigned int offset = ( u * bypp * pipb ) + (x * bypp);

        pixels_24bit[ offset + 0 ] = pal[pixels_indexed[u].pixel1].r;
        pixels_24bit[ offset + 1 ] = pal[pixels_indexed[u].pixel1].g;
        pixels_24bit[ offset + 2 ] = pal[pixels_indexed[u].pixel1].b;
        x++;

        offset = ( u * bypp * pipb ) + (x * bypp);
        pixels_24bit[ offset + 0 ] = pal[pixels_indexed[u].pixel2].r;
        pixels_24bit[ offset + 1 ] = pal[pixels_indexed[u].pixel2].g;
        pixels_24bit[ offset + 2 ] = pal[pixels_indexed[u].pixel2].b;
    }
    free(pixels_indexed);
    return pixels_24bit;
}

pxInByte_2* readIndexed4(const std::string& path) {
    long fsize = GetFileSize(path);
    FILE* filp = fopen(path.c_str(), "rb" );
    if (!filp) { fprintf(stderr, "Can't open the file\n"); return nullptr; }

    auto * buffer = new uint8_t[BUFFERSIZE];

    auto *pixels_indexed = static_cast<pxInByte_2 *>(malloc(fsize*4));

    int i = 0;
    while (true) {
        size_t bytes = fread(buffer, sizeof(uint8_t), 1, filp);
        if (bytes > 0) {
            *(uint8_t *) (pixels_indexed + i) = *buffer;
            i++;
        } else {
            break;
        }
    }
    if (i != fsize) {
        fprintf(stderr, "File size vs bytes read mismatch: %li/%i", fsize, i);
        free(pixels_indexed);
        return nullptr;
    }

    printf("%i (%x) bytes read.\n", i, i);
    fclose(filp);

    return pixels_indexed;
}

pxInByte_4* readIndexed16(const std::string& path) {
    long fsize = GetFileSize(path);
    FILE* filp = fopen(path.c_str(), "rb" );
    if (!filp) { fprintf(stderr, "Can't open the file\n"); return nullptr; }

    auto * buffer = new uint8_t[BUFFERSIZE];

    auto *pixels_indexed = static_cast<pxInByte_4 *>(malloc(fsize*4));

    int i = 0;
    while (true) {
        size_t bytes = fread(buffer, sizeof(uint8_t), 1, filp);
        if (bytes > 0) {
            *(uint8_t *) (pixels_indexed + i) = *buffer;
            i++;
        } else {
            break;
        }
    }
    if (i != fsize) {
        fprintf(stderr, "File size vs bytes read mismatch: %li/%i", fsize, i);
        free(pixels_indexed);
        return nullptr;
    }

    printf("%i (%x) bytes read.\n", i, i);
    fclose(filp);

    return pixels_indexed;
}


SDL_Texture* load(std::string path, SDL_Color* pal, size_t pals, int custom_w) {
    int countpixels, w, h, pixelsperbyte;
    long fsize = GetFileSize(path);
    SDL_Texture* newTexture = nullptr;

    switch(pals) {
    	case 4:
    		pixelsperbyte = 4; break; // 2 bits per pixel
    	case 16:
    		pixelsperbyte = 2; break; // 4 bits per pixel
		default:
			fprintf(stderr, "Can't load palette of length %zu\n", pals);
			return nullptr;
    }

    countpixels = fsize * pixelsperbyte;

    if (custom_w > 0) {
    	w = custom_w;
    	h = countpixels / w;
    } else {
    	w = h = sqrt(countpixels);
    }

    uint8_t * pixels_24bit;
    if (pixelsperbyte == 4) {
        pxInByte_2* indexed = readIndexed4(path);
        pixels_24bit = indexed4ToNormal24Bit(indexed, pal, w, h, fsize);
    } else {
        pxInByte_4* indexed = readIndexed16(path);
        pixels_24bit = indexed16ToNormal24Bit(indexed, pal, w, h, fsize);
    }

    int bypp = 3; 		 // bytes per pixel
    int bipp = bypp * 8; // bits per pixel

    uint32_t Rmask, Gmask, Bmask, Amask;
    SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGB24, &bipp, &Rmask, &Gmask, &Bmask, &Amask);
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
								                    pixels_24bit,
								                    w, h,
								                    bipp, bypp*w,
								                    Rmask, Gmask, Bmask, Amask
            									   );

    if (surface == nullptr) {
        fprintf(stderr, "Failed to create surface: %s\n", SDL_GetError());
        return nullptr;
    }

    SDL_Surface* optimizedSurface = nullptr;

    uint32_t pf = SDL_GetWindowPixelFormat(gWindow);
    optimizedSurface = SDL_ConvertSurface( surface, SDL_AllocFormat(pf), 0 );
    if( optimizedSurface == nullptr )
    {
        printf( "Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        SDL_FreeSurface( surface );
        return nullptr;
    }

    SDL_FreeSurface( surface );
    free(pixels_24bit);

    newTexture = SDL_CreateTextureFromSurface( gRenderer, optimizedSurface );
    if( newTexture == nullptr )
    {
        fprintf( stderr, "Unable to create texture for bg! SDL Error: %s\n", SDL_GetError() );
        SDL_FreeSurface( optimizedSurface );
        return nullptr;
    }

    SDL_FreeSurface( optimizedSurface );
    return newTexture;
}

void save(SDL_Texture *tex, const char *filename) {
    SDL_Texture *ren_tex;
    SDL_Surface *surf;
    int st;
    int w;
    int h;
    int format;
    void *pixels;

    pixels  = nullptr;
    surf    = nullptr;
    ren_tex = nullptr;
    format  = SDL_PIXELFORMAT_RGBA32;

    /* Get information about texture we want to save */
    st = SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
    if (st != 0) {
        SDL_Log("Failed querying texture: %s\n", SDL_GetError());
        goto cleanup;
    }

    ren_tex = SDL_CreateTexture(gRenderer, format, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!ren_tex) {
        SDL_Log("Failed creating render texture: %s\n", SDL_GetError());
        goto cleanup;
    }

    /*
     * Initialize our canvas, then copy texture to a target whose pixel data we 
     * can access
     */
    st = SDL_SetRenderTarget(gRenderer, ren_tex);
    if (st != 0) {
        SDL_Log("Failed setting render target: %s\n", SDL_GetError());
        goto cleanup;
    }

    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(gRenderer);

    st = SDL_RenderCopy(gRenderer, tex, nullptr, nullptr);
    if (st != 0) {
        SDL_Log("Failed copying texture data: %s\n", SDL_GetError());
        goto cleanup;
    }

    /* Create buffer to hold texture data and load it */
    pixels = malloc(w * h * SDL_BYTESPERPIXEL(format));
    if (!pixels) {
        SDL_Log("Failed allocating memory\n");
        goto cleanup;
    }

    st = SDL_RenderReadPixels(gRenderer, nullptr, format, pixels, w * SDL_BYTESPERPIXEL(format));
    if (st != 0) {
        SDL_Log("Failed reading pixel data: %s\n", SDL_GetError());
        goto cleanup;
    }

    /* Copy pixel data over to surface */
    surf = SDL_CreateRGBSurfaceWithFormatFrom(pixels, w, h, SDL_BITSPERPIXEL(format), w * SDL_BYTESPERPIXEL(format), format);
    if (!surf) {
        SDL_Log("Failed creating new surface: %s\n", SDL_GetError());
        goto cleanup;
    }

    /* Save result to an image */
    //st = SDL_SaveBMP(surf, filename);
    st = IMG_SavePNG(surf, filename);
    if (st != 0) {
        SDL_Log("Failed saving image: %s\n", SDL_GetError());
        goto cleanup;
    }

    SDL_Log("Saved texture as BMP to \"%s\"\n", filename);

    cleanup:
    SDL_FreeSurface(surf);
    free(pixels);
    SDL_DestroyTexture(ren_tex);
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		printf("osuds-res-conv. usage:\n");
		printf("osuds-res-conv [path-to-bin-texture] [output-file.bmp] ");
		printf("[palette-id(0-8)] {custom-width(optional)}\n");
		return 0;
	}

    if( !init() ) {
        fprintf(stderr, "Failed to initialize!\n");
        close(); return 1;
    }

    SDL_Color* pal;
    size_t pal_size;
    int custom_width;

    switch (atoi(argv[3])) {
    	case 0:
    		pal = palette0;
    		pal_size = sizeof(palette0) / sizeof(palette0[0]);
    		break;
    	case 1:
    		pal = palette1;
    		pal_size = sizeof(palette1) / sizeof(palette1[0]);
    		break;
    	case 2:
    		pal = palette2;
    		pal_size = sizeof(palette2) / sizeof(palette2[0]);
    		break;
    	case 3:
    		pal = palette3;
    		pal_size = sizeof(palette3) / sizeof(palette3[0]);
    		break;
    	case 4:
    		pal = palette4;
    		pal_size = sizeof(palette4) / sizeof(palette4[0]);
    		break;
    	case 5:
    		pal = palette5;
    		pal_size = sizeof(palette5) / sizeof(palette5[0]);
    		break;
    	case 6:
    		pal = palette6;
    		pal_size = sizeof(palette6) / sizeof(palette6[0]);
    		break;
    	case 7:
    		pal = palette7;
    		pal_size = sizeof(palette7) / sizeof(palette7[0]);
    		break;
    	case 8:
    		pal = palette8;
    		pal_size = sizeof(palette8) / sizeof(palette8[0]);
    		break;
    	default:
    		fprintf(stderr, "Unknown palette id %i\n", atoi(argv[3]));
    		close(); return 1;
    }    

    if (argc >= 5) {
    	custom_width = atoi(argv[4]);
    } else {
    	custom_width = 0;
    }

    SDL_Texture* imp = load(argv[1], pal, pal_size, custom_width);
    save(imp, argv[2]);
    SDL_DestroyTexture(imp);

    close();
	return 0;
}
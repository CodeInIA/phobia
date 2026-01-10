#include "Texture.h"

//------------------------
// Creates the texture
//------------------------
void Texture::initTexture(const char *textureFile) {
    
 // Create the texture to configure
    glGenTextures(1,&texture);  
    glBindTexture(GL_TEXTURE_2D, texture);
    
 // Load the image
    unsigned int  w, h;
    unsigned char *pixels = loadTexture(textureFile, w, h);  
    
 // Create the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    
 // Set texture parameters
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float aniso;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

}

//--------------------------------------------------
// Loads a texture using the FreeImage library
//--------------------------------------------------
unsigned char* Texture::loadTexture(const char *textureFile, unsigned int &w, unsigned int &h) {
    
 // Read the texture file
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(textureFile,0);
    if(format==FIF_UNKNOWN) format = FreeImage_GetFIFFromFilename(textureFile);
    if((format==FIF_UNKNOWN) || !FreeImage_FIFSupportsReading(format)) {
        std::cout << "Texture format for file " << textureFile << " is not supported." << std::endl;
        std::cin.get();
        exit(1);
    }
    FIBITMAP *texture = FreeImage_Load(format,textureFile);
    if(texture==NULL) {
        std::cout << "El fichero " << textureFile << " no se puede abrir." << std::endl;
        std::cin.get();
        exit(1);
    }
    FIBITMAP *temp = texture;
    texture = FreeImage_ConvertTo32Bits(texture);
    FreeImage_Unload(temp);

 // Convert from BGRA to RGBA
    w = FreeImage_GetWidth(texture);
    h = FreeImage_GetHeight(texture);
    unsigned char *pixelsBGRA = (unsigned char *)FreeImage_GetBits(texture);
    unsigned char *pixelsRGBA = new unsigned char[4*w*h];
    for(int j=0; j<w*h; j++){
        pixelsRGBA[j*4+0] = pixelsBGRA[j*4+2];
        pixelsRGBA[j*4+1] = pixelsBGRA[j*4+1];
        pixelsRGBA[j*4+2] = pixelsBGRA[j*4+0];
        pixelsRGBA[j*4+3] = pixelsBGRA[j*4+3];
    }

    FreeImage_Unload(texture);

    return pixelsRGBA;
}

//-----------------------------------------
// Returns the texture identifier
//-----------------------------------------
unsigned int Texture::getTexture() {
    
    return texture;
    
}

//-----------------------
// Class destructor
//-----------------------
Texture::~Texture() {
    
    glDeleteTextures(1,&texture);
    
}

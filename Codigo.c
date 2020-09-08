#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <io.h>
#include <time.h>


char filename[100];
int choisenOption;

#define NULL 0
#define HEADER_SIZE 54
#define FILE_HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define BYTES_PER_PIXEL 3
#define DEFAULT_PATH "images/"
#define NORMAL  "\x1B[0m"
#define YELLOW  "\x1B[33m"

typedef unsigned char BYTE; 
typedef unsigned short WORD; // 2 Bytes
typedef unsigned long DWORD; // 4 Bytes

typedef struct PIXEL{
    BYTE blue, green, red;
} PIXEL;

const PIXEL paddPixel = {0,0,0};

#pragma pack(push, 1)

typedef struct BITMAPFILEHEADER{
    WORD bfType;  
    DWORD bfSize; 
    WORD bfReserved1; 
    WORD bfReserved2; 
    DWORD bfOffBits; 
} BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER{
    DWORD biSize;   
    DWORD biWidth;  
    DWORD biHeight; 
    WORD biPlanes; 
    WORD biBitCount;   
    DWORD biCompression;  
    DWORD biSizeImage;  
    DWORD biXPelsPerMeter; 
    DWORD biYPelsPerMeter;  
    DWORD biClrUsed;    
    DWORD biClrImportant;  
} BITMAPINFOHEADER;

typedef struct IMAGE{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    PIXEL **pixels;
} IMAGE;

#pragma pack(pop)

void delay(int number_of_seconds) // https://www.geeksforgeeks.org/time-delay-c/
{ 
    // Converting time into milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Storing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds); 
} 

int getPadding(int width){
    int padding = 0;
    while((width + padding) % 4){
        padding++;
    }

    return padding;
}

int getSizePixel(){
    return sizeof(struct PIXEL);
}

int combLinear(BYTE blue, BYTE green, BYTE red){
    int result;
    result = 0.11 * blue + 0.59 * green + 0.3 * red;
    
    return result;
}

float convertByteToMegabyte(float byte){
    return (float) byte / pow(2, 20);
}

BITMAPFILEHEADER loadBitmapFileHeader(FILE *file){
    BITMAPFILEHEADER fileHeader;
    fseek(file, 0, SEEK_SET);
    fread(&fileHeader.bfType, sizeof(WORD), 1, file);
    fread(&fileHeader.bfSize, sizeof(DWORD), 1, file);
    fread(&fileHeader.bfReserved1, sizeof(WORD), 1, file);
    fread(&fileHeader.bfReserved2, sizeof(WORD), 1, file);
    fread(&fileHeader.bfOffBits, sizeof(DWORD), 1, file);
    return fileHeader;
}

BITMAPINFOHEADER loadBitmapInfoHeader(FILE *file){
    BITMAPINFOHEADER infoHeader;
    fseek(file, 14, SEEK_SET);
    fread(&infoHeader.biSize, sizeof(DWORD), 1, file);
    fread(&infoHeader.biWidth, sizeof(DWORD), 1, file);
    fread(&infoHeader.biHeight, sizeof(DWORD), 1, file);
    fread(&infoHeader.biPlanes, sizeof(WORD), 1, file);
    fread(&infoHeader.biBitCount, sizeof(WORD), 1, file);
    fread(&infoHeader.biCompression, sizeof(DWORD), 1, file);
    fread(&infoHeader.biSizeImage, sizeof(DWORD), 1, file);
    fread(&infoHeader.biXPelsPerMeter, sizeof(DWORD), 1, file);
    fread(&infoHeader.biYPelsPerMeter, sizeof(DWORD), 1, file);
    fread(&infoHeader.biClrUsed, sizeof(DWORD), 1, file);
    fread(&infoHeader.biClrImportant, sizeof(DWORD), 1, file);

    return infoHeader;

}

IMAGE readImage(FILE *file){
    int width, height, padding, sizePixel, i, j;
    PIXEL **pixels;
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    IMAGE image;
    fileHeader = loadBitmapFileHeader(file);
    infoHeader = loadBitmapInfoHeader(file);
    width = infoHeader.biWidth;
    height = infoHeader.biHeight;
    image.fileHeader = fileHeader;
    image.infoHeader = infoHeader;
    padding = getPadding(width);
    sizePixel = getSizePixel(); 
    pixels = (struct PIXEL **)malloc(height * sizeof(struct PIXEL*));
    for(i = 0; i < height; i++){
        pixels[i] = (struct PIXEL *)malloc(width * sizeof(sizePixel));
    }
    if(pixels == NULL){
        printf("| AVISO | Memória insuficiente.");
        fclose(file);
        exit(1);
    }
    char tempPadding[4];
    for(i = 0; i < height; i++){
        for(j=0; j < width; j++){
            fread(&pixels[i][j], BYTES_PER_PIXEL, 1, file);
        }
        if(padding != 0){
            fread(&tempPadding, BYTES_PER_PIXEL, 1, file);
        }
    }
    image.pixels = pixels;
    return image;
}
IMAGE copy(IMAGE image){

    return image;
}

IMAGE rotate90DegreesRight(IMAGE image){

    IMAGE rotatedImage;
    PIXEL **rotatedPixels, **pixels = image.pixels;

    int width = image.infoHeader.biHeight;
    int height = image.infoHeader.biWidth;
    int sizePixel, i, j;

    sizePixel = getSizePixel();
    
    rotatedPixels = (struct PIXEL **)malloc(height * sizeof(struct PIXEL*));
    for(i = 0; i < height; i++){
        rotatedPixels[i] = (struct PIXEL *)malloc(width * sizeof(sizePixel));
    }

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            rotatedPixels[i][j] = pixels[j][height-1-i];
        }
    }

    rotatedImage.fileHeader = image.fileHeader;

    rotatedImage.infoHeader = image.infoHeader;
    rotatedImage.infoHeader.biWidth = width;
    rotatedImage.infoHeader.biHeight = height;

    rotatedImage.pixels = rotatedPixels;

    return rotatedImage;
}

IMAGE rotate90DegreesLeft(IMAGE image){
    
    IMAGE rotatedImage;
    PIXEL **rotatedPixels, **pixels = image.pixels;

    int width = image.infoHeader.biHeight;
    int height = image.infoHeader.biWidth;
    int sizePixel, i, j;

    sizePixel = getSizePixel();
    
    rotatedPixels = (struct PIXEL **)malloc(height * sizeof(struct PIXEL*));
    for(i = 0; i < height; i++){
        rotatedPixels[i] = (struct PIXEL *)malloc(width * sizeof(sizePixel));
    }

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            rotatedPixels[i][j] = pixels[width-1-j][i];
        }
    }

    rotatedImage.fileHeader = image.fileHeader;

    rotatedImage.infoHeader = image.infoHeader;
    rotatedImage.infoHeader.biWidth = width;
    rotatedImage.infoHeader.biHeight = height;

    rotatedImage.pixels = rotatedPixels;
    
    return rotatedImage;
}

IMAGE rotate180Degrees(IMAGE image){
    
    IMAGE rotatedImage;
    PIXEL **rotatedPixels, **pixels = image.pixels;

    int width = image.infoHeader.biWidth;
    int height = image.infoHeader.biHeight;
    int sizePixel, i, j;

    sizePixel = getSizePixel();
    
    rotatedPixels = (struct PIXEL **)malloc(height * sizeof(struct PIXEL*));
    for(i = 0; i < height; i++){
        rotatedPixels[i] = (struct PIXEL *)malloc(width * sizeof(sizePixel));
    }

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            rotatedPixels[i][j] = pixels[height-1-i][width-1-j];
        }
    }

    rotatedImage.fileHeader = image.fileHeader;

    rotatedImage.infoHeader = image.infoHeader;

    rotatedImage.pixels = rotatedPixels;

    return rotatedImage;
}

IMAGE flipHorizontally(IMAGE image){
    
    IMAGE flippedImage;
    PIXEL **flippedPixels, **pixels = image.pixels;

    int width = image.infoHeader.biWidth;
    int height = image.infoHeader.biHeight;
    int sizePixel, i, j;

    sizePixel = getSizePixel();
    
    flippedPixels = (struct PIXEL **)malloc(height * sizeof(struct PIXEL*));
    for(i = 0; i < height; i++){
        flippedPixels[i] = (struct PIXEL *)malloc(width * sizeof(sizePixel));
    }

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            flippedPixels[i][j] = pixels[i][width-1-j];
        }
    }

    flippedImage.fileHeader = image.fileHeader;

    flippedImage.infoHeader = image.infoHeader;

    flippedImage.pixels = flippedPixels;

    return flippedImage;
}

IMAGE flipVertically(IMAGE image){
    
    IMAGE flippedImage;
    PIXEL **flippedPixels, **pixels = image.pixels;

    int width = image.infoHeader.biWidth;
    int height = image.infoHeader.biHeight;
    int sizePixel, i, j;

    sizePixel = getSizePixel();
    
    flippedPixels = (struct PIXEL **)malloc(height * sizeof(struct PIXEL*));
    for(i = 0; i < height; i++){
        flippedPixels[i] = (struct PIXEL *)malloc(width * sizeof(sizePixel));
    }

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            flippedPixels[i][j] = pixels[height-1-i][j];
        }
    }

    flippedImage.fileHeader = image.fileHeader;

    flippedImage.infoHeader = image.infoHeader;

    flippedImage.pixels = flippedPixels;

    return flippedImage;
}

IMAGE applyGrayscale(IMAGE image){

    IMAGE appliedImage;
    PIXEL **grayscaledPixels, **pixels = image.pixels;
    
    int width = image.infoHeader.biWidth;
    int height = image.infoHeader.biHeight;
    int sizePixel, i, j;

    sizePixel = getSizePixel();
    
    grayscaledPixels = (struct PIXEL **)malloc(height * sizeof(struct PIXEL*));
    for(i = 0; i < height; i++){
        grayscaledPixels[i] = (struct PIXEL *)malloc(width * sizeof(sizePixel));
    }

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            grayscaledPixels[i][j].blue = combLinear(pixels[i][j].blue, pixels[i][j].green, pixels[i][j].red);
            grayscaledPixels[i][j].green = combLinear(pixels[i][j].blue, pixels[i][j].green, pixels[i][j].red);
            grayscaledPixels[i][j].red = combLinear(pixels[i][j].blue, pixels[i][j].green, pixels[i][j].red);
        }
    }

    appliedImage.fileHeader = image.fileHeader;

    appliedImage.infoHeader = image.infoHeader;

    appliedImage.pixels = grayscaledPixels;

    return appliedImage;

}

void writeToFile(FILE *file, IMAGE image){
    int i, padding;
    int width = image.infoHeader.biWidth;
    int height = image.infoHeader.biHeight;

    fwrite(&image.fileHeader, FILE_HEADER_SIZE, 1, file);
    fwrite(&image.infoHeader, INFO_HEADER_SIZE, 1, file);

    padding = getPadding(width);
    for(i = 0; i < height; i++){
        fwrite(image.pixels[i], width, BYTES_PER_PIXEL, file);
        fwrite(&paddPixel, padding, BYTES_PER_PIXEL, file);
    }

}

void showInterface(){
    int size;

    printf("\n\n\n%s            Hello, welcome!\n", YELLOW);
    printf("         / Which image do you want to process today?\n");
    printf("   |||||\n");
    printf("  ||O O|`____.\n");
    printf(" |||\\-/|| \\ __\\\n");
    printf(" |.--:--|  .   :\n");
    printf(" /( ):( |_.-~~_.\n");
    printf("(~m  : /  | oo:|\n");
    printf("~~~~~~~~~~~~~~~~~%s\n\n\n", NORMAL); 

    printf("Directory: ");

    if ( fgets(filename, sizeof(filename), stdin)){
        size = strlen(filename) - 1;

        if (filename[size] == '\n'){
            filename[size] = '\0';
        }
    }
}

void showMenu(){

    printf("\n\n\n%s            Me again here,\n", YELLOW);
    printf("         / What transformation do you want to do?\n");
    printf("   |||||\n");
    printf("  ||O O|`____.\n");
    printf(" |||\\-/|| \\ __\\\n");
    printf(" |.--:--|  .   :\n");
    printf(" /( ):( |_.-~~_.\n");
    printf("(~m  : /  | oo:|\n");
    printf("~~~~~~~~~~~~~~~~~%s\n\n\n", NORMAL);

    printf("1. Copy image\n");
    printf("2. Rotate 90 degrees right\n");
    printf("3. Rotate 90 degrees left\n");
    printf("4. Rotate 180 degrees\n");
    printf("5. Flip horizontally\n");
    printf("6. Flip vertically\n");
    printf("7. Apply grayscale\n");
    printf("0. Close program\n\n");

    printf("Option: ");

    scanf("%d", &choisenOption);
    
}

void showSuccess(){

    printf("\n\n\n%s        Congratulations, processed image.\n", YELLOW);
    printf("         / Don't forget to check the images directory.\n");
    printf("   |||||\n");
    printf("  ||O O|`____.\n");
    printf(" |||\\-/|| \\ __\\\n");
    printf(" |.--:--|  .   :\n");
    printf(" /( ):( |_.-~~_.\n");
    printf("(~m  : /  | oo:|\n");
    printf("~~~~~~~~~~~~~~~~~%s\n\n\n", NORMAL);
    
}

void showProgressArt(){
    printf("\n\n\n%s         / Please, wait a little bit.\n", YELLOW);
    printf("   |||||\n");
    printf("  ||O O|`____.\n");
    printf(" |||\\-/|| \\ __\\\n");
    printf(" |.--:--|  .   :\n");
    printf(" /( ):( |_.-~~_.\n");
    printf("(~m  : /  | oo:|\n");
    printf("~~~~~~~~~~~~~~~~~%s\n\n\n", NORMAL);
}

void showProgressBar(){
    int i, prefixLength, suffixLength; 
    char *progress;
	const char prefix[] = "Progress: ";
	const char suffix[] = "]";

	prefixLength = sizeof(prefix) - 1;
	suffixLength = sizeof(suffix) - 1;

	progress = calloc(100 + prefixLength + suffixLength + 1, 1);

	for (i = 0; i <= 100; i+=20){
        system("cls");
        showProgressArt();
        if(i == 100){
            system("cls");
        }
        printf("%s %d%%: [", prefix, i);
        for(int j = 0; j < 100; j++){
            progress[j] = j < i ? '|' : ' ';  
        }	
        
        printf("%s%s\n", progress, suffix);
        delay(1);
        
	}
}

void closeProgram(){
    int i;

    printf("Exiting program, wait a moment");
    for(i = 0; i < 3; i++){
        delay(1);
        printf(".");
    }

    delay(1);
    exit(1);
}

int main(){

    system("COLOR 0");
    system("cls");
    
    FILE *file, *processedFile;
    IMAGE image = { NULL }, processedImage = { NULL };
    int i, width, height, size, dirCheck;

    showInterface();

    file = fopen(filename, "rb");

    if(file == NULL){
        printf("| WARNING | Error opening the file.\n");
    } else {
        printf("| WARNING | Image successfully opened.\n");
    }

    dirCheck = mkdir(DEFAULT_PATH);
    if(!dirCheck){
        printf("| WARNING | Created the images directory.\n");
    } else {
        printf("| WARNING | Unable to create or already exists the images directory to store the processed images.\n");
    }

    image = readImage(file);

    width = image.infoHeader.biWidth;
    height = image.infoHeader.biHeight;
    size = image.infoHeader.biSizeImage;

    printf("\nImage Information:\n\n");
    printf("Type: Bitmap 24-bit\n");
    printf("Width: %d pixels\n", width);
    printf("Height: %d pixels\n", height);
    printf("Size: %.2f megabytes\n\n", convertByteToMegabyte(size));

    printf("Please, wait a litle bit");
    for(i = 0; i < 5; i++){
        delay(1);
        printf(".");
    }
    system("cls");
    showMenu();

    system("cls");
    switch(choisenOption){
        
        case 0: closeProgram();
            break;

        case 1: 
            showProgressBar();
            processedFile = fopen(DEFAULT_PATH "copied-image.bmp", "wb");
            processedImage = copy(image);
            writeToFile(processedFile, processedImage);
            showSuccess(file, image);
            fclose(processedFile);
            closeProgram();
            break;

        case 2:
            showProgressBar();
            processedFile = fopen(DEFAULT_PATH "rotated-image-90-degrees-right.bmp", "wb");
            processedImage = rotate90DegreesRight(image);
            writeToFile(processedFile, processedImage);
            showSuccess(file, image);
            fclose(processedFile);
            closeProgram();
            break;

        case 3:
            showProgressBar();
            processedFile = fopen(DEFAULT_PATH "rotated-image-90-degrees-left.bmp", "wb");
            processedImage = rotate90DegreesLeft(image);
            writeToFile(processedFile, processedImage);
            showSuccess(file, image);
            fclose(processedFile);
            closeProgram();
            break;

        case 4:
            showProgressBar();
            processedFile = fopen(DEFAULT_PATH "rotated-image-180-degrees.bmp", "wb");
            processedImage = rotate180Degrees(image);
            writeToFile(processedFile, processedImage);
            showSuccess(file, image);
            fclose(processedFile);
            closeProgram();
            break;

        case 5:
            showProgressBar();
            processedFile = fopen(DEFAULT_PATH "flipped-image-horizontally.bmp", "wb");
            processedImage = flipHorizontally(image);
            writeToFile(processedFile, processedImage);
            showSuccess(file, image);
            fclose(processedFile);
            closeProgram();
            break;

        case 6:
            showProgressBar();
            processedFile = fopen(DEFAULT_PATH "flipped-image-vertically.bmp", "wb");
            processedImage = flipVertically(image);
            writeToFile(processedFile, processedImage);
            showSuccess(file, image);
            fclose(processedFile);
            closeProgram();
            break;

        case 7:
            showProgressBar();
            processedFile = fopen(DEFAULT_PATH "grayscaled-image.bmp", "wb");
            processedImage = applyGrayscale(image);
            writeToFile(processedFile, processedImage);
            showSuccess(file, image);
            fclose(processedFile);
            closeProgram();
            break;

    }
    
    fclose(file);

    return 0;
}
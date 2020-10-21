#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct Image {
    int width;
    int height;
    uint8_t* data;
} Image;


// PGMをロードする
// dataは動的確保される
static Image load_pgm(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open %s\n", path);
        exit(1);
    }
    char buf[1024];
    fgets(buf, sizeof(buf), fp);
    if (strncmp(buf, "P5", 2) != 0) {
        fprintf(stderr, "Unexpected file format.\n");
        exit(1);
    }
    int width, height, max;
    if (fscanf(fp, "%d %d", &width, &height) != 2) {
        fprintf(stderr, "Unexpected file format.\n");
        exit(1);
    }
    if (fscanf(fp, "%d", &max) != 1) {
        fprintf(stderr, "Unexpected file format.\n");
        exit(1);
    }
    if (max != 255) {
        fprintf(stderr, "Unexpected file format.\n");
        exit(1);
    }
    // 改行読み飛ばし
    fgetc(fp);
    printf("[INFO] width: %d, height: %d, max: %d\n", width, height, max);

    Image img = {
        .width = width,
        .height = height,
        .data = malloc(height * width * sizeof(uint8_t))
    };
    if (img.data == NULL) {
        fprintf(stderr, "Failed memory allocation.\n");
        exit(1);
    }
    for (int i = 0; i < height * width; ++i) {
        int c = fgetc(fp);
        if (c == EOF) {
            fprintf(stderr, "Failed get char.\n");
            exit(1);
        }
        img.data[i] = c;
    }
    fclose(fp);
    return img;
}

// dataを解放する
static bool destroy_image(Image* img) {
    if (img->data != NULL) {
        free(img->data);
        img->data = NULL;
        return true;
    }
    return false;
}


// Imageをpgmとして保存
static bool save_pgm(const Image* img, const char* path) {
    FILE* fp = fopen(path, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open %s\n", path);
        exit(1);
    }
    fprintf(fp, "P5\n%d %d\n%d\n", img->width, img->height, 255);
    for (int i = 0; i < img->width * img->height; ++i) {
        if (fputc(img->data[i], fp) == EOF) {
            fprintf(stderr, "Failed write pgm.\n");
            return false;
        }
    }
    fclose(fp);
    return true;
}

int main(void) {
    Image img = load_pgm("lena_gray.pgm");
    save_pgm(&img, "lene_gray_copy.pgm");

    return 0;
}


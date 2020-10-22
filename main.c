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


static Image convert_error_diffusion(const Image* img) {
    static const int RATIO_COUNT = 4;
    static const int TDX[] = {1, -1, 0, 1};
    static const int TDY[] = {0,  1, 1, 1};
    static const int RATIO[] = {7, 3, 5, 1};
    static const int DENOMINATOR = 16;

    int width = img->height;
    int height = img->width;
    uint8_t* bmp_from = img->data;
    short* bmp_to = (short*)calloc(height * width, 4);

    if (bmp_to == NULL) {
        fprintf(stderr, "Failed memory allocation.\n");
        exit(1);
    }
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int f = bmp_to[y * width + x] + bmp_from[y * width + x];
            short d;
            if (f >= 128) {
                d = f - 255;
                bmp_to[y * width + x] = 255;
            } else {
                d = f;
                bmp_to[y * width + x] = 0;
            }
            for (int c = 0; c < RATIO_COUNT; ++c) {
                int px = x + TDX[c];
                int py = y + TDY[c];
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    bmp_to[py * width + px] += d * RATIO[c] / DENOMINATOR;
                }
            }
        }
    }
    Image converted = {
        .width = width,
        .height = height,
        .data = malloc(width * height)
    };
    if (converted.data == NULL) {
        fprintf(stderr, "Failed memory allocation.\n");
        exit(1);
    }
    for (int i = 0; i < width * height; ++i) {
        converted.data[i] = bmp_to[i];
    }
    free(bmp_to);
    return converted;
}


int main(int argc, char* argv[]) {
    char in_pgm_path[1024] = "lena_gray.pgm";
    char out_pgm_path[1024] = "out.pgm";

    if (argc == 2) {
        strncpy(in_pgm_path, argv[1], sizeof(in_pgm_path));
    }
    if (argc == 3) {
        strncpy(in_pgm_path, argv[1], sizeof(in_pgm_path));
        strncpy(out_pgm_path, argv[2], sizeof(out_pgm_path));
    }

    Image img = load_pgm(in_pgm_path);
    Image converted = convert_error_diffusion(&img);

    save_pgm(&converted, out_pgm_path);

    destroy_image(&converted);
    destroy_image(&img);

    return 0;
}


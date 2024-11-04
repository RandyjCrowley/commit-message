#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // You'll need to download this header-only library

// Structure to hold pixel information
typedef struct {
    int week;
    int day;
    int commits;
} PixelInfo;

// Structure to hold array of pixels
typedef struct {
    PixelInfo* pixels;
    int count;
} PixelArray;

// Get the date string in git format
void format_date(char* buffer, size_t size, struct tm* date) {
    strftime(buffer, size, "%a %b %d %H:%M %Y +0700", date);
}

// Create a single commit for a specific date
void create_commit(struct tm* date) {
    char date_str[100];
    char command[256];
    FILE* file;

    // Append to test.txt
    file = fopen("test.txt", "a");
    if (file) {
        fprintf(file, "aaaaa");
        fclose(file);
    }

    // Git commands
    system("git add test.txt");
    system("git commit -m \"some stuff\"");
    
    format_date(date_str, sizeof(date_str), date);
    snprintf(command, sizeof(command), 
             "git commit --amend -m \"some stuff\" --date=\"%s\"", 
             date_str);
    system(command);
}

// Create multiple commits for a specific date
void create_commits_for_date(int num_commits, struct tm* date) {
    char date_str[100];
    format_date(date_str, sizeof(date_str), date);
    printf("Creating %d commits on %s\n", num_commits, date_str);

    for (int i = 0; i < num_commits; i++) {
        create_commit(date);
    }
}

// Get the start date (52 weeks ago from last Monday)
time_t get_start_date() {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    
    // Get to Monday
    int days_since_monday = tm_now->tm_wday;
    if (days_since_monday == 0) days_since_monday = 7;
    
    // Subtract days to get to Monday
    now -= (days_since_monday - 1) * 24 * 60 * 60;
    
    // Subtract 52 weeks and 1 day
    now -= ((52 * 7) + 1) * 24 * 60 * 60;
    
    return now;
}

// Read and process the image
PixelArray read_image(const char* img_name) {
    int width, height, channels;
    uint8_t* image = stbi_load(img_name, &width, &height, &channels, 1);
    PixelArray result = {NULL, 0};
    
    if (!image) {
        printf("Failed to load image: %s\n", img_name);
        return result;
    }

    // First count how many pixels we need to process
    int pixel_count = 0;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            uint8_t pixel_val = image[j * width + i];
            int avg_darkness = 255 - pixel_val;
            int green_level = avg_darkness / 51;
            if (green_level > 0) pixel_count++;
        }
    }

    // Allocate array
    result.pixels = malloc(sizeof(PixelInfo) * pixel_count);
    result.count = pixel_count;

    // Fill array
    int idx = 0;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            uint8_t pixel_val = image[j * width + i];
            int avg_darkness = 255 - pixel_val;
            int green_level = avg_darkness / 51;
            if (green_level > 0) {
                result.pixels[idx].week = i;
                result.pixels[idx].day = j;
                result.pixels[idx].commits = green_level;
                idx++;
            }
        }
    }

    stbi_image_free(image);
    return result;
}

// Get commit date based on offsets
time_t get_commit_date(time_t start_date, int weeks, int days) {
    return start_date + (weeks * 7 + days) * 24 * 60 * 60;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <image_file>\n", argv[0]);
        return 1;
    }

    // Get base date
    time_t base_date = get_start_date();
    struct tm* tm_base = localtime(&base_date);
    char base_date_str[100];
    format_date(base_date_str, sizeof(base_date_str), tm_base);
    printf("Base date: %s\n", base_date_str);

    // Read pixels
    PixelArray pixels = read_image(argv[1]);
    printf("Found %d pixels to process\n", pixels.count);

    // Process each pixel
    for (int i = 0; i < pixels.count; i++) {
        time_t commit_date = get_commit_date(base_date, 
                                          pixels.pixels[i].week, 
                                          pixels.pixels[i].day);
        struct tm* tm_commit = localtime(&commit_date);
        char date_str[100];
        strftime(date_str, sizeof(date_str), "%m-%d-%Y", tm_commit);
        
        printf("Commit date: %s, commits: %d\n", 
               date_str, pixels.pixels[i].commits);
               
        create_commits_for_date(pixels.pixels[i].commits, tm_commit);
    }

    // Clean up
    free(pixels.pixels);
    return 0;
}

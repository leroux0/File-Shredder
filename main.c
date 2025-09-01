#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    // For srand()
#include <fcntl.h>   // For open(), O_WRONLY, etc.
#include <sys/stat.h>// For fstat() to get file size
#include <getopt.h>  // For command-line argument parsing

#ifdef _WIN32
#include <io.h>      // For _commit on Windows
#include <windows.h> // For alternative file handling if needed
#define fsync _commit
#define unlink _unlink  // Use _unlink on Windows
#else
#include <unistd.h>  // For unlink(), fsync() on Unix-like systems
#endif

// On Windows, O_SYNC might not be defined or supported; define it as 0 to avoid errors
#ifndef O_SYNC
#define O_SYNC 0
#endif

// Function to display usage instructions
void usage(const char *progname) {
    fprintf(stderr, "Usage: %s -f <file> [-n <passes>] [-p <pattern>]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -f <file>    : Path to the file to shred (required)\n");
    fprintf(stderr, "  -n <passes>  : Number of overwrite passes (default: 3)\n");
    fprintf(stderr, "  -p <pattern> : Overwrite pattern: 'zeros' or 'random' (default: 'zeros')\n");
    exit(EXIT_FAILURE);
}

// Function to overwrite the file with a given pattern
int overwrite_file(int fd, off_t file_size, const char *pattern, int passes) {
    char *buffer = malloc(file_size);
    if (!buffer) {
        perror("malloc failed");
        return -1;
    }

    for (int pass = 0; pass < passes; ++pass) {
        // Prepare the buffer based on pattern
        if (strcmp(pattern, "zeros") == 0) {
            memset(buffer, 0, file_size);
        } else if (strcmp(pattern, "random") == 0) {
            // Fill with random data
            for (off_t i = 0; i < file_size; ++i) {
                buffer[i] = rand() & 0xFF;  // Bitwise AND to get a byte
            }
        } else {
            fprintf(stderr, "Invalid pattern: %s\n", pattern);
            free(buffer);
            return -1;
        }

        // Seek to beginning
        if (lseek(fd, 0, SEEK_SET) == -1) {
            perror("lseek failed");
            free(buffer);
            return -1;
        }

        // Write the buffer
        ssize_t written = write(fd, buffer, file_size);
        if (written != file_size) {
            perror("write failed");
            free(buffer);
            return -1;
        }

        // Sync to disk (using fsync or equivalent)
        if (fsync(fd) == -1) {
            perror("fsync failed");
            free(buffer);
            return -1;
        }

        printf("Pass %d/%d completed with pattern '%s'.\n", pass + 1, passes, pattern);
    }

    free(buffer);
    return 0;
}

int main(int argc, char *argv[]) {
    char *filename = NULL;
    int passes = 3;  // Default passes
    char *pattern = "zeros";  // Default pattern
    int opt;

    // Parse command-line options using getopt
    while ((opt = getopt(argc, argv, "f:n:p:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'n':
                passes = atoi(optarg);
                if (passes <= 0) {
                    fprintf(stderr, "Invalid number of passes: %d\n", passes);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                if (strcmp(optarg, "zeros") != 0 && strcmp(optarg, "random") != 0) {
                    fprintf(stderr, "Invalid pattern: %s. Must be 'zeros' or 'random'.\n", optarg);
                    exit(EXIT_FAILURE);
                }
                pattern = optarg;
                break;
            default:
                usage(argv[0]);
        }
    }

    if (!filename) {
        usage(argv[0]);
    }

    // Seed random number generator if using random pattern
    if (strcmp(pattern, "random") == 0) {
        srand(time(NULL));
    }

    // Open the file with low-level access (O_WRONLY, and O_SYNC where supported)
    int fd = open(filename, O_WRONLY | O_SYNC);
    if (fd == -1) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    // Get file size
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
    off_t file_size = st.st_size;
    if (file_size == 0) {
        printf("File is empty, nothing to shred.\n");
        close(fd);
        if (unlink(filename) == -1) {
            perror("unlink failed");
        }
        exit(EXIT_SUCCESS);
    }

    // Overwrite the file
    if (overwrite_file(fd, file_size, pattern, passes) == -1) {
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Close the file
    close(fd);

    // Delete the file
    if (unlink(filename) == -1) {
        perror("unlink failed");
        exit(EXIT_FAILURE);
    }

    printf("File '%s' securely shredded and deleted.\n", filename);
    return EXIT_SUCCESS;
}
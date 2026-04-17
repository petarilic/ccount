#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FUNCS 256
#define MAX_NAME 128

typedef struct {
    char name[MAX_NAME];
    int loc;
} Function;

// Optimized: use strspn for faster blank detection
static inline int is_blank(const char *line) {
    return line[strspn(line, " \t\r\n")] == '\0';
}

// Strip comments from line - optimized with early returns
static inline void strip_comments(char *line, int *in_block_comment) {
    char *src = line;
    char *dst = line;

    while (*src) {
        if (*in_block_comment) {
            if (src[0] == '*' && src[1] == '/') {
                *in_block_comment = 0;
                src += 2;
            } else {
                src++;
            }
        } else {
            if (src[0] == '/' && src[1] == '*') {
                *in_block_comment = 1;
                src += 2;
            } else if (src[0] == '/' && src[1] == '/') {
                break;  // Line comment - rest of line is ignored
            } else {
                *dst++ = *src++;
            }
        }
    }
    *dst = '\0';
}

// Optimized: avoid redundant checks and use pointer arithmetic
static inline int is_function_start(const char *line, char *name_out) {
    const char *p = strchr(line, '(');
    if (!p) return 0;

    const char *brace = strchr(line, '{');
    if (!brace || brace < p) return 0;

    // Extract function name (word before '(')
    const char *end = p - 1;
    while (end > line && isspace((unsigned char)*end)) end--;

    if (end <= line) return 0;

    const char *start = end;
    while (start > line && (isalnum((unsigned char)*start) || *start == '_')) start--;

    // Move forward if we stopped at non-alphanumeric (but not at start)
    if (start > line && !isalnum((unsigned char)*start) && *start != '_') {
        start++;
    }

    int len = (int)(end - start + 1);
    if (len <= 0 || len >= MAX_NAME) return 0;

    memcpy(name_out, start, len);
    name_out[len] = '\0';

    return 1;
}

// Check if inside string/character literal (simple heuristic)
static inline int in_string_literal(const char *line, const char *pos) {
    int in_string = 0;
    char quote = 0;
    
    for (const char *c = line; c < pos; c++) {
        if (!in_string) {
            if (*c == '"' || *c == '\'') {
                in_string = 1;
                quote = *c;
            }
        } else {
            if (*c == quote && *(c-1) != '\\') {
                in_string = 0;
            }
        }
    }
    return in_string;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Error opening file");
        return 1;
    }

    // Use larger buffer for faster I/O
    char line[4096];
    int in_block_comment = 0;

    int total_loc = 0;
    long total_chars = 0;
    int include_loc = 0;
    int global_loc = 0;

    int in_function = 0;
    int brace_depth = 0;

    // Static allocation - no malloc needed, automatic cleanup on exit
    Function funcs[MAX_FUNCS];
    int func_count = 0;

    while (fgets(line, sizeof(line), f)) {
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }

        strip_comments(line, &in_block_comment);

        if (is_blank(line)) continue;

        // Count characters (excluding comments already stripped)
        total_chars += strlen(line);

        // Trim leading spaces
        const char *trim = line;
        while (isspace((unsigned char)*trim)) trim++;

        total_loc++;

        if (!in_function) {
            if (trim[0] == '#' && strncmp(trim, "#include", 8) == 0) {
                include_loc++;
                continue;
            }

            char fname[MAX_NAME];
            if (is_function_start(trim, fname)) {
                in_function = 1;
                brace_depth = 1;

                if (func_count < MAX_FUNCS) {
                    memcpy(funcs[func_count].name, fname, MAX_NAME);
                    funcs[func_count].name[MAX_NAME-1] = '\0';
                    funcs[func_count].loc = 1;
                    func_count++;
                }
                continue;
            }

            global_loc++;
        } else {
            // Inside function - count LOC and track brace depth
            if (func_count > 0) {
                funcs[func_count - 1].loc++;
            }

            // Count braces, but skip those in string literals
            for (const char *c = trim; *c; c++) {
                if ((*c == '{' || *c == '}') && !in_string_literal(trim, c)) {
                    if (*c == '{') brace_depth++;
                    else brace_depth--;
                }
            }

            if (brace_depth == 0) {
                in_function = 0;
            }
        }
    }

    fclose(f);

    // Output results
    printf("=== LOC REPORT ===\n");
    printf("Total LOC (excl. comments/empty lines): %d\n", total_loc);
    printf("Total chars (incl. whitespace, excl. comments/empty): %ld\n\n", total_chars);

    printf("Includes: %d\n", include_loc);
    printf("Global variables: %d\n\n", global_loc);

    printf("Functions:\n");
    for (int i = 0; i < func_count; i++) {
        printf("  %s: %d lines\n", funcs[i].name, funcs[i].loc);
    }

    // All memory is stack-allocated, automatically freed on exit
    // No dynamic allocation used - nothing to free
    return 0;
}

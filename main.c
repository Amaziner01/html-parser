/* 
 * Copyright (c) 2021 Amaziner 01
 * Unlicenced code
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int read_file(const char *filepath, uint32_t *count, char *out)
{
    if (count == NULL && out == NULL) return 0;
    FILE *stream = fopen(filepath, "rb");
    
    if (!stream) return 0;

    fseek(stream, 0, SEEK_END);
    int len = ftell(stream);
    if (count) *count = len;

    if (out) 
    {
        fseek(stream, 0, SEEK_SET);
        fread(out, len, 1, stream);
    }

    fclose(stream);
    return 1;
}

struct html_tag
{
    uint32_t        tag_id;
    char            *inner_text;
    struct html_tag *inner_tags;
};

struct token
{
    void *value;

    enum token_type 
    {
        TOKEN_OPTAG = '<',
        TOKEN_CLTAG = '>',
        TOKEN_IDEN  = 'a',
        TOKEN_STR   = 's',
        TOKEN_EQ    = '=',
        TOKEN_SLASH = '/',

        TOKEN_EOF   = 'F',
    } type;
};

static struct 
{
    struct token *tokens;
    uint32_t      count, capacity;
} lexing_obj = {NULL, 0, 10};

__attribute((__always_inline__)) 
static inline void lexing_obj_init() {
    lexing_obj.tokens = malloc(sizeof(struct token) * 10);
}

__attribute((__always_inline__)) 
static inline void lexing_obj_insert(struct token *token) {
    if (lexing_obj.count >= lexing_obj.capacity) 
    {
        lexing_obj.capacity *= 2;
        lexing_obj.tokens   = (struct token*)realloc(
                lexing_obj.tokens, 
                lexing_obj.capacity * sizeof(struct token)
                );
    }

    printf("Pushing TOKEN{%c, %p} to position %i\n", token->type, token->value, lexing_obj.count);
    memcpy(&lexing_obj.tokens[lexing_obj.count], 
            token, sizeof(struct token));
    ++lexing_obj.count;
}

int html_lex(const char *src)
{
    lexing_obj_init();
    struct token dummy_token = {0};
    int line_no = 0;

    uint8_t state = *src;
    while (*src) {

        uint8_t statebuff = state;
        switch (state) {
            case '<': // Open Tag
            case '>': // Closing Tag
            case '/': // Slash
            case '=': // Equals
                dummy_token.type = state;
                break;

            case '\t': // Space & Tabs
            case ' ':
                while (*src == ' ' || *src == '\t') ++src;
                break;

            case '\n': // Newlines
                puts("Found newline");
                line_no += 1;
                break;

            case '\"': //
                break;

            case '\0':
                return 1;

            default:

                if (*src >= 'a' && *src <= 'z'
                 || *src >= 'A' && *src <= 'Z')
                {
                    puts("Found string");
                    dummy_token.type = TOKEN_IDEN;
                    const char *beg = src;
                    while (*src >= 'a' && *src <= 'z'
                            || *src >= 'A' && *src <= 'Z'
                            || *src >= '0' && *src <= '9') ++src; 
                    long len = src - beg;
                    --src;

                    break;
                }

                return 0;
        }
        lexing_obj_insert(&dummy_token);
        ++src;
        state = *src;
    }
}

int html_parse(struct token *tokens)
{
    
}

int main(int argc, char **argv) 
{
    if (argc < 2) return -1;

    uint32_t file_size = 0;
    char     *src      = NULL;

    read_file(argv[1], &file_size, NULL);
    src = calloc(1, file_size); 
    read_file(argv[1], &file_size, src);

    printf("File contents: %s\n", src);

    // Lexing
    html_lex(src);

    for (int i = 0; i < lexing_obj.count; i++)
    {
        const char *type = NULL;
        switch (lexing_obj.tokens[i].type)
        {
            case TOKEN_OPTAG: type = "Open Tag";   break;
            case TOKEN_CLTAG: type = "Close Tag";  break;
            case TOKEN_IDEN:  type = "Identifier"; break;
            case TOKEN_STR:   type = "String";     break;
            case TOKEN_SLASH: type = "Slash";      break;
        }
        printf("[TAG] Type: %s\n", type);
    }

    // Cleanup
    free((void*)src); 

    return 0;
}

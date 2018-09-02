#include "parse.h"
#include "parser.h"
#include <assert.h>

int parse_primary(struct parser *parser)
{
    if (parser_match(parser, TOKEN_KIND_INT_LITERAL)) {
        return parser_previous(parser).as.i;
    } else if (parser_match(parser, TOKEN_KIND_OPEN_PAREN)) {
        int result = parse_expression(parser);
        parser_match(parser, TOKEN_KIND_CLOSE_PAREN);
        return result;
    }
    assert(false);
}

int parse_unary(struct parser *parser)
{
    if (parser_match(parser, TOKEN_KIND_DASH)) {
        return -parse_unary(parser);
    }
    return parse_primary(parser);
}

int parse_factor(struct parser *parser)
{
    int lhs = parse_unary(parser);
    while ((parser_match(parser, TOKEN_KIND_STAR)) ||
           (parser_match(parser, TOKEN_KIND_SLASH))) {
        struct token op = parser_previous(parser);
        int rhs = parse_unary(parser);
        if (op.kind == TOKEN_KIND_STAR) {
            lhs *= rhs;
        } else if (op.kind == TOKEN_KIND_SLASH) {
            lhs /= rhs;
        }
    }
    return lhs;
}

int parse_term(struct parser *parser)
{
    int lhs = parse_factor(parser);
    while ((parser_match(parser, TOKEN_KIND_PLUS)) ||
           (parser_match(parser, TOKEN_KIND_DASH))) {
        struct token op = parser_previous(parser);
        int rhs = parse_factor(parser);
        if (op.kind == TOKEN_KIND_PLUS) {
            lhs += rhs;
        } else if (op.kind == TOKEN_KIND_DASH) {
            lhs -= rhs;
        }
    }
    return lhs;
}

int parse_expression(struct parser *parser)
{
    return parse_term(parser);
}

void parse_string(uint8_t *string)
{
    struct parser parser = {
        .lexer = (struct lexer) {
            .buffer = string,
            .at = string,
            .column = 1,
            .line = 1
        },
        .tokens = NULL,
        .current_token = 0,
    };
    buf_push(parser.tokens, lexer_get_token(&parser.lexer));

    while (!parser_eof(&parser)) {
        int r = parse_expression(&parser);
        printf("r = %d\n", r);
    }
}

void parse_file(uint8_t *file)
{
    struct parser parser;
    parser_init(&parser, file);

    while (!parser_eof(&parser)) {
        int r = parse_expression(&parser);
        printf("r = %d\n", r);
    }
}
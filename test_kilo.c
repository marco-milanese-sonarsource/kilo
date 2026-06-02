/* Rename kilo's main so we can define our own entry point. */
#define main kilo_main
#include "kilo.c"
#undef main

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RUN(test) do { test(); printf("PASS  " #test "\n"); } while (0)

static void test_is_separator(void) {
    assert(is_separator('\0') == 1);
    assert(is_separator(' ')  == 1);
    assert(is_separator('\t') == 1);
    assert(is_separator('\n') == 1);

    assert(is_separator(',') == 1);
    assert(is_separator('.') == 1);
    assert(is_separator('(') == 1);
    assert(is_separator(')') == 1);
    assert(is_separator('+') == 1);
    assert(is_separator('-') == 1);
    assert(is_separator('/') == 1);
    assert(is_separator('*') == 1);
    assert(is_separator('=') == 1);
    assert(is_separator('~') == 1);
    assert(is_separator('%') == 1);
    assert(is_separator('[') == 1);
    assert(is_separator(']') == 1);
    assert(is_separator(';') == 1);

    assert(is_separator('a') == 0);
    assert(is_separator('Z') == 0);
    assert(is_separator('9') == 0);
    assert(is_separator('_') == 0);
}

static void test_editorSyntaxToColor(void) {
    assert(editorSyntaxToColor(HL_COMMENT)   == 36);
    assert(editorSyntaxToColor(HL_MLCOMMENT) == 36);
    assert(editorSyntaxToColor(HL_KEYWORD1)  == 33);
    assert(editorSyntaxToColor(HL_KEYWORD2)  == 32);
    assert(editorSyntaxToColor(HL_STRING)    == 35);
    assert(editorSyntaxToColor(HL_NUMBER)    == 31);
    assert(editorSyntaxToColor(HL_MATCH)     == 34);
    assert(editorSyntaxToColor(HL_NORMAL)    == 37);
}

static void test_abuf(void) {
    struct abuf ab = ABUF_INIT;
    assert(ab.b == NULL);
    assert(ab.len == 0);

    abAppend(&ab, "foo", 3);
    assert(ab.len == 3);
    assert(memcmp(ab.b, "foo", 3) == 0);

    abAppend(&ab, "bar", 3);
    assert(ab.len == 6);
    assert(memcmp(ab.b, "foobar", 6) == 0);

    abFree(&ab);
}

static void test_editorRowHasOpenComment(void) {
    erow row;

    /* NULL hl: not in a comment */
    row.hl = NULL;
    row.rsize = 0;
    assert(editorRowHasOpenComment(&row) == 0);

    /* rsize == 0: not in a comment even with hl set */
    unsigned char hl_zero[1] = {HL_MLCOMMENT};
    row.hl = hl_zero;
    row.rsize = 0;
    assert(editorRowHasOpenComment(&row) == 0);

    /* Single HL_MLCOMMENT char (rsize < 2): open comment */
    unsigned char hl_open[1] = {HL_MLCOMMENT};
    char render_open[1] = {'x'};
    row.hl = hl_open;
    row.render = render_open;
    row.rsize = 1;
    assert(editorRowHasOpenComment(&row) == 1);

    /* Ends with star-slash and last hl is HL_MLCOMMENT: comment properly closed */
    unsigned char hl_closed[2] = {HL_MLCOMMENT, HL_MLCOMMENT};
    char render_closed[2] = {'*', '/'};
    row.hl = hl_closed;
    row.render = render_closed;
    row.rsize = 2;
    assert(editorRowHasOpenComment(&row) == 0);

    /* Last char is HL_NORMAL: not inside a comment block */
    unsigned char hl_normal[3] = {HL_MLCOMMENT, HL_MLCOMMENT, HL_NORMAL};
    char render_normal[3] = {' ', ' ', 'x'};
    row.hl = hl_normal;
    row.render = render_normal;
    row.rsize = 3;
    assert(editorRowHasOpenComment(&row) == 0);
}

static void test_editorSelectSyntaxHighlight(void) {
    E.syntax = NULL;
    editorSelectSyntaxHighlight("main.c");
    assert(E.syntax != NULL);

    E.syntax = NULL;
    editorSelectSyntaxHighlight("header.h");
    assert(E.syntax != NULL);

    E.syntax = NULL;
    editorSelectSyntaxHighlight("app.cpp");
    assert(E.syntax != NULL);

    E.syntax = NULL;
    editorSelectSyntaxHighlight("script.py");
    assert(E.syntax == NULL); /* no Python entry in HLDB */

    E.syntax = NULL;
    editorSelectSyntaxHighlight("Makefile");
    assert(E.syntax == NULL);
}

static void test_editorRowsToString(void) {
    E.numrows = 2;
    E.row = malloc(2 * sizeof(erow));

    E.row[0].chars = strdup("hello");
    E.row[0].size  = 5;
    E.row[1].chars = strdup("world");
    E.row[1].size  = 5;

    int buflen = 0;
    char *s = editorRowsToString(&buflen);

    assert(buflen == 12); /* "hello\nworld\n" */
    assert(strcmp(s, "hello\nworld\n") == 0);

    free(s);
    free(E.row[0].chars);
    free(E.row[1].chars);
    free(E.row);
    E.row = NULL;
    E.numrows = 0;
}

static void test_editorRowInsertChar(void) {
    E.syntax = NULL;
    E.dirty  = 0;

    erow row = {0};
    row.chars = strdup("helo");
    row.size  = 4;

    editorRowInsertChar(&row, 3, 'l'); /* "helo" → "hello" */

    assert(row.size == 5);
    assert(memcmp(row.chars, "hello", 5) == 0);
    assert(E.dirty == 1);

    free(row.chars);
    free(row.render);
    free(row.hl);
}

static void test_editorRowDelChar(void) {
    E.syntax = NULL;
    E.dirty  = 0;

    erow row = {0};
    row.chars = strdup("hello");
    row.size  = 5;

    editorRowDelChar(&row, 2); /* delete chars[2]='l': "hello" → "helo" */

    assert(row.size == 4);
    assert(memcmp(row.chars, "helo", 4) == 0);

    /* delete beyond size is a no-op */
    editorRowDelChar(&row, 10);
    assert(row.size == 4);

    free(row.chars);
    free(row.render);
    free(row.hl);
}

static void test_editorRowAppendString(void) {
    E.syntax = NULL;
    E.dirty  = 0;

    erow row = {0};
    row.chars = strdup("hello");
    row.size  = 5;

    editorRowAppendString(&row, " world", 6);

    assert(row.size == 11);
    assert(memcmp(row.chars, "hello world", 11) == 0);

    free(row.chars);
    free(row.render);
    free(row.hl);
}

int main(void) {
    RUN(test_is_separator);
    RUN(test_editorSyntaxToColor);
    RUN(test_abuf);
    RUN(test_editorRowHasOpenComment);
    RUN(test_editorSelectSyntaxHighlight);
    RUN(test_editorRowsToString);
    RUN(test_editorRowInsertChar);
    RUN(test_editorRowDelChar);
    RUN(test_editorRowAppendString);
    printf("\nAll tests passed.\n");
    return 0;
}

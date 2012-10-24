/*
 * Copyright (C) 2012 Carolina Aguilar <caroagse@gmail.com>
 * Copyright (C) 2012 Carlos Jenkins <carlos@jenkins.co.cr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"

bool file_exists(char *fname)
{
    FILE* file = fopen(fname, "r");

    if(file != NULL) {
        fclose(file);
        return true;
    }
    return false;
}

char* read_file(char* fname)
{
    /* Check if file exists */
    if(!file_exists(fname)) {
        return NULL;
    }

    /* Open file */
    FILE *f = fopen(fname, "rb");
    if(f == NULL) {
        return NULL;
    }

    /* Count the size of the file */
    fseek(f, 0, SEEK_END);
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long pos = ftell(f);
    if (fseek(f, 0L, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    /* Allocate memory to read the file */
    char *content = malloc(pos + sizeof(char));
    if(content == NULL) {
        fclose(f);
        return NULL;
    }

    /* Read the file */
    fread(content, pos, 1, f);
    fclose(f);

    /* Return the string */
    content[pos] = '\0';
    return content;
    /* You should later "free(bytes);" */
}

char* get_current_time()
{
    GDateTime* now = g_date_time_new_now_local();
    /* 31/12/2012 03:00AM */
    char* formatted = g_date_time_format(now, "%d/%m/%Y %I:%M%p");
    g_date_time_unref(now);
    return formatted;
}

bool fequal(float a, float b)
{
    return fabs(a-b) < F_EPSILON;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

bool copy_streams(FILE* input, FILE* output)
{
    /* Rewind input stream so we can start reading from the beginning */
    rewind(input);

    /* Copy streams */
    char ch;
    while(!feof(input)) {

        ch = fgetc(input);

        if(ferror(input)) {
          return false;
        }
        if(!feof(input)) {
            fputc(ch, output);
        }
        if(ferror(output)) {
            return false;
        }
    }
    return true;
}

bool insert_file(char* filename, FILE* output)
{
    FILE* input = fopen(filename, "rt");
    if(input == NULL) {
        return false;
    }

    bool success = copy_streams(input, output);
    if(!success) {
        return false;
    }

    if(fclose(input) == EOF) {
        return false;
    }

    return true;
}

bool is_empty_string(char* string)
{
    if(*string == '\0') {
        return true;
    }
    return false;
}

const char* seq_names[] = {"A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S",
    "T", "U", "V", "W", "X", "Y", "Z"}; /* 26 */
char* sequence_name(int s)
{
    if(s < 0) { s *= -1; }
    return g_strdup_printf("%s%i", seq_names[s % 26], s / 26);
}

char* get_line(FILE * f)
{
    size_t size = 0;
    size_t len  = 0;
    size_t last = 0;
    char* buf  = NULL;

    do {
        size += BUFSIZ;
        buf = realloc(buf, size);
        fgets(buf + last, size, f);
        len = strlen(buf);
        last = len - 1;
    } while (!feof(f) && (buf[last] != '\n'));
    buf[last] = '\0';
    return buf;
}

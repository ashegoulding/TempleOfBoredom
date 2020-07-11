#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>


typedef enum {
	WD_NONE = -1,
	WD_N,
	WD_NE,
	WD_E,
	WD_SE,
	WD_S,
	WD_SW,
	WD_W,
	WD_NW,
	NB_WD
} word_dir_t;

typedef struct {
	unsigned int w, h;
	char *data; // not null-terminated
} map_t;

static const unsigned int MAP_DIM_LIMIT = 1023;
static const unsigned int WORD_LIST_LIMIT = 1023*1023;


const char *str_word_dir (const word_dir_t dir) {
	switch (dir) {
	case WD_N: return "north";
	case WD_NE: return "northeast";
	case WD_E: return "east";
	case WD_SE: return "southeast";
	case WD_S: return "south";
	case WD_SW: return "southwest";
	case WD_W: return "west";
	case WD_NW: return "northwest";
	}
	return NULL;
}

void transform (const int x, const int y, const word_dir_t dir, const int delta, int *ret_x, int *ret_y) {
	switch (dir) {
	case WD_N: 
		*ret_x = x + 0;
		*ret_y = y - delta;
		break;
	case WD_NE: 
		*ret_x = x + delta;
		*ret_y = y - delta;
		break;
	case WD_E: 
		*ret_x = x + delta;
		*ret_y = y + 0;
		break;
	case WD_SE: 
		*ret_x = x + delta;
		*ret_y = y + delta;
		break;
	case WD_S: 
		*ret_x = x + 0;
		*ret_y = y + delta;
		break;
	case WD_SW: 
		*ret_x = x - delta;
		*ret_y = y + delta;
		break;
	case WD_W: 
		*ret_x = x - delta;
		*ret_y = y + 0;
		break;
	case WD_NW: 
		*ret_x = x - delta;
		*ret_y = y - delta;
		break;
	default: abort();
	}
}

bool in_bounds (const unsigned int w, const unsigned int h, const int x, const int y) {
	return
		0 <= x && x < (int)w &&
		0 <= y && y < (int)h;
}

bool word_in_bounds (const unsigned int w, const unsigned int h, const int x, const int y, const unsigned int l, const word_dir_t d) {
	int dst_x, dst_y;

	transform(x, y, d, l, &dst_x, &dst_y);
	return in_bounds(w, h, x, y) && in_bounds(w, h, dst_x, dst_y);
}

void print_word (const map_t *map, const char *word, const int x, const int y, const word_dir_t dir, const bool show_on_map) {
	printf(
		"   -\n"
		"      word: %s\n"
		"      coor: (%d, %d)\n"
		"      dir: %s\n",
		word,
		x, y,
		str_word_dir(dir)
		);
	if (show_on_map) {
		// TODO
	}
}

char pull_char (const map_t *map, const int x, const int y) {
	return map->data[y*map->w + x];
}

bool search_word (const map_t *map, const int x, const int y, const char *word, const word_dir_t dir) {
	const unsigned int word_len = (unsigned int)strlen(word);
	char buf[MAP_DIM_LIMIT];

	assert(word_len > 0);

	if (!word_in_bounds(map->w, map->h, x, y, word_len - 1, dir)) {
		return false;
	}

	for (unsigned int i = 0; i < word_len; i += 1) {
		int dst_x, dst_y;

		transform(x, y, dir, i, &dst_x, &dst_y);
		buf[i] = pull_char(map, dst_x, dst_y);
	}

	return memcmp(buf, word, word_len) == 0;
}

/*
* File format:
*	<map width> <map height>
* 	<number of words>
*	<word 1>
*	<word 2>
*	<word ...>
*	<map row 1>
*	<map row 2>
*	<map row ...>
*/
int main (const int argc, const char **args) {
	int ret = 1;
	map_t map;
	size_t nb_words;
	char **word_list = NULL;
	unsigned int *occur_list = NULL;
	char line_buf[MAP_DIM_LIMIT + 1];
	size_t line_len;
	FILE *in_f = NULL;

	map.data = NULL;

	if (argc <= 1) {
		// TODO
		ret = 2;
		goto END;
	}
	else if (argc >= 2 && strcmp(args[1], "-") != 0) {
		in_f = fopen(args[1], "rb");
		if (in_f == NULL) {
			perror("Failed to open map file");
			goto END;
		}
	}
	else {
		in_f = stdin;
	}

	// load file
	if (fscanf(in_f, "%u %u %zu", &map.w, &map.h, &nb_words) != 3) {
		fprintf(stderr, "Format error whilst reading map size.\n");
		goto END;
	}
	if (map.w > MAP_DIM_LIMIT || map.h > MAP_DIM_LIMIT) {
		fprintf(stderr, "Format error: map too large(limit: %u)\n", MAP_DIM_LIMIT);
		ret = 2;
		goto END;
	}
	if (nb_words > WORD_LIST_LIMIT) {
		fprintf(stderr, "Format error: too many words(limit: %zu)\n", nb_words);
		ret = 2;
		goto END;
	}

	// load words 
	word_list = (char**)calloc(sizeof(char*), nb_words);
	occur_list = (unsigned int*)calloc(sizeof(unsigned int), nb_words);
	if (word_list == NULL || occur_list == NULL) {
		perror("Error allocating memory for words");
		goto END;
	}
	for (size_t i = 0; i < nb_words; i += 1) {
		if (fscanf(in_f, "%s", line_buf) != 1) {
			fprintf(stderr, "Format error whilst reading words.\n");
			goto END;
		}
		line_len = strlen(line_buf);
		word_list[i] = (char*)malloc(line_len + 1);
		if (word_list[i] == NULL) {
			perror("Error allocating memory for words");
			goto END;
		}
		strcpy(word_list[i], line_buf);
		
		// convert to uppercase
		for (size_t j = 0; j < line_len; j += 1) {
			if (!isalnum(word_list[i][j])) {
				fprintf(stderr, "Format error: word \"\" contains alpha-numberic character\n");
				ret = 2;
				goto END;
			}
			word_list[i][j] = (char)toupper(word_list[i][j]);
		}
	}

	// load map
	map.data = (char*)calloc(map.w, map.h);
	if (map.data == NULL) {
		perror("Error allocating memory for map");
		goto END;
	}
	{
		char *pos;
		char *const end = map.data + map.w*map.h;

		for (pos = map.data; fscanf(in_f, "%s", line_buf) == 1; pos += line_len) {
			line_len = strlen(line_buf);

			if (pos + line_len > end) {
				fprintf(stderr, "Format error: map larger than specified");
				ret = 2;
				goto END;
			}

			// accept alphanumberic values only
			// convert to upper
			for (size_t i = 0; i < line_len; i += 1) {
				if (!isalnum(line_buf[i])) {
					fprintf(stderr, "Format error: non-alphanumberic character found in map");
					ret = 2;
					goto END;
				}
				line_buf[i] = toupper(line_buf[i]);
			}
			
			// append
			memcpy(pos, line_buf, line_len);
		}

		if (pos != end) {
			fprintf(stderr, "Format error: map smaller than specified");
			ret = 2;
			goto END;
		}
	}

	// done loading
	// close file as it might take a while to process
	fclose(in_f);
	in_f = NULL;

	// perform search
	printf("result:\n");

	for (unsigned int x = 0; x < map.w; x += 1) {
		for (unsigned int y = 0; y < map.h; y += 1) {
			for (word_dir_t dir = WD_NONE + 1; dir < NB_WD; dir += 1) {
				for (size_t word = 0; word < nb_words; word += 1) {
					if (search_word(&map, x, y, word_list[word], dir)) {
						print_word(&map, word_list[word], x, y, dir, true);
						occur_list[word] += 1;
					}
				}
			}
		}
	}

	// stat
	printf("stat:\n");
	for (size_t i = 0; i < nb_words; i += 1) {
		printf("   %s: %u\n", word_list[i], occur_list[i]);
	}

	ret = 0;
END:
	if (in_f != NULL) {
		fclose(in_f);
		in_f = NULL;
	}

	if (word_list != NULL) {
		for (size_t i = 0; i < nb_words; i += 1) {
			free(word_list[i]);
		}
		free(word_list);
		word_list = NULL;
	}
	free(occur_list);
	
	free(map.data);
	map.data = NULL;

	return ret;
}

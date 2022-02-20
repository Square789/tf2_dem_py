#include <stdio.h>
#include <stdlib.h>

#include "tf2_dem_py/demo_parser/parser_state.h"

int main(int nargs, char **args) {
	if (nargs != 2) {
		printf("You must specify the demo's filename as only argument!\n");
		return EXIT_FAILURE;
	}

	ParserState *parser_state;
	FILE *fp;

	fp = fopen(args[1], "rb");
	if (fp == NULL) {
		printf("Failed opening file!\n");
		goto file_error;
	}

	parser_state = ParserState_new();
	if (parser_state == NULL) {
		printf("Failed creating parser state.\n");
		goto parser_creation_error;
	}

	ParserState_read_demo_header(parser_state, fp);
	if (parser_state->failure != 0) {
		goto parser_error;
	}

	while (!parser_state->finished) {
		ParserState_parse_packet(parser_state, fp);
		if (parser_state->failure != 0) {
			goto parser_error;
		}
	}

	if (parser_state->print_msg != NULL) {
		printf("%s ok\n", parser_state->print_msg);
	}

	ParserState_destroy(parser_state);
	fclose(fp);

#if defined(_WIN32)
	system("pause");
#elif defined(__linux__)
	// __unix__ may also work but:
	//	No one besides me is going to ever use this program or even read this comment
	//	I will never use unix
	// yoink: https://stackoverflow.com/questions/92802/what-is-the-linux-equivalent-to-dos-pause
	system("read -n1 -r -p \"Press any key to continue...\"")
#else
	// do nothing idk
#endif

	return EXIT_SUCCESS;

parser_error:
	ParserState_destroy(parser_state);
parser_creation_error:
	fclose(fp);
file_error:

	printf("Failure\n");
	return EXIT_FAILURE;
}

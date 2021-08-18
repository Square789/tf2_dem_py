#include <stdio.h>
#include <stdlib.h>

#include "tf2_dem_py/demo_parser/packet/parse_any.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

int main(int nargs, char **args) {
	CharArrayWrapper_err_t caw_err;

	if (nargs != 2) {
		printf("You must specify the demo's filename as only argument!\n");
		return EXIT_FAILURE;
	}

	FILE *fp = fopen(args[1], "rb");
	if (fp == NULL) {
		printf("Failed opening file!\n");
		goto file_error;
	}

	ParserState *parser_state = ParserState_new();
	if (parser_state == NULL) {
		printf("Failed creating parser state.\n");
		goto parser_creation_error;
	}

	switch (DemoHeader_read(parser_state->demo_header, fp, &caw_err)) {
	case 1:
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		break;
	case 2:
		parser_state->failure |= ParserState_ERR_CAW;
		parser_state->RELAYED_CAW_ERR = caw_err;
		break;
	default:
		break;
	}
	if (parser_state->failure != 0) {
		goto parser_error;
	}
	while (!parser_state->finished) {
		packet_parse_any(fp, parser_state);
		if (parser_state->failure != 0) {
			goto parser_error;
		}
	}

	if (parser_state->print_msg != NULL) {
		printf("%s ok\n", parser_state->print_msg);
	}

	ParserState_destroy(parser_state);
	fclose(fp);

	return EXIT_SUCCESS;

parser_error:
	ParserState_destroy(parser_state);
parser_creation_error:
	fclose(fp);
file_error:
	return EXIT_FAILURE;
}

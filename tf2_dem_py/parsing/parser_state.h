#ifndef PARSER_STATE__H
#define PARSER_STATE__H

#include <stdint.h>

typedef struct ParserState {
	uint8_t finished;
	uint8_t FAILURE;
	uint8_t RELAYED_CAW_ERR;

} ParserState;

#endif

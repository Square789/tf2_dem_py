#ifndef PARSER_STATE__H
#define PARSER_STATE__H

#include <stdint.h>

typedef struct ParserState {
	uint16_t flags;
	uint8_t finished;
	uint8_t FAILURE;
	uint8_t RELAYED_CAW_ERR;
	uint32_t tick;
} ParserState;

#endif

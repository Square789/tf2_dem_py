#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state.h"

#include "tf2_dem_py/demo_parser/message/various.h"

void Empty_parse(CharArrayWrapper *caw, ParserState *parser_state) {}
void Empty_skip(CharArrayWrapper *caw, ParserState *parser_state) {}


void File_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	File_skip(caw, parser_state);
}

void File_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 4, 0);
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
	CharArrayWrapper_skip(caw, 0, 1);
}


void NetTick_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	uint32_t tick = CharArrayWrapper_get_uint32(caw);
	//uint16_t frame_time = caw->get_uint16();
	CharArrayWrapper_skip(caw, 2, 0);
	//uint16_t std_dev = caw->get_uint16();
	CharArrayWrapper_skip(caw, 2, 0);

	parser_state->tick = tick;
}

void NetTick_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 8, 0);
}


void StringCommand_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	StringCommand_skip(caw, parser_state);
}

void StringCommand_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	// char *tmp = CharArrayWrapper_get_nulltrm_str(caw);
	// if (tmp != NULL) { printf("StringCommand: %s\n", tmp); }
	// free(tmp);
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
}


void SetConVar_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	SetConVar_skip(caw, parser_state);
}

void SetConVar_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t amt = CharArrayWrapper_get_uint8(caw);
	for (uint16_t i = 0; i < amt; i++) {
		// char *tmp1 = CharArrayWrapper_get_nulltrm_str(caw);
		// char *tmp2 = CharArrayWrapper_get_nulltrm_str(caw);
		// if (tmp1 != NULL && tmp2 != NULL) { printf("SetConVar: %s ||| %s\n", tmp1, tmp2); }
		// free(tmp1);
		// free(tmp2);
		CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
		CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
	}
}


void SigOnState_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	SigOnState_skip(caw, parser_state);
}

void SigOnState_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 5, 0);
}


void Print_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	// Unlikely and weird, but skip it in this case.
	// TODO: or don't. It seems as if multiple print messages are not too unusual,
	// maybe throw them into an arraylist.
	if (parser_state->print_msg != NULL) {
		Print_skip(caw, parser_state);
		return;
	}

	parser_state->print_msg = CharArrayWrapper_get_nulltrm_str(caw);

	if (parser_state->print_msg == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
	}
}

void Print_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
}


void ServerInfo_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	// This would mean the packet is duplicated. No reason of happening, but just ignore then.
	if (parser_state->server_info != NULL) { ServerInfo_skip(caw, parser_state); }

	parser_state->server_info = ServerInfo_new();
	if (parser_state->server_info == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return;
	}

	// mallocs in there may fail, but will be caught by outer CAW error check.
	ServerInfo_read(parser_state->server_info, caw);
}

void ServerInfo_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 36, 2);
	for (uint8_t _ = 0; _ < 4; _++) {
		CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
	}
	CharArrayWrapper_skip(caw, 0, 1);
}


void SetView_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	SetView_skip(caw, parser_state);
}

void SetView_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 1, 3);
}


void FixAngle_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	FixAngle_skip(caw, parser_state);
}

void FixAngle_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 6, 1);
}


void BspDecal_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	BspDecal_skip(caw, parser_state);
}

void BspDecal_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t existing_coords[3] = {0, 0, 0};
	uint8_t i;
	for (i = 0; i < 3; i++) {
		existing_coords[i] = CharArrayWrapper_get_bit(caw);
	}
	for (i = 0; i < 3; i++) {
		if (existing_coords[i] == 1) {
			CharArrayWrapper_get_bit_coord(caw);
		} 
	}
	CharArrayWrapper_skip(caw, 1, 1);
	if (CharArrayWrapper_get_bit(caw) == 1) {
		CharArrayWrapper_skip(caw, 3, 0);
	}
	CharArrayWrapper_skip(caw, 0, 1);
}


void Entity_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	Entity_skip(caw, parser_state);
}

void Entity_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 2, 4);
	uint16_t length = 0;
	CharArrayWrapper_read_raw(caw, &length, 1, 3);
	CharArrayWrapper_skip(caw, length / 8, length % 8);
}


void PreFetch_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	PreFetch_skip(caw, parser_state);
}

void PreFetch_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 1, 6);
}


void GetCvarValue_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	GetCvarValue_skip(caw, parser_state);
}

void GetCvarValue_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 4, 0);
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
}

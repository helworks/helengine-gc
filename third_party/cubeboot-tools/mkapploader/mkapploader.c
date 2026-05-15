/**
 * mkapploader.c
 *
 * Wraps a raw apploader code image with a GameCube apploader header.
 *
 * Copyright (C) 2026 Helengine contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *DefaultApploaderDate = "2006/01/06";
static uint32_t align32(uint32_t value)
{
	return (value + 31U) & ~31U;
}

static void write_be32(unsigned char *bytes, uint32_t value)
{
	bytes[0] = (unsigned char)((value >> 24) & 0xff);
	bytes[1] = (unsigned char)((value >> 16) & 0xff);
	bytes[2] = (unsigned char)((value >> 8) & 0xff);
	bytes[3] = (unsigned char)(value & 0xff);
}

int main(int argc, char **argv)
{
	FILE *input;
	FILE *output;
	char *entry_end;
	long raw_length_long;
	uint32_t raw_length;
	uint32_t padded_length;
	uint32_t entry_point;
	unsigned char *raw_bytes;
	unsigned char header[0x20];

	if (argc != 4) {
		fprintf(stderr, "usage: mkapploader <raw-input> <output> <entry-point-hex>\n");
		return 1;
	}

	entry_point = (uint32_t)strtoul(argv[3], &entry_end, 0);
	if (entry_end == argv[3] || *entry_end != '\0') {
		fprintf(stderr, "invalid entry point '%s'\n", argv[3]);
		return 1;
	}

	input = fopen(argv[1], "rb");
	if (input == NULL) {
		fprintf(stderr, "could not open input '%s': %s\n", argv[1], strerror(errno));
		return 1;
	}

	if (fseek(input, 0, SEEK_END) != 0) {
		fprintf(stderr, "could not seek input '%s'\n", argv[1]);
		fclose(input);
		return 1;
	}

	raw_length_long = ftell(input);
	if (raw_length_long < 0) {
		fprintf(stderr, "could not determine input size '%s'\n", argv[1]);
		fclose(input);
		return 1;
	}

	if (fseek(input, 0, SEEK_SET) != 0) {
		fprintf(stderr, "could not rewind input '%s'\n", argv[1]);
		fclose(input);
		return 1;
	}

	raw_length = (uint32_t)raw_length_long;
	padded_length = align32(raw_length);
	raw_bytes = (unsigned char *)calloc(padded_length, 1);
	if (raw_bytes == NULL) {
		fprintf(stderr, "could not allocate %u bytes for apploader\n", padded_length);
		fclose(input);
		return 1;
	}

	if (fread(raw_bytes, 1, raw_length, input) != raw_length) {
		fprintf(stderr, "could not read input '%s'\n", argv[1]);
		free(raw_bytes);
		fclose(input);
		return 1;
	}

	fclose(input);

	memset(header, 0, sizeof(header));
	memcpy(header, DefaultApploaderDate, 10);
	write_be32(header + 0x10, entry_point);
	write_be32(header + 0x14, padded_length);
	write_be32(header + 0x18, 0);
	write_be32(header + 0x1c, 0);

	output = fopen(argv[2], "wb");
	if (output == NULL) {
		fprintf(stderr, "could not open output '%s': %s\n", argv[2], strerror(errno));
		free(raw_bytes);
		return 1;
	}

	if (fwrite(header, 1, sizeof(header), output) != sizeof(header)) {
		fprintf(stderr, "could not write header to '%s'\n", argv[2]);
		free(raw_bytes);
		fclose(output);
		return 1;
	}

	if (fwrite(raw_bytes, 1, padded_length, output) != padded_length) {
		fprintf(stderr, "could not write image to '%s'\n", argv[2]);
		free(raw_bytes);
		fclose(output);
		return 1;
	}

	free(raw_bytes);
	fclose(output);
	return 0;
}

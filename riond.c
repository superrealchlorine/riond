#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 49130
#define MAGIC_BYTE 0x43
#define FRAME_CODE_STREAM 0x02
#define MAX_PAYLOAD_LEN (16 * 1024 * 1024) /* 16 MB limit */

struct __attribute__((__packed__)) chloride_header {
	uint8_t magic;
	uint8_t frame_type;
	uint16_t flags;
	uint32_t payload_len;
};

static int send_all(int fd, const void *buf, uint32_t len) {
	const uint8_t *p = buf;
	uint32_t sent = 0;

	while (sent < len) {
		ssize_t n = send(fd, p + sent, len - sent, 0);
		if (n < 0 && errno == EINTR)
			continue;
		if (n <= 0)
			return -1;
		sent += (uint32_t)n;
	}
	return 0;
}

static int send_chloride_frame(int fd, uint8_t type, const uint8_t *payload, uint32_t len) {
	struct chloride_header hdr;

	hdr.magic = MAGIC_BYTE;
	hdr.frame_type = type;
	hdr.flags = htons(0);
	hdr.payload_len = htonl(len);

	if (send_all(fd, &hdr, sizeof(hdr)) < 0)
		return -1;

	if (len > 0 && payload != NULL) {
		if (send_all(fd, payload, len) < 0)
			return -1;
	}

	return 0;
}

static uint8_t *read_wasm_file(const char *filename, uint32_t *out_size) {
	FILE *f = fopen(filename, "rb");
	if (!f) {
		perror("fopen");
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (size <= 0 || size > MAX_PAYLOAD_LEN) {
		fprintf(stderr, "Error: Invalid or oversized WASM file size (%ld bytes)\n", size);
		fclose(f);
		return NULL;
	}

	uint8_t *buffer = malloc((size_t)size);
	if (!buffer) {
		fclose(f);
		return NULL;
	}

	if (fread(buffer, 1, (size_t)size, f) != (size_t)size) {
		fprintf(stderr, "Error reading file data\n");
		free(buffer);
		fclose(f);
		return NULL;
	}

	fclose(f);
	*out_size = (uint32_t)size;
	return buffer;
}

int main(int argc, char *argv[]) {
	int server_fd, client_fd;
	struct sockaddr_in address;
	int opt = 1;
	socklen_t addrlen = sizeof(address);
	uint8_t *wasm_data = NULL;
	uint32_t wasm_size = 0;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <path_to_wasm_file>\n", argv[0]);
		return 1;
	}

	wasm_data = read_wasm_file(argv[1], &wasm_size);
	if (!wasm_data) {
		return 1;
	}

	printf("[riond] Loaded %s (%u bytes)\n", argv[1], wasm_size);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket failed");
		free(wasm_data);
		return 1;
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt failed");
		close(server_fd);
		free(wasm_data);
		return 1;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		close(server_fd);
		free(wasm_data);
		return 1;
	}

	if (listen(server_fd, 5) < 0) {
		perror("listen failed");
		close(server_fd);
		free(wasm_data);
		return 1;
	}

	printf("[riond] Listening on port %d...\n", PORT);

	while (1) {
		client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
		if (client_fd < 0) {
			if (errno == EINTR)
				continue;
			perror("accept failed");
			break;
		}

		printf("[riond] Client connected. Streaming CODE_STREAM frame...\n");

		if (send_chloride_frame(client_fd, FRAME_CODE_STREAM, wasm_data, wasm_size) == 0) {
			printf("[riond] Successfully sent %u bytes.\n", wasm_size);
		} else {
			fprintf(stderr, "[riond] Failed to stream frame to client.\n");
		}

		close(client_fd);
	}

	close(server_fd);
	free(wasm_data);
	return 0;
}

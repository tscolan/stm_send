#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <termios.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#define CHUNK_LEN 32

int open_serial_port(const char * device) {
	int fd = open(device, O_RDWR | O_NOCTTY);
	if (fd == -1) {
		return 0;
	}

	tcflush(fd, TCIOFLUSH);

	struct termios options;
	unsigned char result = tcgetattr(fd, &options);
	if (result) {
		close(fd);
		return 0;
	}

	options.c_iflag &= ~(INLCR | IGNCR | ICRNL | IXON | IXOFF);
	options.c_oflag &= ~(ONLCR | OCRNL);
	options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

	options.c_cc[VTIME] = 10; //10=1s
	//options.c_cc[VMIN] = 0;

	cfsetospeed(&options, B9600);
	cfsetispeed(&options, B9600);

	result = tcsetattr(fd, TCSANOW, &options);
	if (result) {
		close(fd);
		return 0;
	}

	return fd;
}

int Expect(int sd, char c) {
	char tmp;
	do {

		ssize_t r = read(sd, &tmp, 1);

		if (r < 0) {
			return 0;
		} else if (r == 0) {
			return 0;
		} 
	} while (c != tmp);

	return 1;
}

int SendCmd(int sd, char cmd,unsigned char argslen, char * args,unsigned char retlen, char * ret) {
	unsigned char tmp;

	tmp = write(sd, "*", 1);
	if (tmp != 1) {
		return 0;
	}

	tmp = write(sd, &cmd, 1);
	if (tmp != 1) {
		return 0;
	}

	if (argslen) {
		tmp = write(sd, args, argslen);
		if (tmp != argslen) {
			return 0;
		}
	}

	tmp = 0;
	while (tmp < retlen) {
		if (read(sd, &ret[tmp], 1) != 1) {
			return 0;
		}
		tmp++;
	}

	if (cmd != 'G') {
		tmp = Expect(sd, '*');
		if (tmp != 1) {
			return 0;
		}

		tmp = Expect(sd, cmd);
		if (tmp != 1) {
			return 0;
		}

		tmp = Expect(sd, 0x0D);
		if (tmp != 1) {
			return 0;
		}

		tmp = Expect(sd, 0x0A);
		if (tmp != 1) {
			return 0;
		}
	} else {
		tcdrain(sd); // Wait until transmission ends
	}

	return 1;
}

int main(int argc, char ** argv) {
	const char * device = "/dev/ttyUSB0";
	uint32_t address = 0x00044000;

	if (argc < 2) {
		printf("Usage: %s binaryfile [address]\n",basename(argv[0]));
		return 1;
	}

	if (argc > 2) {
		address = strtol(argv[2], NULL, 16);
		address &= ~1; //Even only
	}

	char * filename = malloc(PATH_MAX);
	realpath(argv[1], filename);
	printf("Address : %08X\n",address);
	printf("Filename : %s\n",filename);

	if( access( filename, F_OK ) != 0 ) {
		printf("Invalid input file\n");
		return 1;
	}

	FILE * fd;
	fd = fopen(filename, "rb");
	free(filename);

	if( !fd ) {
		printf("Input file opening error\n");
		return 1;
	}
	fseek(fd, 0L, SEEK_END);
	unsigned int sz = ftell(fd);
	rewind(fd);

	printf("Input file size : %i\n",sz);

	char * hexdata = malloc(2*CHUNK_LEN + 1);

	int sd = open_serial_port(device);
	if ( !sd ) 
		return 1;

	for (unsigned int i=0; i<32; i++)
		write(sd, " ", 1);

	if (!SendCmd(sd, 'V', 0, NULL, 0, NULL)) {
		printf("Communication error (1)\n");
		return 1;
	}

	printf("STM Found\n");

	if (!SendCmd(sd, 'A', 0, NULL, 0, NULL)) {
		printf("Communication error (2)\n");
		return 1;
	}

	char tmpstr[9];
	sprintf(tmpstr,"%08X",address);
	if (!SendCmd(sd, 'L', 8, tmpstr, 0, NULL)) {
		printf("Communication error (3)\n");
		return 1;
	}

	unsigned int send = 0;
	char * tmp = hexdata;
	while (!feof(fd)) {
		unsigned char c = fgetc(fd);
		sprintf(tmp,"%02X",c);
		tmp+=2;

		if (strlen(hexdata) == 2*CHUNK_LEN) {
			sprintf(tmpstr,"%04X",CHUNK_LEN);
			if (!SendCmd(sd, 'B', 4, tmpstr, 0, NULL)) {
				printf("Communication error (4)\n");
				return 1;
			}

			if (!SendCmd(sd, 'D', strlen(hexdata), hexdata, 0, NULL)) {
				printf("Communication error (5)\n");
				return 1;
			}

			printf("\r%i/%i",send,sz);
			fflush(stdout);

			tmp=hexdata;
			tmp[0] = 0;
		}
		send++;
	}
	send--;
	if (strlen(hexdata)) {
		sprintf(tmpstr,"%04X",strlen(hexdata)/2);
		if (!SendCmd(sd, 'B', 4, tmpstr, 0, NULL)) {
			printf("Communication error (6)\n");
			return 1;
		}

		if (!SendCmd(sd, 'D', strlen(hexdata), hexdata, 0, NULL)) {
			printf("Communication error (7)\n");
			return 1;
		}

		printf("\r%i/%i",send,sz);
		fflush(stdout);
	}

	printf("\nSend complete.\n");

	sprintf(tmpstr,"%08X",address);

	printf("Launch code at 0x%s ...\n",tmpstr);

	if (!SendCmd(sd, 'G', 8, tmpstr, 0, NULL)) {
		printf("Communication error (8)\n");
		return 1;
	}

	fclose(fd);
	close(sd);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>

char get_name_for_scancode(char scancode)
{
	switch (scancode)
	{
	case 0x55:
		return '1';
	case 0x4c:
		return '2';
	case 0x44:
		return '3';
	case 0x6e:
		return '4';
	case 0x6f:
		return '5';
	case 0x70:
		return '6';
	case 0x2a:
		return '7';
	case 0x39:
		return '8';
	case 0x38:
		return '9';
	case 0x31:
		return '*';
	case 0x32:
		return '0';
	case 0x33:
		return '#';
	case 0x43:
		return '-';
	case 0x52:
		return '+';
	case 0x29:
		return 'H'; // Home
	case 0x37:
		return 'M'; // Mute
	case 0x3e:
		return 'S'; // Speaker phone
	case 0x3c:
		return 'Z'; // Portable headset
	case 0x4b:
		return 'P'; // Phone 'on dock' sensor button
	}

	return 0;
}

/*
	scancode = unique code per button
	press_down = 1 on keypress, 0 on key release
*/
void handle_scancode(char scancode, char press_down)
{
	char codeName = get_name_for_scancode(scancode);
	if (codeName != 0)
	{
		if (press_down == 1)
		{
			printf("Key %c pressed\n", codeName);
		}
		else if (press_down == 0)
		{
			printf("Key %c released\n", codeName);
		}
		else
		{
			printf("Unknown key: scancode=0x%02x, keyCodeName='%c', press_down=0x%02x\n", scancode, codeName, press_down);
		}
	}
	else
	{
		printf("Unknown key: scancode=0x%02x, press_down=0x%02x\n", scancode, press_down);
	}
}

int main()
{
	printf("Running!\n");
	struct pollfd fds;
	fds.fd = 0; /* this is STDIN */
	fds.events = POLLIN;

	int loop = 1;
	while (1)
	{
		int num = 0;
		while (1)
		{
			int ret = poll(&fds, 1, 0);
			if (ret == 1)
			{
				char ch[16];
				read(STDIN_FILENO, &ch, 16);

				handle_scancode(ch[10], ch[12]);

				num++;
			}
			else if (ret == 0)
			{
				/* No error, no input*/
				break;
			}
			else
			{
				printf("Error\n");
				break;
			}
		}

		loop++;
		usleep(1000 * 50); // sleep for 50ms
	}

	return 0;
}

#include <string.h>


// From Debug.c of N90 Buddy for Palm
void dataToHex(unsigned char *input, int count, char *output, const char *prefix) {
	unsigned char	byte;
	unsigned char nibble;
	int	wrap = 1;
	
	if (prefix)
	{
		strcpy(output, prefix);
		output += strlen(prefix);
	}
	
	while (count--)
	{
		byte = *input++;
		nibble = byte >> 4;
		*output++ = nibble + ((nibble < 0x0A) ? '0' : 'A' - 0x0A);
		nibble = byte & 0x0F;	
		*output++ = nibble + ((nibble < 0x0A) ? '0' : 'A' - 0x0A);
		if ((wrap % 8) == 0)
		{
			*output++ = 0x0A;
			if (prefix && (count > 1))
			{
				strcpy(output, prefix);
				output += strlen(prefix);
			}
		}
		else
			*output++ = ' ';
		wrap++;
	}
	output[-1] = '\0';
}

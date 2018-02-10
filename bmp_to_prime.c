#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#include "bmp.h"

int main (int argc, char *argv[])
{
	// ensure proper usage
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
		return 1;
	}

	// remember filenames
	char *infile = argv[1];
	char *outfile = argv[2];

	// open input file 
	FILE *inptr = fopen(infile, "r");
	if (inptr == NULL)
	{
		fprintf(stderr, "Could not open %s.\n", infile);
		return 2;
	}

	// read infile's BITMAPFILEHEADER
	BITMAPFILEHEADER bf;
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

	// read infile's BITMAPINFOHEADER
	BITMAPINFOHEADER bi;
	fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

	// ensure infile is (likely) a 24-bit uncompressed BMP 4.0
	if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || bi.biBitCount != 24 || bi.biCompression != 0)
	{
		fclose(inptr);
		fprintf(stderr, "Unsupported file format.\n");
		return 3;
	}

	// determine padding for scanlines
	int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

	int bmp_size = bi.biWidth * abs(bi.biHeight);

	char *bmp_numbers = (char*)malloc(bmp_size + 1);
	if (bmp_numbers == NULL)
	{
		fclose(inptr);
		fprintf(stderr, "Problem while allocating memory.\n");
		return 4;
	}

	bmp_numbers[bmp_size] = '\0';

	// iterate over infile's scanlines
	for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
	{
		// iterate over pixels in scanline
		for (int j = 0; j < bi.biWidth; j++)
		{
			// temporary storage
			RGBTRIPLE triple;

			// read RGB triple from infile
			fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

			// calculate luminance
			// https://en.wikipedia.org/wiki/Relative_luminance
			double L = 0.2126 * triple.rgbtRed + 0.7152 * triple.rgbtGreen + 0.0722 * triple.rgbtBlue;

			// Write the character
			bmp_numbers[i * bi.biWidth + j] = L < 200 ? '8' : '1';
		}

		// skip over padding, if any
		fseek(inptr, padding, SEEK_CUR);
	}
	// close infile
	fclose(inptr);

	char *bmp_topdown;
	if (bi.biHeight < 0)
	{
		bmp_topdown = (char*)malloc(bmp_size + 1);
		if (bmp_topdown == NULL)
		{
			free(bmp_numbers);
			fprintf(stderr, "Problem while allocating memory.\n");
			return 5;
		}
		bmp_topdown[bmp_size] = '\0';

		// iterate over character lines top-down
		for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
		{
			// iterate over characters in lines
			for (int j = 0; j < bi.biWidth; j++)
			{
				bmp_topdown[i * bi.biWidth + j] = bmp_numbers[(abs(bi.biHeight) - i - 1) * bi.biWidth + j];
			}
		}
		free(bmp_numbers);
	}
	else
	{
		bmp_topdown = bmp_numbers;
	}

	int valid;

	// create an integer for characters
	mpz_t noprime;
	valid = mpz_init_set_str(noprime, bmp_topdown, 10);
	free(bmp_topdown);
	if (valid == -1)
	{
		mpz_clear(noprime);
		fprintf(stderr, "String is not a valid number.\n");
		return 6;
	}

	// create an integer for the prime
	mpz_t next_prime;
	mpz_init(next_prime);

	mpz_nextprime(next_prime, noprime);
	mpz_clear(noprime);

	FILE *outptr = fopen(outfile, "w");
	if (outptr == NULL)
	{
		mpz_clear(next_prime);
		fprintf(stderr, "Could not open %s.\n", outfile);
		return 7;
	}

	valid = gmp_fprintf(outptr, "<!DOCTYPE html>\n<html lang=\"en\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <title>%s</title>\n  <meta name=\"author\" content=\"\">\n  <meta name=\"description\" content=\"\">\n  <style>\n  @font-face {\n    font-family: Square;\n	 src: url('./square.woff');\n  }\n	</style>\n	<script>\n	(function(window, document, undefined) {\n	  function addNewlines(str, lngth) {\n		var result = '';\n		while (str.length > 0) {\n		  result += str.substring(0, lngth) + '<br>';\n		   str = str.substring(lngth);\n	  }\n	   return result;\n    }\n	  var prime = '%Zd';\n	  window.onload = init;\n\n    function init() {\n		var element = document.getElementById('prime');\n	   element.innerHTML = addNewlines(prime, %d) + '<br> is a %d digit number and a prime.';\n    }\n\n  })(window, document, undefined);\n  </script>\n</head>\n\n<body>\n  <div id=\"prime\" style=\"font-family: Square; font-size:9px;\"></div>\n</body>\n</html>", outfile, next_prime, bi.biWidth, mpz_sizeinbase(next_prime, 10));
	// close outfile
	fclose(outptr);
	mpz_clear(next_prime);
	if (valid == -1)
	{
		fprintf(stderr, "Can not write to file %s.\n", outfile);
		return 8;
	}

	printf("%d characters written to file.\n", valid);

	return 0;
}

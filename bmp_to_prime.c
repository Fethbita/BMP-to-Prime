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
	if (bi.biHeight > 0)
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
        char base64_part1[] = "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";
        char base64_part2[] = "5LvuCDqdqzS4FRd/6crLvi552en7XH+sa16n51q2C/mlK+ipoeS9/7BUWhXyQTv2ySvSqghuuyV09E6/yIXZqc5L2fQAV9KV844CauGbY5YZ6dh+yD6x6U8YFvHCJdcAC52Imr5eG53+v/SUGv4x+Az14HtuWbVcn9toH/T4N+o20A1fI9triDGLQnPgg4Ns+D6JPrX7DJbWgSdDv+TpvHeOJCq71+qVtLPDKpTyby2rHl4ACfrXHzkzolG3PZS7zuWsmVTNh06D9Jfa8KueMrvK78zUlTlXomjx389UNevEOemlSsdqtu6DdtKfdV3vazfUHEUStQf0Yv3e0G4+g3XBnT7t7OT6M+I7afdxJHAowvOsB2RoTxQ3Oruc3jXex7nb9Dfq98fcRsX4/or7Xe/rd+W4mM/6r/rONwBn3XDuT78WxmBtuOjFdNHrC7Uq7H1OfWcZF1O67gTVzld+K9aJZ9AJsZk3JEOmFuccrnXZfbrQ7q1YuaDesrlxSBu12VCYltDtfzx0t1KSPhcD5KAEUDVduPUO9Ztotq1TvYAZ6SlG0T+MJOatk5YJ267XlDZoCPbL0KCf28Qa9mAKo0G70sxCXv9Vf1l/+NtOAm19+k752Z6wptJtreRfe2kM90Une1M+B15wnI1ns1umB3Lz2xmCz+jp2rq/lbF4v6gW4hIsODzgCvt3r/1rYd6NoY+9r6EYVlF7X8SWN7fC2Gx94hPN4SZzFro5Sy4XlUGChvW11lnfSQ80MODzCTppf6fBbTcPLNH5RXPso4FiM89B8t3uj7Vk3+R0qvw/t3vuFLQfrdNTyzL5yDOnjezWs500U91eMIbCdGpgkvHcaPSu0KPTV0XEWLgbespXw4YWs5/8AYJu88IR+W8uvTf8Hs9Hb83icfZBBagIxFIb/6DggtC5tKS7mAB3JjLhw9l2KoKDgLmCwgk4wM+N9vEAP0BN014P0BN31N33dCDUhyfd+/rz3EgA9vEHhd/QxFFaIsRBukV+F23jEWTii/iHcwT2+hGP01AOdKuoyeg63LqxwhxfhFnkt3IaGE46ovwt38IRP4RgDfGNFX4MaFh4JTjAoec6oeqqHoFecljpWrqmtT06mTGbO1wfrq8pSX+DIJIZWy+DYGM9zyhybcJ+OqdtYf+VMMee+ZbwPSoqcbWcYocCEK8f4L1s6t9tmb3ya62xUTIp8/E/RpTS8Y/HLQzJ+vw4TS3a7c2WSDbXWtzu5qoofai5K+QAAAHicXci3TkIBAEDRw7NgF3uv2AUE7L13EbFjNw46OLi6+Wl+mdEwepabXIGCn29ffwn576VwA0WKlSgVVqZchUpVqtWoFVGnXoNGTZq1aNWmXYdOXbr16NWnX9SAQUOGjRg1ZlxMXMKEpJS0SVOmzZg1Z96CRUuWrVi1Zt2GTVu27di1Z9+BQ0cyjmWdyDl15tyFS1eu5d24defeg0dPnkNB+PX98+MtlfwF1iQXxQAAAAAAAf//AAJ4nGNgZGBg4AFiMSBmYmAEwiQgZgHzGAAH1ACRAAAAAQAAAADUGBYRAAAAAM5niUAAAAAAzomoTg==";

	valid = gmp_fprintf(outptr, "<!DOCTYPE html>\n<html lang=\"en\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <title>%s</title>\n  <meta name=\"author\" content=\"\">\n  <meta name=\"description\" content=\"\">\n  <style>\n    @font-face {\n      font-family: 'square';\n      src: url(data:application/x-font-woff;charset=utf-8;base64,%s%s) format('woff');\n    }\n\n    #prime {\n      font-family: 'square';\n      font-size: 9px;\n    }\n  </style>\n  <script>\n    (function(window, document, undefined) {\n      function addNewlines(str, lngth) {\n        var result = '';\n        while (str.length > 0) {\n          result += str.substring(0, lngth) + '<br>';\n          str = str.substring(lngth);\n        }\n        return result;\n      }\n      var prime = '%Zd';\n      window.onload = init;\n\n      function init() {\n        var element = document.getElementById('prime');\n        element.innerHTML = addNewlines(prime, %d) + '<br> is a %d digit number and a prime.';\n      }\n\n    })(window, document, undefined);\n  </script>\n</head>\n\n<body>\n  <div id=\"prime\"></div>\n</body>\n</html>\n", outfile, base64_part1, base64_part2, next_prime, bi.biWidth, mpz_sizeinbase(next_prime, 10));
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

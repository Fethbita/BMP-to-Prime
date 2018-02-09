# BMP-to-Prime
Creates a prime number from a bmp
## Getting Started
To compile: make
### Prerequisites
To build the project you need the [GMP](https://gmplib.org/) library.

Download the lastest release of GMP from: https://gmplib.org/ as tar.xz.
Extract it via 
```
tar -xf gmp-*.*.*.tar.xz
```
cd to new folder

```
cd gmp-*.*.*
./configure
make
sudo make install
make check
```
### Installing
To compile run 
```
make
```
## Running
To run, you need a bmp 24 bit BMP 4.0 file. It is preferred that it has 2 colors in it, white and any other color of your choice.
```
./bmp_to_prime file.bmp outfile.html
```
will give you an html file containing your prime number. BMP files are recommended to be under 100x100, in my own experience 130x140 took a long time.

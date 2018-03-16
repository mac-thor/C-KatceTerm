
# Description:

This is the "c" implementation of the KatCe terminal programm foramly written in Pascal.

# From source to binary:

##  configure:
```bash
./configure [--enable-ncurses]
```
Use "--enable-ncurses" to force the usage of GNU ncurses instead of std. curses.
Other standard autoconf options can be used. Run `./configure --help` to see all.

##  build:
```bash
make
```

##  install:
```bash
make install
```

# Usage
## katceterm
The main terminal program. Used to operate the KatCe computer and upload and download software and programms.

```bash
katceterm [-l "serial device"] [-s "serial line speed"]
```

## asczukat
This converts an ascii text file into the KatCE own text format.

```bash
asczukat [-i inputfile] -o outputfile
```
If the input file is ommited the ascii text file is read from <stdin>.

## katzuasc
This converts an KatCe text file into an ascii text file.

```bash
katceterm [-i inputfile] [-o outputfile]
```
If the input file is ommited the ascii text file is read from <stdin>.
And if the output file is ommited the converted file is send to <stdout>.


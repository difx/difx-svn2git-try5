# DiFX

The official repo for DiFX.

## Install

DiFX requires MPI, PGPLOT and IPP.

Details see https://www.atnf.csiro.au/vlbi/dokuwiki/doku.php/difx/installation.

To install the latest trunk version of DiFX, follow these simple instructions:

```bash
# make sure the setting in setup.bash is correct
$ source setup/setup.bash # (or .csh). 
#You will probably want to add this to your .bashrc or .cshrc files

# Help on options for that last bit are available with ./install-difx --help.
$ ./install-difx
```

### Debian / Ubuntu

Before install DiFX on Debian/Ubuntu ,you need install the following dependency:

```bash
$ apt-get update 
$ apt-get install -y build-essential subversion libopenmpi-dev libfftw3-dev libtool flex bison pgplot5 pkg-config automake libexpat1-dev gfortran openmpi-bin doxygen

```


The troubleshooting area on http://www.atnf.csiro.au/vlbi/dokuwiki/doku.php/difx/start

#### TESTING ####################

See http://www.atnf.csiro.au/dokuwiki/doku.php/difx/benchmarks

## Usage

See the userguide of DiFX.



## Maintainer

[@adamdeller](https://github.com/adamdeller)



## Contributing

Feel free to join us!  [Open an issue](https://github.com/difx/difx/issues/new) or submit PRs is always welcome.

Please follows the [Contributor Covenant](http://contributor-covenant.org/version/1/3/0/) Code of Conduct.

### Contributors

This project exists thanks to all the people who contribute. 
<a href="https://github.com/difx/difx/graphs/contributors"><img src="https://opencollective.com/difx/contributors.svg?width=890&button=false" /></a>

## License






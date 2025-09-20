# Zigned Distance Field

A 2D Signed Distance Field experiment
using fixed-point integers
(no `float`s allowed).

## Compiling Running

```sh
$ ./build.sh
$ ./outbin/main
```

`main` generates many `.ppm` files.

If you cannot read `.ppm`,
try running `pnmtopng ${in}.ppm > ${out}.png`.
The converter may be in package `netpbm`.

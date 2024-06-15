# TFTPp - c++ TFTP tool

## Building

Using Cmake

```bash
mkdir build
cd build
cmake .. -G "YourFavouriteGenerator"
```

Due to Native File Dialog use of gtk, on Linux you also have to install those packages:
```bash
pkg-config gtk-3-dev libsystemd-dev libwebp-dev libzstd-dev
```
Exact list will vary depending on your distro

## Licensing

TFTPp itself it distributed under MIT license.
As for submodules - I do not claim ownership of thier source code - see their respective license files.

## Todos

- [ ] Server functionality
    - [ ] Use tabs to separate client and server
- [ ] Better layout?
- [ ] Test on unix  - IT WON'T WORK FOR NOW
- [ ] Use better font
- [ ] Document and reformat the code - separate concerns

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
sudo apt install pkg-config libgtk-3-dev libsystemd-dev libwebp-dev libzstd-dev
```
Exact list will vary depending on your distro (ex. window-manager-specific libraries, etc.)

## Licensing

TFTPp itself it distributed under MIT license.
As for submodules - I do not claim ownership of thier source code - see their respective license files.

## Todos

- [ ] Server functionality
- [ ] Better layout?
- [X] Test on unix - tested on newest debian with wayland - works fine
- [ ] Use better font
- [ ] Documentation

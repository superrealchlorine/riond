# riond

Minimalist Chloride server that streams a single bare-metal Wasm binary.

## Build dependencies

- C99 compiler (gcc or clang, clang is required for compiling examples)
- make

On Debian/Ubuntu:
`sudo apt install build-essential`

## Runtime dependencies

N/A

## Building & Installation

`make`

`make examples` (optional, compiles examples)

`sudo make install`

## Systemd Service Setup

How to set up the riond service, using our hello_fb.wasm example application and hosting it over the Internet on port 49130.

`sudo mkdir -p /var/lib/riond`

`sudo cp hello_fb.wasm /var/lib/riond/app.wasm`

`sudo chmod 644 /var/lib/riond/app.wasm`


`sudo cp riond.service /etc/systemd/system/`

`sudo systemctl daemon-reload`

`sudo systemctl enable --now riond`

## Usage

`riond [path_to_wasm_file]`

example: `riond /var/lib/riond/app.wasm`

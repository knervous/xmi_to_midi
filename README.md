## XMI To Midi Converter

This tool was taken from EQEmu Tools, originally written by KLS, and modified to support output of multiple midi files and multiple targets (native and WebAssembly)

WASM Examples are found in the examples folder for nodejs/web

To get started, install cmake and run

`cmake .`

`cmake --build .`

To compile the native version

To compile the webassembly version, run

`cmake -DBUILD_WASM=ON .`

`cmake --build .`

## Examples

There are examples for NodeJS and web included. For NodeJS, simply run:

`cd examples && node node.js`

Which will run the script and convert the qeytoqrg.xmi to all midi outputs

For the web example, you will need to run a server, through python, nodejs, etc. If you are building emscripten and have the emsdk.sh sourced in your environment, you can use:

` cd examples && emrun index.html`

to use the built-in server from emscripten.
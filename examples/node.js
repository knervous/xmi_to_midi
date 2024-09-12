import XMICore from './lib/xmi_to_midi.js';
import fs from 'node:fs';

const XMI = await XMICore({
    print   : (msg) => console.debug(`[stdout] ${msg}`),
    printErr: (msg) => console.debug(`[stderr] ${msg}`),
})

const arrayBuffer = new Uint8Array(fs.readFileSync('./qeytoqrg.xmi').buffer);
const fileName = 'qeytoqrg';

// Step 1: Allocate memory for the file name (C string)
const namePtr = XMI._malloc(fileName.length + 1); // Allocate space for file name (+1 for null terminator)
XMI.stringToUTF8(fileName, namePtr, fileName.length + 1); // Copy string to WASM heap

// Step 2: Allocate memory for the array buffer (binary data)
const arrayBufferPtr = XMI._malloc(arrayBuffer.length); // Allocate space for the array buffer
XMI.HEAPU8.set(arrayBuffer, arrayBufferPtr); // Copy the JS buffer into the WASM memory (HEAPU8)

XMI._xmi_to_midi(namePtr, arrayBufferPtr, arrayBuffer.length);

// For compatibility we don't use node's raw filesystem with link flag -sNODERAWFS so we can also use in web, so let's take these out and write them to disk
const files = XMI.FS.readdir('./')
for (const file of files) {
    if (file.startsWith(fileName)) {
        const buffer = XMI.FS.readFile(file);
        fs.writeFileSync(file, buffer);
    }
}

console.log('Finished');
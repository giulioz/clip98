# clip98

![gif](https://media.giphy.com/media/2SFebJNbVQCHI1PoS2/giphy.gif)

**Learn more about this in my latest blog post: https://giuliozausa.dev/posts/bm-1-clipboard/**

Bidirectional clipboard sync between any Windows machine (from 95 to 10) and any other Node.js–capable machine.
You simply need to have the two machines connected with a serial port. To do so in QEMU you simply need to add `-serial pty` to the command line. The serial port dev path will be printed at vm start, if it's different than `ttys000` you may need to change it in `index.js`.

To build the Windows executable you may need Visual C++ 6.0. To run the Node.js script you simply need to run:

```
cd js
yarn install
yarn start
```

## Downloads

Latest CI built EXE for the guest: https://github.com/giulioz/clip98/releases/download/latest/clip98.exe

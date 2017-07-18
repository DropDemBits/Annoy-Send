# Annoy-Send
A program to send an entire file to a destination

## Building

### Building on Unixes (Linux, macOS, etc.)
Prerequisites: cmake, make
Run build.sh. The executable should be located in the build folder.

### Building on Windows
TODO

## Usage
```
annoy <file> [url] [port] [format] [cooldown]
```
+ <file> File to send
+ [url] URL to send to (Default <i>annoy.michaelreeves.us</i>)
+ [port] Port to connect to (Default <i>2888</i>)
+ [format] Format of sent block (Default <i>"GET /annoy?message=%s HTTP/1.0\r\n\r\n"</i>)
+ [cooldown] Delay between sending blocks (Default <i>6</i>)

For example, to send example.txt to annoy.michaelreeves.us:2888
```
annoy example.txt annoy.michaelreeves.us 2888
```
OR
```
annoy example.txt
```

## It's not working!
### (*nix) I can't execute the program.
Try doing build/annoy
If that doesn't work, then try running chmod +x build/annoy, then run build/annoy

### My problem isn't on here.
This <i>might</i> be a bug. Submit an issue on this repository.

## Whats left to do
+ Port to other operating systems (ie. Windows)
+ Cleanup code

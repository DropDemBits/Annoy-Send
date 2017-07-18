# Annoy-Send
A program to send an entire file to a destination

## Usage
```
annoy <file> [url] [port] [format] [cooldown]
```
+ <file> File to send
+ [url] URL to send to (Default <i>annoy.michaelreeves.us</i>)
+ [port] Port to connect to (Default <i>2888</i>)
+ [format] Format of sent block (Default <i>"GET /annoy?message=%s HTTP/1.0\r\n\r\n"</i>)
+ [cooldown] Delay between sending blocks (Default <i>6</i>)

## Whats left to do
+ Port to other operating systems (ie. Windows)
+ Cleanup code

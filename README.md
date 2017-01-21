Multi-Get
---------

The multi-get client uses chunked transfers to download a file. If run with
`--serial`, it will download a chunk and write it to disk as the next chunk
downloads. If run without `--serial`, it will start several threads and each will
download part of the file, writing the parts to a memory buffer. Once the entire
file has been downloaded, the buffer will be written to a file.

Running `build/client` with no arguments will print a usage message.

Building
--------

`make` will build the project and run unit tests on the HTTP message library.

Limitations
-----------

multi-get only supports downloading over port 80. A URL that specifies a different port number will not be parsed correctly.

# egl-gles-remoting

egl-gles-remoting is a prototype implementation of EGL / OpenGL ES 2.0 remoting
via shared memory (POSIX mmap for now). The project is in a very early
stage. Code and documentation quality are rather bad at the moment. Expect
steady improvements in the course of the next few weeks. A technical paper
elaborating on the concepts and the implementation will be uploaded soon.

If you're eager to try it anyway, clone it and run make. Make sure the following
dependencies are installed.

- EGL 1.4, GLES 2.0
- uthash   (apt-get install uthash-dev)
- vrb      (apt-get install libvrb0-dev)
- dlmalloc (http://g.oswego.edu/dl/html/malloc.html)

## License

Copyright (c) 2011 Jens Schumann

Licensed under the Apache License, Version 2.0
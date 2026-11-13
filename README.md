*This project has been created as part of the 42 curriculum by lfournea.*

# webserv

## Description

webserv is an HTTP/1.1 server written in C++98, built for the 42 curriculum.
It reads an NGINX-style configuration file, listens on one or more ports and
serves static websites, file uploads, file deletion and CGI scripts (Python and
PHP). It works with a standard web browser.

The goal of the project is to understand how HTTP works from the inside: how a
request is received and parsed, how a response is built (status line, headers,
body), and how a server handles many clients at once with a single I/O
multiplexing call (`poll()`).

### Features

- **Methods:** `GET`, `POST` (multipart file upload, or a raw body saved as a
  file) and `DELETE`. Unknown methods get `501`, malformed requests `400`.
- **Multiple servers and ports:** several `server` blocks, each listening on one
  or more `port` or `interface:port` pairs. `listen 8080;` accepts IPv4 and
  IPv6 (`localhost` may resolve to `::1`).
- **Virtual hosts:** several `server` blocks can share an `interface:port`; the
  `Host` header picks the block by `server_name`, the first block being the
  default. Two blocks without `server_name`, or with the same name, on the same
  `interface:port` are a configuration error.
- **Routes (`location` blocks):** accepted methods, root directory, default
  index file, directory listing (`autoindex`), HTTP redirection (`return`) and
  upload directory (`upload_store`) for each route.
- **Request bodies:** `Content-Length` and `Transfer-Encoding: chunked`, with a
  size limit set by `client_max_body_size`.
- **CGI:** runs `.py` scripts with `/usr/bin/python3` and `.php` scripts with
  `/usr/bin/php-cgi`, chosen by file extension. The full request is passed
  through environment variables and the body through stdin. The script runs in
  its own directory. If the script prints CGI headers (`Status`,
  `Content-Type`, ...), the server uses them.
- **Non-blocking I/O:** every socket and CGI pipe is non-blocking and driven
  by a single `poll()` loop. A slow client or a slow CGI script never blocks
  the other clients.
- **Timeouts:** a CGI script running for more than 5 seconds is killed and the
  client gets `504 Gateway Timeout`. A client that stays idle for 30 seconds is
  disconnected (`408 Request Timeout` if its request is incomplete).
- **Clean shutdown:** `Ctrl-C` (SIGINT) or SIGTERM stops the loop, kills
  running CGI scripts and frees all memory.
- **Error pages:** custom pages with `error_page` (a static file, or a `.py` /
  `.php` script run as CGI). Without one, the server
  internally runs `cgi-bin/error_page.py`. If that script fails, the server
  falls back to a page built into the code.
- **Directory listing:** when `autoindex on;` is set and the directory has no
  index file, the server internally runs `cgi-bin/autoindex.py` to build the
  listing.
- **Redirects:** `return` in a location, and a directory requested without its
  trailing `/` is redirected with `301` (or `308` for methods other than `GET`).

## Instructions

### Requirements

- A C++ compiler that supports C++98 (`c++`, i.e. g++ or clang++) and `make`.
- `/usr/bin/python3` for the Python CGI scripts, the error pages and the
  directory listing.
- `/usr/bin/php-cgi` (optional) for the PHP CGI scripts.

### Build

```sh
make        # builds ./webserv
make clean  # removes object files
make fclean # removes object files and the binary
make re     # rebuilds from scratch
```

### Run

Run the server from the root of the repository (the paths in the default
configuration file are relative to it):

```sh
./webserv                              # uses configuration/default.conf
./webserv configuration/default.conf   # same, with an explicit file
```

Then open <http://localhost:6767/> in a browser.

### Configuration file

The syntax is inspired by NGINX `server` blocks. Lines starting with `#` are
comments.

```nginx
server
{
    listen 6767;                      # port, on every interface
    listen 127.0.0.1:7676;            # interface:port
    server_name webserver.fr;         # names matched against the Host header
    root .;                           # server root (relative to the launch directory)
    error_page 404 html_files/error_page.html;        # static error page
    error_page 500 502 503 504 cgi-bin/error_page.py; # several codes, CGI page
    client_max_body_size 10000000;    # max request body size, in bytes

    location /
    {
        root html_files;              # replaces the location prefix
        index index.html;             # file served for a directory
        accepted_methods GET;
    }
    location /configuration/
    {
        accepted_methods GET;
        autoindex on;                 # directory listing on/off
    }
    location /redirect
    {
        return 302 /get.html;         # HTTP redirection (301 302 303 307 308)
    }
    location /store/
    {
        accepted_methods POST;
        upload_store upload;          # where uploaded files are saved
    }
    location .py                      # CGI, chosen by file extension
    {
        accepted_methods GET POST;
        fast_pass_cgi /usr/bin/python3;
    }
}
```

The `root` of a location replaces its prefix: if `/kapouet/` has
`root /tmp/www;`, the URL `/kapouet/pouic/toto` is looked for in
`/tmp/www/pouic/toto`. Without a `root` in the location, the server `root` is
followed by the full URL. Relative paths in a location (`root`, `upload_store`)
are relative to the server `root`.

The first `server` block listening on a port is the default one for that port;
a block whose `server_name` matches the `Host` header exactly is chosen first.

Error page paths are absolute, or relative to the server `root`. A missing
error page file or upload directory, an unknown directive or an invalid port
is a configuration error: the server prints it and refuses to start.

### Quick tests

```sh
curl -i http://localhost:6767/                          # static page
curl -i http://localhost:6767/configuration/            # directory listing
curl -i http://localhost:6767/missing.html              # 404 error page
curl -i -F "file=@README.md" http://localhost:6767/upload/   # upload
curl -i -F "file=@README.md" http://localhost:6767/store/    # upload_store
curl -i http://localhost:6767/redirect                       # redirection
curl -i -X DELETE http://localhost:6767/upload/README.md     # delete
curl -i "http://localhost:6767/cgi-bin/hello.py?name=42"     # Python CGI
curl -i -d "name=42" http://localhost:6767/cgi-bin/hello.py   # CGI with POST
curl -i http://localhost:6767/cgi-bin/relative.py            # CGI working directory
curl -i http://localhost:6767/cgi-bin/crash.py               # 502: script error
curl -i http://localhost:6767/cgi-bin/infinite.py            # 504: killed after 5 s
curl -i -X POST -H "Content-Type: plain/text" --data "short body" \
     http://localhost:9998/upload/                          # 201 (limit: 1000 bytes)
curl -i http://localhost:9998/                               # second website
```

Stress test (an empty page is provided for it):

```sh
siege -b -c 50 -t 30S http://localhost:6767/empty.html
```

## Resources

### References

- [RFC 9110 – HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110)
- [RFC 9112 – HTTP/1.1](https://www.rfc-editor.org/rfc/rfc9112) (message
  format, chunked transfer coding)
- [RFC 1945 – HTTP/1.0](https://www.rfc-editor.org/rfc/rfc1945)
- [RFC 3875 – The Common Gateway Interface (CGI) 1.1](https://www.rfc-editor.org/rfc/rfc3875)
- [NGINX documentation](https://nginx.org/en/docs/), in particular the
  `server`, `location`, `index`, `autoindex` and `error_page` directives
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [MDN – HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP) (status codes,
  headers, MIME types)
- Linux man pages: `socket(2)`, `bind(2)`, `listen(2)`, `accept(2)`,
  `select(2)`, `recv(2)`, `send(2)`, `fork(2)`, `execve(2)`, `pipe(2)`,
  `waitpid(2)`, `stat(2)`

### Use of AI

An AI assistant (Claude) was used for the following tasks:

- **Creation of GUI** AI was used for the front end of this project 
- **Compliance check:** it reviewed the server against the subject and listed
  the missing requirements.
- **Documentation:** it drafted this README.

All AI-generated code was read, tested (with `curl` and a browser) and
understood before being kept.

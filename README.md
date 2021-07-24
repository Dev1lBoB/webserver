# webserver
A simple webserver in C++

The mandatory part:
- You must write an HTTP server in C++ 98.
- The C++ standard must be C++ 98. Your project must compile with it.
- No external library, no Boost, etc...
- Your server must be compatible with the web browser of your choice.
- We will consider that Nginx is HTTP 1.1 compliant and may be used to compare
headers and answer behaviors.
- In the subject and the scale we will mention poll but you can use equivalent like
select, kqueue, epoll.
- It must be non-blocking and use only 1 poll (or equivalent) for all the IO between
the client and the server.
- poll (or equivalent) should check read and write at the same time.
- Your server should never block and the client should be bounce properly if necessary.
- You should never do a read operation or a write operation without going through
poll (or equivalent).
- Checking the value of errno is strictly forbidden after a read or a write operation.
- A request to your server should never hang forever.
- You server should have default error pages if none are provided.
- Your program should not leak and should never crash, (even when out of memory
if all the initialization is done)
- You can’t use fork for something else than CGI (like php or python etc...)
- You can’t execve another webserver...
- Your program should have a config file in argument or use a default path.
- You don’t need to use poll (or equivalent) before reading your config file.
- You should be able to serve a fully static website.
- Client should be able to upload files.
- Your HTTP response status codes must be accurate.
- You need at least GET, POST, PUT and DELETE methods.
- Stress tests your server it must stay available at all cost.
- Your server can listen on multiple ports (See config file).
-  In this config file we should be able to:
  - Choose the port and host of each "server"
  - Setup the server_names or not
  - The first server for a host: port will be the default for this host:port (meaning
it will answer to all request that doesn’t belong to an other server)
  - Setup default error pages
  - Limit client body size
  - Setup routes with one or multiple of the following rules/configuration (routes
wont be using regexp):
    - define a list of accepted HTTP Methods for the route
    - define an HTTP redirection.
    - define a directory or a file from where the file should be search
    - turn on or off directory listing
    - default file to answer if the request is a directory
    - execute CGI based on certain file extension (for example .php)
    - make the route able to accept uploaded files and configure where it should
be saved

The default configuration files are based in "cfg" directory.

You also can run special tester with webserver working with "cfg/TesterConfig.ws" config by executing command "./tester http://localhost:1024"

The bonus part of the project was to:
- Handle multiple CGI (php, python, etc)
- Handle cookies
- Handle session managment

To compile bonus part use "make bonus".

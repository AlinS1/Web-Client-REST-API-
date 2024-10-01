**Similea Alin-Andrei 324CA**

# Web Client. Communication with REST API

### Description

The project's goal is to implement a HTTP client in C that interacts with
a REST API. The main functionalities are to register, login, access the library
of books, show all the books in the library, show a book, add a book, delete a
book and logout.


### Implementation
I started the implementation based on the HTTP Protocol Lab (#9) from the
Communications Protocols Course.

All the input data is read from STDIN. In order to accommodate the cases in
which the input could be a line with spaces, I used the *fgets()* function.
Depending on the cases where the input is needed, it will be verified to be
according to the desired input format.

The commands are dealt with in the client.c file, where each command string is
converted to a command id. Then, the client interacts with the server depending
on the given command.

For register, login and add_book, the payload type is *application/json*, so I
used the Parson library to create JSON objects because it provides a simple
interface for creating JSON objects and eases the work with memory as it has
explicit memory management functions.

For the commands that need to receive additional input, but also need a token
to be performed(ex.: token of access to the library) such as get_book, add_book,
delete_book, the error handling is in the following order:
1. input errors => the input is asked for and verified if it is according to the desired format
2. token errors => if the input is good, we then verify whether the token exists

Depending on the server's response after a request, it is easy to determine
whether there was an error or if the command was a success based on the
respone's body. In the case of login and enter_library, when the command is a
succes we also need to extract a cookie/token which states that the login or
the access to the library was done successfully. We will need to store this
data in the main function because, without it, we shouldn't perform the
commands that interact with the library.
In the case of logout, we will erase not only the login cookie, but also the
access token so we won't be able to perform any more commands on the library.

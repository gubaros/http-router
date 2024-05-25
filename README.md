# HTTP Redirect Server with CDB

This project implements an HTTP redirect server using the `tinycdb` library to handle a large number of URL redirects efficiently. The server reads URL mappings from a `routes.txt` file, generates a `redirects.cdb` database, and uses it to perform redirections.

## Table of Contents

- [Description](#description)
- [Setup](#setup)
  - [Dependencies](#dependencies)
  - [Installation](#installation)
- [Usage](#usage)
  - [Generating the CDB Database](#generating-the-cdb-database)
  - [Running the Server](#running-the-server)
  - [Testing the Server](#testing-the-server)
- [License](#license)

## Description

This project consists of two main components:
1. `create.c`: Reads URL mappings from `routes.txt` and creates a `redirects.cdb` database.
2. `server.c`: Implements an HTTP server that reads from `redirects.cdb` to perform URL redirections.

## Setup

### Dependencies

- `tinycdb` library

### Installation

#### Installing `tinycdb`

You can install `tinycdb` from source as follows:

1. Download and extract the source code:
   ```sh
   curl -O http://www.corpit.ru/mjt/tinycdb/tinycdb-0.78.tar.gz
   tar -xzf tinycdb-0.78.tar.gz
   cd tinycdb-0.78
```
2. Configure and compile:
```sh
CFLAGS="-arch arm64" ./configure
make
sudo make install

gcc -o create_cdb create.c -lcdb
gcc -o http_server server.c -lcdb

## Usage

Generating the CDB Database

    Create the routes.txt file with your URL mappings. Example:

    txt

example,https://www.example.com
google,https://www.google.com

Run the create_cdb program to generate the redirects.cdb file:

sh

    ./create_cdb redirects.cdb routes.txt

Running the Server

Start the HTTP redirect server:

sh

./http_server

Testing the Server

To test the redirections, you can use curl or a web browser.
Using curl

sh

curl -I http://localhost:8080/example

Expected response:

http

HTTP/1.1 302 Found
Location: https://www.example.com
Content-Length: 0

Using a Web Browser

Navigate to http://localhost:8080/example in your web browser. You should be redirected to https://www.example.com.
License

This project is licensed under the terms of the GNU General Public License v3.0. See the LICENSE file for details.

css


Este `README.md` actualizado refleja correctamente la licencia GNU GPL 3.0 y proporciona una guía completa para configurar, compilar, y usar el proyecto, incluyendo instrucciones detalladas para la instalación de dependencias, generación de la base de datos CDB, y ejecución del servidor HTTP.

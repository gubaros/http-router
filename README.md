# HTTP Redirect Server with CDB

This project implements an HTTP redirect server using the `tinycdb` library to handle a large number of URL redirects efficiently. The server reads URL mappings from a `routes.txt` file, generates a `redirects.cdb` database, and uses it to perform redirections.

## Table of Contents

- [Description](#description)
- [Purpose](#purpose)
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

## Purpose

The purpose of this project is to provide a simple and efficient way to handle a large number of HTTP URL redirects. The key motivations behind this project include:

- **Simplicity**: The project aims to demonstrate how to build a basic yet functional HTTP server that performs URL redirections using minimal dependencies and straightforward code.
- **Understanding HTTP**: By building and running this server, users can gain a deeper understanding of how HTTP requests and responses work, and how to implement redirection logic.
- **Performance**: The use of the `tinycdb` library for managing URL mappings allows the server to handle large datasets efficiently. CDB (Constant Database) provides fast lookups with a minimal memory footprint, making it ideal for applications requiring high performance and scalability.

## Aproximacion a la logica 

Lógica del Bucle, Manejo de Conexiones y Uso de kqueue
`
El servidor HTTP está diseñado para manejar un alto número de conexiones concurrentes de manera eficiente. Para lograr esto, se utiliza kqueue, una interfaz de notificación de eventos proporcionada por los sistemas operativos BSD y macOS. kqueue permite monitorear múltiples descriptores de archivo para diversas condiciones de eventos, como lectura, escritura y errores, de una manera escalable y eficiente.
Lógica del Bucle Principal

El servidor crea un socket maestro que escucha en un puerto especificado (8080 por defecto). Este socket se configura como no bloqueante para permitir operaciones de E/S asíncronas. Se crea una instancia de kqueue y se añade el socket maestro a la lista de eventos a monitorear.

El bucle principal del servidor realiza las siguientes operaciones:

    Esperar Eventos: El servidor llama a kevent para esperar eventos en los descriptores de archivo registrados. Esta llamada se bloquea hasta que haya eventos disponibles.

    Procesar Eventos: Cuando kevent devuelve eventos, el servidor los procesa uno por uno:
        Nueva Conexión: Si el evento se asocia con el socket maestro, se acepta una nueva conexión. El nuevo socket de cliente también se configura como no bloqueante y se añade a kqueue para monitorear eventos de lectura.
        Datos Disponibles para Leer: Si el evento se asocia con un socket de cliente, se leen los datos disponibles. Si el cliente se ha desconectado, el socket se cierra. Si se reciben datos, se procesan para determinar la solicitud del cliente y se genera la respuesta apropiada.

Manejo de Conexiones

El manejo eficiente de conexiones es crucial para el rendimiento del servidor. El uso de kqueue permite al servidor manejar miles de conexiones concurrentes sin bloquear, lo que sería ineficiente y escalable con otras técnicas como el uso de hilos o procesos para cada conexión.

    No Bloqueante: Todos los sockets (tanto el maestro como los de clientes) se configuran como no bloqueantes. Esto asegura que las operaciones de E/S no bloqueen el bucle principal del servidor.
    Reutilización de Conexiones: Al finalizar la lectura de datos de un cliente, el socket se cierra y se elimina de kqueue, liberando recursos del sistema.

Uso de kqueue

kqueue es una opción ideal para aplicaciones que requieren manejar un gran número de conexiones simultáneas debido a su eficiencia y escalabilidad. A diferencia de select y poll, que pueden volverse ineficientes con un gran número de descriptores, kqueue maneja eventos de manera eficiente mediante un modelo basado en notificaciones.

Ventajas de kqueue:

    Escalabilidad: Maneja eficientemente miles de descriptores de archivo.
    Eficiencia: Reduce el consumo de CPU y mejora el rendimiento general del servidor.
    Flexibilidad: Soporta una amplia gama de eventos y condiciones de notificación.

En resumen, la combinación de kqueue y E/S no bloqueante permite al servidor HTTP manejar un alto volumen de tráfico con baja latencia y alta eficiencia, lo que es fundamental para aplicaciones web modernas y servicios que requieren un rendimiento robusto y escalable.
`
## Setup

### Dependencies

- `tinycdb` library

### Installation

## Installing `tinycdb`

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
gcc -o create create.c -lcdb
gcc -o server server.c -lcdb
```
### Usage

## Generating the CDB Database

Create the routes.txt file with your URL mappings. Example txt:

```sh
example,https://www.example.com
google,https://www.google.com
```

Run the create program to generate the redirects.cdb file:

```sh
./create redirects.cdb routes.txt
```
### Running the Server

Start the HTTP redirect server:

```sh
./server
```

### Testing the Server

To test the redirections, you can use curl or a web browser.
Using curl

```sh
curl -I http://localhost:8080/example
```

Expected response:
```sh
http

HTTP/1.1 302 Found
Location: https://www.example.com
Content-Length: 0
```
Using a Web Browser

Navigate to http://localhost:8080/example in your web browser. You should be redirected to https://www.example.com.

## License

This project is licensed under the terms of the GNU General Public License v3.0. See the LICENSE file for details.


# Project 3: Producers and Consumers
Launches a number of producer and consumer threads. Every producer will produce a given number events. Consumers will wait to consume events produced by consumers. All together. producers may only produce a given max number of outstanding events before they are consumed by a consumer. Once all producers are finished producing, and consumers finished consuming, the program exits.

## Building
* `make` to build. Creates an executable called `pc`.
* `make clean` to remove all build products including the executable.

VS Code:
* The default build task runs `make`.

## Files
* `pc.c`: Main file that launches and manages producers and consumemrs.
* `eventbuf.c`: A FIFO event queue.

## Data
* `producer_count`: The number of producer threads to create.
* `consumer_count`: The number of consumer threads to create.
* `event_count`: The number of events each producer will produce.
* `max_events`: The maximum number of events there can be outstanding (i.e. not consumes yet) before producers will stop producing.

## Functions
* `main()`
    * `sem_open_temp`: Creates a semaphore.
    * `eventbuf_create`: Creates an event buffer for producers and consumers to use.
    * `create_producer_args`: Creates an array of producer arguments equal to the number of producer threads. Each producer gets one of the arguments.
    * `create_consumer_args`: Creates an array of consumer arguments equal to the number of consumer threads. Each consumer gets one of the arguments.
    * `produce`: Ran by each producer thread. Creates a given number of events.
        * `eventbuf_add`: Add an event to the buffer.
    * `consume`: Ran by each consumer thread. Consumes events as they become available.
        * `eventbuf_empty`: Returns true if the event buffer is empty.
        * `eventbuf_get`: Get the next event from the buffer.
## Usage
./pc `<producer_count>` `<consumer_count>` `<event_count>` `<max_events>`

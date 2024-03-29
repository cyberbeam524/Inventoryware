# Inventoryware

## Section 1: User's Console Program
<img src="./images/console_example.gif" style="width:100%">

## Tech Stack
- C++ console program (user-interface & backend)
- MongoDB (database)

## Inventory's Layered Architecture
<img src="./images/architecture_layers.png" style="width:50%">

## Running locally

#### Install dependencies
- mongocxx (guide for installing: https://mongocxx.org/mongocxx-v3/installation/)
- bsoncxx

#### Start Console Program
1. Create a folder called "output" with 
``` 
mkdir output 
``` 
2. To compile the c++ program and start it, open terminal and run 
``` 
make mongo 
``` 
or 
```
g++ --std=c++11 -o ./output/mongo mongo.cpp -I/usr/local/include/mongocxx/v_noabi \
	-I/usr/local/include/bsoncxx/v_noabi \
	-L/usr/local/lib -lmongocxx -lbsoncxx
	./output/mongo
```
<img src="./images/startprogram.png" style="width:50%">

3. Add Item
Type option 1 to add items. Users will be prompted for the name, quantity, and expiration date of the items to be added.

<img src="./images/additem.png" style="width:50%">

4. Remove Item
Type option 3 to add items. Users will be prompted for the name of the item to be removed.

<img src="./images/removeitem.png" style="width:50%">

5. View Items
Type option 2 to view items current items in inventory as shown above.


## Section 2: Making it a Multithreaded App

### Mulithreaded App Architecture 
<img src="./images/workerpool_diagram.png" style="width:50%">

### Design Considerations:
- To relieve hotspots in the program when multiple edit requests are made such as adding, deleting, editting 100 or more documents at once, concurrency can be used instead of running it sequentially. This reduces time taken overall as well.
- Concurrent operations require creating new threads to execute parallel operations. Creating and closing threads can be a resource intensive process. Thus, having **_reusable threads_** that can be fetched from pools can be used. 
- A worker threadpool and a database connections threadpool are used to synchronise with each other and execute multiple transactions in database. 
- A thread safe queue was used in both thread pools to ensure race conditions of tasks are avoided. A **_thread safe queue_** was created using **_mutex_** to prevent two or more different threads from accessing the same task queue. 
    - A possible race condition might be two threads executing the same popped task, resulting in the task getting executed twice such as a bank transaction of deducting $100 from user being done twice. 
- Having more worker threads than hardware cores results in each core having 2 or more threads. With each core handling 2 or more threads, it can result in **_context switching_**, another memory intensive process. Thus, to prevent this, an amount equivalent to the number of hardware cores were used.
- Each worker pool thread only needs one database connection. Thus, an equivalent number of postgresql database connection threads were created in database pool. 
- Postgresql was used instead of mongodb due to library incompatibility issues that prevents fetching multiple mongodb connections as needed.
- Smart pointers used:
    - unique pointer used to store all postgresql connection objects as only one thread will be accessing it at a time. Only one pointer **_cannot be duplicated_** and the object will be destroyed once the unique pointer goes out of scope (RAI principles apply).  If **_memory leaks_** were to occur from undestoryed postgresql connections stored in raw pointers, it will case the machine to break down in a few months possibly. Using unique pointers prevents memory leaks by safely destroying postgresql connection objects at the end of program.
    - shared pointer to the pool can be **_duplicated_** and sent to all the threads so that they are able to fetch connections from the common pool. The pool object is destroyed only when the last pointer goes out of scope. As it keeps track of the number of references used, it is **_more memory inefficient_** than unique pointers and its use should be minimised. 
    - weak pointer of pool's shared pointer created to **_prevent memory leaks_** due to threads not completing in time at end of main scope and thus still holding onto some shared pointer of pool.

## Tech Stack
- C++ console program (user-interface & backend)
- Postgresql (database)

## Running locally

1. Create a folder called "output" with 
```
mkdir output
``` 

2. To start the postgresql database, run 
```
docker compose up
``` 

3. To compile the c++ program and start it, open terminal and run 
```
make postgres_threadpool
``` 


## To Dos:
- [x] Basic CRUD Operations
- [x] Inventory as a Business Layer
- [ ] Items abstraction for better comparison between items
- [ ] Sorting inventory items
    - [ ] by expiry date
        - [ ] option to remove items with expiry date more than today
    - [ ] other columns 
- [ ] Add CPU performance profiler and race condition detector(race_detector) for better analysis

References:
- https://github.com/mongodb/mongo-cxx-driver
- https://www.oreilly.com/library/view/software-architecture-patterns/9781491971437/ch01.html#:~:text=Although%20the%20layered%20architecture%20pattern,(Figure%201%2D1).
- https://medium.com/@extio/microservices-with-c-scalable-software-architecture-for-modern-development-a1bbe766e41a
- https://levelup.gitconnected.com/a-universal-connection-pool-written-in-c-6f4f14a98a9e 
- https://www.codeproject.com/Articles/16960/Connection-Pool-in-a-Static-Library
- https://github.com/active911/connection-pool/blob/master/ConnectionPool.h
- https://github.com/sewenew/redis-plus-plus/blob/master/src/sw/redis%2B%2B/async_connection_pool.cpp
- http://mongocxx.org/api/mongocxx-3.2.0/pool_8hpp_source.html
- https://levelup.gitconnected.com/a-universal-connection-pool-written-in-c-6f4f14a98a9e 
- https://stackoverflow.com/questions/6508037/postgres-connection-pooling-library
- https://arctype.com/blog/connnection-pooling-postgres/ 
- https://github.com/joegsn/uvpgpool/blob/master/uvpgpool/UVPGPool.cpp
- https://www.linkedin.com/pulse/memory-management-using-smart-pointers-c-part-2-pratik-parvati

C++ Methods Specific:
- https://www.scaler.com/topics/unordered_map-cpp/

Profiling CPU utilisation by program:
- https://stackoverflow.com/questions/11445619/profiling-c-on-mac-os-x 
- https://go.dev/doc/articles/race_detector

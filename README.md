# Overview

dlog is a tiny header-only library for producing ungarbled (non-interleaved) output from multiple threads.

## Reasons for creating dlog

Debugging multi-threaded programs can be a real headache. Contrary to single-threaded programs, where execution follows a strict order determined by the main() function, in multi-threaded programs the main() function is the point of entry to the program but not necessarily the point of exit. In other words, threads can survive beyohd the end of the main() function. This can cause all sorts of trouble, and sometimes having the ability to *see* the output can help immensely with debugging.

However, there is the added problem that unsynchronised threads can access output streams at the same time, producing garbled (interleaved) output. We have to ensure that each time we output something to a stream we do so atomically, for example, by locking a mutex. But locking a mutex manually for each output operation can be dreadfully cumbersome.

## What dlog does

You can use dlog to direct output to streams like you do with any other stream (such as `std::cout` or a `std::ofstream`). It is nothing but a thin veil over your stream that wraps an output operation and ensures that it is done atomically, thus preventing interleaved output. It also has the added benefit of revealing the *actual* order of events occurring in multiple threads rather than what can be (often wrongly) inferred from looking at the source code.

You can use dlog in two ways. There are several convenient constructors that allow dlog to be used as a "one-liner":

```c++
dlog("This is output as a single atomic operation.\n", "It will not be interleaved with output from other threads.\n");
```
Essentially, this creates a temporary object that lives just long enough to print whatever is passed to the constructor.

The other option is to create a named object and print to it using `operator <<`.

```c++

dlog d(">> Dlog instantiated.");

// Code...

d << "This will be printed in a single atomic operation.\n";

// More code ...

d << "It will not be interleaved with output from other threads.\n";
```
The entire sequence will be printed nicely without interference from other threads when the `dlog` object is destroyed.

## Linking

`dlog` is a header-only library, so no linking required - just download the header and `#include` it in your project. Note that it depends on the threadpool library (included). The two will be merged into a larger project in the near future. 

## Usage

An example is provided under `src`. To run the example, do the following (assuming that the sources are located under the directory `dlog` in your home directory):

```bash
$ cd ~/dlog
$ mkdir build && cd build && cmake .. && make
$ cd ../bin
$ ./dlog
```

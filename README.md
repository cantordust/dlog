# Overview

dlog is a tiny header-only library for producing ungarbled (non-interleaved) output from multiple threads.

## Reasons for creating dlog

Debugging multi-threaded programs can be a real headache. Contrary to single-threaded programs, where execution follows a strict order determined by the main() function, in multi-threaded programs the main() function is the point of entry to the program but not necessarily the point of exit. In other words, threads can survive beyohd the end of the main() function. This can cause all sorts of trouble, and sometimes having the ability to *see* the output can help immensely with debugging.

However, there is the added problem that unsynchronised threads can access output streams at the same time, producing garbled (interleaved) output. We have to ensure that each time we output something to a stream we do so atomically, for example, by locking a mutex. But locking a mutex manually for each output operation can be dreadfully cumbersome.

## What dlog does

You can use dlog to direct output to streams like you do with any other stream (such as `std::cout` or a `std::ofstream`). It is nothing but a thin veil over your stream that wraps an output operation and ensures that it is done atomically, thus preventing interleaved output. It also has the added benefit of revealing the *actual* order of events occurring in multiple threads rather than what can be (often wrongly) inferred from looking at the source code.

You can use dlog like you use any other stream, such as `std::cout` (the default output stream), just add parentheses `()` after `dlog`:

```c++
dlog() << "This is output as a single atomic operation.\n"
	   << "It will not be interleaved with output from other threads.\n";
```

dlog is also capable of printing any `std::future` produced by `std::async`, `std::promise` or `std::packaged_task`. To print a future asynchronously, just make sure that you don't call your future's `get()` method when printing it, otherwise it will block.

```c++
std::future f(std::async(std::launch::async, my_function));
dlog() << "Future: " << f; // *Not* f.get()
```

dlog will execute the task and will print the output when it is finished. After you have debugged your program, you can comment out dlog entries and call the `get()` method as usual.

## Usage

dlog is a header-only library, which means that you don't have to link against it - just download the header and `#include` it in your project. An example covering its usage is provided under `src`. To run the example, do the following (assuming that the sources are located under the directory `dlog` in your home directory):

```bash
$ cd ~/dlog
$ mkdir build && cd build && cmake .. && make
$ cd ../bin
$ ./dlog
```

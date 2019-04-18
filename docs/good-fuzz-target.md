# What makes a good fuzz target

This document explains the concept of a *fuzz target* and
gives advice on how to make your fuzz targets really good.

# Basics

A *fuzz target* is a function that accepts an array of bytes and does something interesting with these bytes using the API under test.
We define a specific signature for fuzz targets:

```cpp
// fuzz_target.cc
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  DoSomethingInterestingWithMyAPI(Data, Size);
  return 0;  // Non-zero return values are reserved for future use.
}
```

Note that the interface for this function is C, and so it can be implemented in any programming language that supports C interface.
Despite the name referecing the [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html) fuzz targets are
independent from a particular *fuzzing engine* and can be used with fuzzing, symbolic execution, or any other form of testing. 

The basic things to remember about a fuzz target:

* The fuzzing engine will execute it many times with different inputs in the same process.
* It must tolerate any kind of input (empty, huge, malformed, etc).
* It must not `exit()` or `abort()` on any input (if it does, it's a bug).
* It may use threads but ideally all threads should be joined at the end of the function.
* It must be as deterministic as possible. Non-determinism (e.g. random decisions not based on the input bytes) will make fuzzing inefficient.
* It must be fast. Try avoiding cubic or greater complexity, logging, or excessive memory consumption.
* Ideally, it should not modify any global state (although that’s not strict).
* Usually, the narrower the target the better. E.g. if your target can parse several data formats, split it into several targets, one per format.

More details follow.

# One-time initialization
Sometimes an API under test needs to be initialized.

The simplest and the recommended way is to have a statically initialized global object inside LLVMFuzzerTestOneInput:

```cpp
// fuzz_target.cc
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  static bool Initialized = DoInitialization();
  ...
```

Or (less preferable) in the global scope if that works for you:

```cpp
// fuzz_target.cc
static bool Initialized = DoInitialization();
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  ...
```

# Determinism

A fuzz target needs to be deterministic, i.e. given the same input it should
have the same behavior.
This means, for example, that a fuzz target should not use `rand()` or any other
source of randomness.


# Speed

Fuzzing is a search algorithm that requires **many** iterations, and so
a good fuzz target should be very fast.

A typical good fuzz target will have an order of 1000 executions per second per
one CPU core on average (exec/s) or more. For lightweight targets, 10000 exec/s or more.

If your fuzz target has less than 10 exec/s you are probably doing something
wrong. We recommend to profile fuzz targets and eliminate any obvious hot
spots.

# Memory consumption

For CPU-efficient fuzzing a good fuzz target should consume less RAM than it is
available on a given (virtual) machine per one CPU core.
There is no one-size-fits-all RAM threshold, but as of 2019 a typical good fuzz
target would consume less than 1.5Gb.

# Global state

Ideally, a fuzz target should not modify a global state because otherwise
the fuzz target may be non-deterministic.

It may not be possible in every case
(strictly speaking, even calling `malloc()` modifies a global state).

# Timeouts, OOMs, shallo bugs

A good fuzz target should not have any
timeouts (inputs that takes too long to process),
OOMs(input that cause the fuzz target to consume too much RAM),
or shallow (easily discoverable) bugs.
Otherwise fuzzing will stall quickly.


# Seed corpus

# Coverage discoverability

# Input size

# Dictionaries

# IO

# fmemopen

# Threads

# Splitting inputs

# Structure-aware fuzzing

# Related materials
* [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html)
* [OSS-Fuzz ideal integration](https://github.com/google/oss-fuzz/blob/master/docs/ideal_integration.md)
* [OSS-Fuzz glossary](https://github.com/google/oss-fuzz/blob/master/docs/glossary.md)

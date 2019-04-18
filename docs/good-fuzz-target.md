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

# Build and test integration

A good fuzz target participates in the project's continuous testing:
* it resides in the same repository as the code and other tests,
* it is being compiled with available/applicable [sanitizers](https://github.com/google/sanitizers)
as part of the usual testing process and linked e.g. with this [standalone runner](https://github.com/llvm-mirror/compiler-rt/blob/master/lib/fuzzer/standalone/StandaloneFuzzTargetMain.c) or similar.
* it is being executed as part of the usual testing process using the [seed corpus](#seed-corpus) as inputs.


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
timeouts (inputs that take too long to process),
OOMs(input that cause the fuzz target to consume too much RAM),
or shallow (easily discoverable) bugs.
Otherwise fuzzing will stall quickly.

# Seed corpus

In most cases a good fuzz target should be accompanied with a *seed corpus*,
which is a set of representative inputs for the fuzz target.
These inputs combined should cover large portions of the API under test,
ideally achieving 100% coverage (different coverage metrics can be applied, e.g.
block coverage or edge coverage, depending on a specific case).

Avoid large seed inputs when smaller inputs are sufficient for providing the
same coverage.

A seed corpus is stored as a directory where every individual file represents one input.

# Coverage discoverability

It is often insufficient to have a seed corpus with good code coverage
to claim good *fuzzability*, i.e. the ability of a fuzzing engine to discover
many code paths in the API under test.

For example, imagine we are fuzzing an API that consumes an encrypted input,
and we have a comprehensive seed corpus with such encrypted inputs.
This seed corpus will provide good code coverage, but any mutation
of the inputs will be rejected early as broken.

So, it is important to ensure that the fuzz target can discover
a large subset of reachable pieces of code without using the seed corpus.

# Input size

# Dictionaries

# IO

A good target does not use I/O:
* Avoid debug output to `stderr` or `stdout` as it slows down fuzzing.
* Avoid reading from disk other than during one-time initialization,
* Avoid writing to disk.
* Avoid fmemopen. It might be tempting to use `fmemopen` to fuzz APIs that
  consume `FILE*`, but it may inhibit important search algorithms in the fuzzing engine.
  Prefer using direct in-memory APIs, even if it requires refactoring your API.

# Threads

# Code size

# Splitting inputs

# Structure-aware fuzzing

# Examples

# Related materials
* [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html)
* [OSS-Fuzz ideal integration](https://github.com/google/oss-fuzz/blob/master/docs/ideal_integration.md)
* [OSS-Fuzz glossary](https://github.com/google/oss-fuzz/blob/master/docs/glossary.md)

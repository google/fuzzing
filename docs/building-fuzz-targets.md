# Building fuzz targets

This document focuses on the specifics of how to build fuzzers in common
configurations useful for detecting security bugs. If you haven’t written your
first fuzz target yet or aren’t sure where to start, see the
[introduction to fuzzing](https://github.com/google/fuzzing/blob/master/docs/intro-to-fuzzing.md)
document.

## Requirements

For the purposes of this document, we’ll be providing instructions for building
with the [Clang](https://clang.llvm.org/). **Many features used here require
Clang 6.0 or greater. If you plan to use Windows, Clang 9.0 or greater is
required.** We recommend using the most recent version you can.

To install Clang on Linux, follow [these instructions](https://apt.llvm.org/).
You can download Clang for Windows from
[llvm’s snapshot builds page](https://llvm.org/builds/).

## [Sanitizers]

Regardless of which fuzzing tool you end up using, we strongly recommend
learning how to build your target binaries with the [sanitizers] enabled.
[AddressSanitizer] is the most commonly used and simplest of these tools, so
it’s what we’ll use in our fuzzing tool build examples below.

### [AddressSanitizer](ASan)

Building with AddressSanitizer allows you to detect various security
vulnerabilities deterministically, and will cause issues such as use-after-free
bugs, buffer overflows, and other subtle issues to crash. It will also provide
detailed information when this happens to simplify the debugging process, all
with minimal overhead.

Though we recommend taking a look at the
[full documentation](https://clang.llvm.org/docs/AddressSanitizer.html) for a
complete list of features and detailed usage instructions, the command to build
a binary with this instrumentation can be as simple as:

```bash
clang++ -fsanitize=address test.cc
```

Adding the flag `-fsanitize=address` enables AddressSanitizer and its
instrumentation.

### [UndefinedBehaviorSanitizer](UBSan)

Building with [UndefinedBehaviorSanitizer] allows you to detect various forms of
[undefined behavior](https://en.wikipedia.org/wiki/Undefined_behavior) in your
applications. It is capable of detecting many different types of issues, and
[this page](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html#available-checks)
includes the full list of checks, as well as information on how to enable each
of them.

Similar to ASan, it’s fairly easy to use UBSan out of the box with Clang as
well. If you don’t wish to enable a specific subset of checks, a build command
may be as simple as the following:

```bash
clang++ -fsanitize=undefined test.cc
```

The `-fsanitize=undefined` flag enables UBSan.

[sanitizers]: https://github.com/google/sanitizers
[addresssanitizer]: https://clang.llvm.org/docs/AddressSanitizer.html
[undefinedbehaviorsanitizer]: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html

## Examples

### [libFuzzer]

For users setting up fuzzing for the first time, we recommend libFuzzer for its
simplicity. When building with Clang enabling libFuzzer is as simple as adding
the `-fsanitize=fuzzer` flag to your build command.

Assuming you have already written a [fuzz target] \(fuzzer.cc in this example\),
you can build and run with commands resembling the following:

```bash
# Build the fuzz target.
clang++ -fsanitize=address,fuzzer fuzzer.cc -o fuzzer
# Run.
./fuzzer
```

[libfuzzer]: https://llvm.org/docs/LibFuzzer.html
[fuzz target]: https://llvm.org/docs/LibFuzzer.html#fuzz-target

### [AFL]

**Since AFL can be used in a variety of ways, we recommend
[AFL’s documentation](https://github.com/google/AFL) for generic build
instructions.**

For new fuzzer developers that have developed libFuzzer-style targets (those
which define LLVMFuzzerTestOneInput) who also wish to try AFL, we provide
[a script](https://google.github.io/clusterfuzz/setting-up-fuzzing/build_afl.bash)
to assist with building.

After
[downloading](https://google.github.io/clusterfuzz/setting-up-fuzzing/build_afl.bash)
it, you can build and run with:

```bash
# Build afl-fuzz and FuzzingEngine.a
./build_afl.bash
# Compile target using ASan, coverage instrumentation, and link against FuzzingEngine.a
clang++ -fsanitize=address -fsanitize-coverage=trace-pc-guard fuzzer.cc FuzzingEngine.a -o fuzzer
# Test out the build by fuzzing it. INPUT_CORPUS is a directory containing test files. Ctrl-C when done.
AFL_SKIP_CPUFREQ=1 ./afl-fuzz -i $INPUT_CORPUS -o output -m none ./fuzzer
```

[afl]: https://github.com/google/AFL

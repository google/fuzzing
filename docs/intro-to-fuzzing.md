# Introduction to fuzzing

## Overview

This document assumes that you have a basic understanding of what
[fuzzing](https://en.wikipedia.org/wiki/Fuzzing) is, and that you understand
[how it can be beneficial](https://github.com/google/fuzzing/blob/master/docs/why-fuzz.md).
Rather than go into detail on the specifics of how to use any particular fuzzing
tools, this is intended as a high-level survey of concepts which will be useful
to a fuzzing novice. Links are provided to more detailed further reading.

## Sanitizers

The end goal of fuzzing is to find bugs. Generally, a fuzzer will determine it
has found a bug by detecting an application crash. Many potential interesting
security bugs don’t necessarily cause a normal application to crash immediately.

This is where tools like the [sanitizers](https://github.com/google/sanitizers)
can be useful:

*   [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
    detects various memory safety issues such as use-after-free bugs and buffer
    overflows. If nothing else, we strongly recommend building any of your
    fuzzing binaries with this instrumentation.
*   [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
    detects various forms of
    [undefined behavior](https://en.wikipedia.org/wiki/Undefined_behavior). For
    example, it can detect signed integer overflow, use of misaligned pointers,
    and much more.
*   [MemorySanitizer](https://clang.llvm.org/docs/MemorySanitizer.html) detects
    reads of uninitialized memory. While it is a very useful tool, it can be
    much more difficult to use than the others because it requires that all
    dependencies are also instrumented. We recommend setting this up after you
    already have some familiarity with the other tools.

By taking advantage of these, many subtle bugs will be detected as deterministic
crashes. This not only allows you to find the bugs more easily, but also makes
it simpler to reproduce and fix potential issues. These can be used with any of
the fuzzing tools mentioned in this document, and we strongly recommend taking
advantage of them.

## What to fuzz?

The [Why Fuzz?](https://github.com/google/fuzzing/blob/master/docs/why-fuzz.md)
document goes into detail about the types of bugs that fuzzing can catch, and
the types of applications that may benefit from fuzzing. After deciding to fuzz
an application, it’s important to identify how that application can be fuzzed.

Any application code exposed to untrusted user input is usually a good candidate
for fuzzing. Identifying parts of your code that are exposed to untrusted input
can sometimes be difficult for large applications, but we recommend finding ways
to define entry points that can be used for your fuzzers, mimicking the types of
input your application may be exposed to.

For example, if a browser defines a function that takes an input string of CSS
and parses it, that may be a good candidate for fuzzing. It’s a clear entry
point that could be used for a fuzzer, and it’s likely that arbitrary
attacker-supplied input would be handled by it as well.

If code is exposed to the network, it is usually safe to assume it expects to
handle untrusted inputs. Though the same advice from above applies, some fuzzing
tools allow this code to be
[tested directly](https://github.com/google/honggfuzz/blob/master/socketfuzzer/README.md).

Code can also be tested for correctness by comparing the results of two or more
implementations to one another. Any difference in the output of the applications
implies that at least one of them has a bug. This technique can be used in any
case where a specification has multiple implementations, or for programs with
multiple levels of optimization that can be compared to one another. For
example, it has been used to test
[cryptography libraries](https://github.com/guidovranken/cryptofuzz) and
[javascript engies](https://www.squarefree.com/2007/08/02/fuzzing-for-correctness/).

## Fuzzing Tools

Each fuzzing tool has its own strengths and weaknesses, and it's not always
clear which will be best for a given task. The
[FuzzBench](https://github.com/google/fuzzbench) project attempts to empirically
evalutate various fuzzing tools, and it can be helpful in making a decision.

### [libFuzzer]

[libFuzzer] is a tool which allows you to write fuzz tests in a similar style to
unit tests. For developers setting up fuzzing for the first time, we recommend
it for its simplicity.

A libFuzzer-based fuzzer could be as simple as the following:

```cpp
// fuzz_target.cc
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  DoSomethingInterestingWithMyAPI(Data, Size);
  return 0;  // Non-zero return values are reserved for future use.
}
```

[libfuzzer]: https://llvm.org/docs/LibFuzzer.html

### [Honggfuzz]

[Honggfuzz] is another fuzzing tool with a rich set of
[features](https://github.com/google/honggfuzz#features) available to it. We
recommend it for security researchers looking for more customization options in
their fuzzers, or developers on complex projects where working within libFuzzer's
unit-test-like style is difficult.

It allows users to
[specify how input files should be fed](https://github.com/google/honggfuzz/blob/master/docs/USAGE.md)
to a target program (e.g. on the command line, on stdin) or allows more
complicated customizations such as
[feeding input to a network server](https://github.com/google/honggfuzz/blob/master/socketfuzzer/README.md)
directly.

[Honggfuzz]: https://github.com/google/honggfuzz

### [AFL]

[AFL] is the tool that did the most to drive the early success of
coverage-guided fuzzing. We generally do not recommend AFL for new projects
since it is not as well-maintained as Honggfuzz or libFuzzer. That said, because
of its widespread it is common to encounter existing projects which rely on it
for fuzzing.

Many
[specialized variants](https://github.com/google/fuzzing/blob/master/docs/afl-based-fuzzers-overview.md)
of AFL have been developed for a variety of use cases, and which provide a
number of potential improvements. Forks such as
[AFL++](https://github.com/AFLplusplus/AFLplusplus) are better maintained while
still providing good compatibility with fuzzers originally written for AFL.

[AFL]: https://github.com/google/AFL

## Further reading

### [Building fuzz targets](https://github.com/google/fuzzing/blob/master/docs/building-fuzz-targets.md)

This document provides detailed instructions on how to build fuzz targets using
libFuzzer or AFL with the sanitizer tools enabled.

### [What makes a good fuzz target?](https://github.com/google/fuzzing/blob/master/docs/good-fuzz-target.md)

After you write your first fuzz target, you may be surprised to find that it
isn’t detecting any bugs. There are many ways to improve the efficiency of fuzz
targets, from seed corpora to dictionaries and much more. This document outlines
some of the factors you should consider when writing a target to ensure it
performs well.

### [Structure aware fuzzing](https://github.com/google/fuzzing/blob/master/docs/structure-aware-fuzzing.md)

Some input formats are too complicated for fuzzing tools to mutate effectively
on their own. In these cases, a fuzzer developer may need to do more work to
specify the format to the fuzzer or define how tests should be mutated. While
this may seem complicated, this work is often rewarded with deeper,
harder-to-find bugs.

### [Splitting inputs](https://github.com/google/fuzzing/blob/master/docs/split-inputs.md)

Most fuzzing tools expect fuzzers to take a single file or stream of data as
input, but many real world applications are more complicated than that. They may
require several integers or configuration options in addition to data, or simply
require something other than a stream of bytes. This document outlines some of
the techniques and tools that can be used to handle such cases.

### [Fuzzing glossary](https://github.com/google/fuzzing/blob/master/docs/glossary.md)

Fuzzer developers use a great deal of jargon. If any terms seem unfamiliar to
you, check the glossary to see if it can help clear up any confusion.

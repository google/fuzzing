# Why fuzz?

Original authors: Kostya Serebryany, David Drysdale, Chris Lopez, and Max Moroz

This document describes the types of programs that may benefit from [fuzzing](https://en.wikipedia.org/wiki/Fuzzing) and why.

## Introduction

**Fuzz testing is a process of testing APIs with generated data.** The most common forms are:
- **Mutation based fuzzing** which mutates existing data samples (aka the test corpus) to create test data;
- **Generation based fuzzing** which produces new test data based on models of the input.

**Guided fuzzing** is an important extension to mutation based fuzzing. Guided fuzzers employ a feedback loop when testing newly mutated inputs. If an input results in a new signal  (such as increased code coverage), it is permanently added to the test corpus. The corpus grows over time, therefore increasing the test coverage of the target program.

**Fuzz testing is valuable for:**
- Software that receives inputs from untrusted sources **(security)**;
- Sanity checking the equivalence of two complex algorithms **(correctness)**;
- Verifying the stability of a high-volume API that takes complex inputs **(stability)**,
e.g. a decompressor, even if all the inputs are trusted.

Why? **Fuzz testing finds programming errors unrelated to the project requirements.** Errors are always errors, e.g. memory leaks or buffer overflows. Developers will rarely write tests to try and catch these types of bugs, and these are exactly the types of bugs that lead to security holes and reliability problems. Fuzzing is a great and very effective way to find these bugs before they hit you in production or the bad guys exploit them.

**Fuzzing is not a substitute for explicitly testing functional requirements.** Write the appropriate unit/integration/system tests. Then consider additional fuzzing if applicable.

**Fuzz testing must be done continuously.** This problem is solved at Google. Once a fuzz target is submitted, the fuzzing infrastructure will run it 24/7. With corpus growing over time, it provides both intensive testing for the stable code and regression testing for the new changes.

## Fuzzing is typically used to find the following kinds of bugs
- Bugs specific to C/C++ that require the *[sanitizers](https://github.com/google/sanitizers)* to catch:
  - Use-after-free, buffer overflows
  - Uses of uninitialized memory
  - Memory leaks
- Arithmetic bugs:
  - Div-by-zero, int/float overflows, invalid bitwise shifts
- Plain crashes: 
  - NULL dereferences, Uncaught exceptions
- Concurrency bugs:
  - Data races, Deadlocks
- Resource usage bugs:
  - Memory exhaustion, hangs or infinite loops, infinite recursion (stack overflows)
- Logical bugs:
  - Discrepancies between two implementations of the same protocol ([example](https://blog.fuzzing-project.org/31-Fuzzing-Math-miscalculations-in-OpenSSLs-BN_mod_exp-CVE-2015-3193.html))
  - Round-trip consistency bugs (e.g. compress the input, decompress back, - compare with the original)
  - Assertion failures
 
Most of these are exactly the kinds of bugs that attackers use to produce exploits, from denial-of-service through to full remote code execution.

## Potential Fuzzing Targets
Types of projects where fuzzing has been useful:
- Anything that consumes untrusted or complicated inputs:
  - Parsers of any kind (xml, pdf, truetype, ...)
  - Media codecs (audio, video, raster and vector images, etc)
  - Network protocols, RPC libraries  (gRPC)
  - Network scanners (pmon)
  - Crypto (boringssl, openssl)
  - Compression (zip, gzip, bzip2, brotli, …)
  - Compilers and interpreters (PHP, Perl, Python, Go, Clang, …)
  - Services/libraries that consume protobuffers (i.e. anything in google3)
  - Regular expression matchers (PCRE, RE2, libc)
  - Text/UTF processing (icu)
  - Databases (SQlite)
  - Browsers (all)
  - Text editors/processors (vim, OpenOffice)
- OS Kernels (Linux), drivers, supervisors and VMs
- UI (Chrome UI)

## Fuzzing Successes
Historically, fuzzing has been an extremely effective technique for finding long-standing bugs in code bases that fall into the target categories above. Some trophy list examples (with a total number of tens of thousands bugs found inside and outside of Google):
- [AFL bugs](http://lcamtuf.coredump.cx/afl/#bugs)
- [libFuzzer bugs](http://llvm.org/docs/LibFuzzer.html#trophies)
- [syzkaller bugs](https://github.com/google/syzkaller/blob/master/docs/found_bugs.md)
- [go-fuzz bugs](https://github.com/dvyukov/go-fuzz#trophies)
- [Honggfuzz bugs](https://github.com/google/honggfuzz#trophies)
- [ClusterFuzz bugs in Chrome](https://bugs.chromium.org/p/chromium/issues/list?can=1&q=label%3AClusterFuzz+-status%3AWontFix%2CDuplicate&sort=-id&colspec=ID+Pri+M+Stars+ReleaseBlock+Cr+Status+Owner+Summary+OS+Modified&x=m&y=releaseblock&cells=tiles)
- [OSS-Fuzz bugs](https://bugs.chromium.org/p/oss-fuzz/issues/list?q=label%3AClusterFuzz+-status%3AWontFix%2CDuplicate&can=1)
- [Facebook’s Sapienz](https://engineering.fb.com/developer-tools/sapienz-intelligent-automated-software-testing-at-scale/) (UI fuzzing)


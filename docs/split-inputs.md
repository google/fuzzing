# How To Split A Fuzzer-Generated Input Into Several

This document describes several recommended strategies for splitting
a single fuzzer-generated input into several parts (sub-inputs).

# Examples
Splitting a fuzzer-generated input into several independent
parts (sub-inputs) is required very often. Some examples:

* Fuzzing a regular expression library requires
   * The regular expression (RE)
   * Flags for RE compilation and matching
   * A string to search the RE in

* Fuzzing an audio/video format decoder often requires
  * Decoding flags
  * Several frames

* Fuzzing a XSLT or CSS library requires
  * The stylesheet input
  * The XML/HTML input

* Fuzzing a font-rendering library requires
  * The font file
  * The text to render
  * The rendering flags

* Fuzzing a database library may require
  * The query text
  * The database state

TODO: more examples?

# Common Data Format

When trying to split the fuzzer-generated input into several parts,
the first question one needs to ask is whether the input format is common,
i.e. is it used or processed by other libraries, APIs, of fuzz targets.

If the data format is common (e.g. a widely used media format or network packet
format) then it is highly desirable for a fuzz target to consume exactly this
data format, and not some custom modification.
This way it will be easier to procure a seed corpus for this fuzz target
and to use the generated corpus to test/fuzz other targets.

## Multiple Options
If the data format may be processed by a fuzz target in a small number of different ways,
it is often the best approach to split the fuzz target into several ones,
each processing the input in exactly one way.

Make sure to [cross-pollinate](https://github.com/google/oss-fuzz/blob/master/docs/glossary.md#cross-pollination)
the corpora between these targets.
OSS-Fuzz does that automatically.

## Embedding / Comments
When a fuzz target for a common data format requires some flags, options, or
additional auxiliary sub-input(s), it is sometimes possible to embed the extra input
inside a custom section or a comment of the main data format.

Examples:
* PNG allows custom "chunks", and so a fuzz target for a PNG decoder can
  hide the flags used during PNG processing
  in a separate PNG chunk, e.g. `fUZz`
  ([example](https://github.com/google/oss-fuzz/blob/master/projects/libpng-proto/libpng_transforms_fuzzer.cc)).
* When fuzzing C/C++/Java/JavaScript inputs one may hide a sub-input in a single-line `//` comment. TODO: example?


## Hash
When only one small fixed-size sub-input is required (such as flags / options),
the fuzz target may compute a hash function on the full input and use it as the flag bits.
This option is very easy to implement, but it's applicability is limited to
relatively simple cases. The major problem is that a small local mutation of the input
leads to a large change in the sub-input, which often makes fuzzing less
efficient. Try this approach if the flags are individual bits and the input
type allows some bit flips in the inputs (e.g. a plain text).

TODO: example.

# Custom Serialization Format

If you **do not intend to share the corpus** with any other API or fuzz targets,
then a custom serialization format might be a good option for a multi-input fuzz
target.

## First / Last Bytes

When only one fixed-size sub-input is required (such as flags / options),
it is possible to treat the first (or last) `K` bytes of the input as sub-input,
and the rest of the bytes as the main input.

Just remember to copy the main input into a separate heap buffer of `Size - K`
bytes, so that buffer under/overflows on the main input are detected.

TODO: example.

## Magic separator

Choose a 4-byte (or 8-byte) magic constant that will serve as a separator
between the inputs.
In the fuzz target, split the input using this separator. Use `memmem` to
find the separator in the input -- `memmem` is known to be friendly to fuzzing
engines, at least to libFuzzer.

Example (see full code [here](https://github.com/llvm-mirror/compiler-rt/blob/6cd423889971c0d97801a9f3b9b5afb91ae9c137/test/fuzzer/MagicSeparatorTest.cpp)):
```cpp
// Splits [data,data+size) into a vector of strings using a "magic" Separator.
std::vector<std::vector<uint8_t>> SplitInput(const uint8_t *Data, size_t Size,
                                     const uint8_t *Separator,
                                     size_t SeparatorSize) { ... }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  const uint8_t Separator[] = {0xDE, 0xAD, 0xBE, 0xEF};
  auto Inputs = SplitInput(Data, Size, Separator, sizeof(Separator));
  // Use Inputs.size(), Inputs[0], Inputs[1], ...
}
```

It is relatively easy for a modern fuzzing engine to discover the separator,
but nevertheless we recommend to provide several seed inputs with the desired number
of separators.

## Fuzz Data Provider

TODO

## Type-lenth-value
A custom [Type-lenth-value](https://en.wikipedia.org/wiki/Type-length-value), or TLV,
may sound like a good solution. However, we typically **do not recommend using a custom TLV**
to split your fuzzer-generated input for the following reasons:
* This is more test-only code for you to maintain, and easy to get wrong
* Typical mutations performed by fuzzing engines, such as inserting a byte,
  will break the TLV structure too often, making fuzzing less efficient

However, a TLV input combined with a custom mutator might be a good option.
See [Structure-Aware Fuzzing](https://github.com/google/fuzzer-test-suite/blob/master/tutorial/structure-aware-fuzzing.md).

## Protobufs

Yet another option is to use one of the general-purpose serialization formats,
such as Protobufs, in combination with a custom mutator.
See [Structure-Aware Fuzzing](https://github.com/google/fuzzer-test-suite/blob/master/tutorial/structure-aware-fuzzing.md).

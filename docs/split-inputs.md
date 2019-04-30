# How To Split A Fuzzer-Generated Input Into Several

This document describes several recommended strategies for splitting
a single fuzzer-generated input into several.

# Examples
Splitting a fuzzer-generated input into several independent inputs is required
very often. Some examples:

* Fuzzing a regular expression library requres
   * The regular expression (RE),
   * Flags for RE compilation and matching,
   * A string to search the RE in.

* Fuzzing a audio/video format decoder often requires
  * Decoding flags,
  * Several frames

* Fuzzing a XSLT or CSS library requires
  * The stylesheet input,
  * The XML/HTML input.

* Fuzzing a font-rendering library requires
 * The font file,
 * The text to render,
 * Therendering flags.

 TODO: more examples?

# Common vs Custom Data Format

When trying to split the fuzzer-generated input into several,
the first question one needs to ask is whether the input format is common,
i.e. is used or processed by other libraries, API, of fuzz targets.

If the data format is common (e.g. a widely used media format or network packet
format) then it is highly desirable for a fuzz target to consume exactly this
data format, and not some custom modification.
This way it will be easier to procure a seed corpus for this fuzz target
and to use the generated corpus to test/fuzz other targets.

# Embedding / Comments

TODO

# First / Last Bytes

TODO

# Hash

TODO

# Custom Serialization Format

If you do not intend to share the inputs with any other API or fuzz targets,
then a custom serialization format might be a good option.

## Magic separator

Choose a 4-byte (or 8-byte) magic constant that will serve as a separator
between the inputs.
In the fuzz target, split the input using this separator. Use `memmem` to
find the separator in the input -- `memmem` is known to be friendly to fuzzing
engines, at least to libFuzzer).

Example (see full code [here](https://github.com/llvm-mirror/compiler-rt/blob/master/test/fuzzer/MagicSeparatorTest.cpp)):
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
* this is more test-only code for you to maintain, and easy to get wrong.
* typicall mutations performed by fuzzing engines, such as inserting a byte,
  will break the TLV structure too often, making fuzzing less efficient.

## Protobufs

Yet another option is to use one of the general-purpose serialization formats,
such as Protobufs, in combination with a custom mutator.
See [Structure-Aware Fuzzing](https://github.com/google/fuzzer-test-suite/blob/master/tutorial/structure-aware-fuzzing.md).

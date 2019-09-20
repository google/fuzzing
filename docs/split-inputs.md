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

## Fuzzed Data Provider

[FuzzedDataProvider] is a single-header C++ library that is helpful for
splitting a fuzz input into multiple parts of various types. It is a part of
LLVM and can be included via `#include <fuzzer/FuzzedDataProvider.h>` directive.
If your compiler doesn't have this header (in case it's an older Clang version
or some other compiler), you can copy the header from [here] and add it to your
project manually. It should just work, as the header doesn't depend on LLVM.

An advantage and disadvantage of using this library is that the input splitting
happens dynamically, i.e. you don't need to define any structure of the input.
This might be very helpful in certain cases, but would also make the corpus to
be no longer in a particular format. For example, if you fuzz an image parser
and split the fuzz input into several parts, the corpus elements will no longer
be valid image files, and you won't be able to simply add image files to your
corpus.

[FuzzedDataProvider] is a class whose constructor accepts `const uint8_t*,
size_t` arguments. Below is a quick overview of the available methods.

### Methods for extracting individual values

* `ConsumeBool`, `ConsumeIntegral`, `ConsumeIntegralInRange` methods are helpful
  for extracting a single boolean or integer value (the exact type is defined by
  a template parameter), e.g. some flag for the target API, or a number of
  iterations for a loop, or length of a part of the fuzz input.
* `ConsumeProbability`, `ConsumeFloatingPoint`, `ConsumeFloatingPointInRange`
  methods are very similar to the ones mentioned above. The difference is that
  these methods return a floating point value.
* `ConsumeEnum` and `PickValueInArray` methods are handy when the fuzz input
  needs to be selected from a predefined set of values, such as an enum or an
  array.

These methods are using the last bytes of the fuzz input for deriving the
requested values. This allows to use valid / test files as a seed corpus in some
cases.

### Methods for extracting sequences of bytes

Many of these methods have a length argument. You can always know how many bytes
are left inside the provider object by calling `remaining_bytes()` method on it.

* `ConsumeBytes` and `ConsumeBytesWithTerminator` methods return a `std::vector`
  of the requested size. These methods are helpful when you know how long a
  certain part of the fuzz input should be.
* `ConsumeBytesAsString` method returns a `std::string` of the requested length.
  This is useful when you need a null-terminated C-string. Calling `c_str()` on
  the resulting object is the best way to obtain it.
* `ConsumeRandomLengthString` method returns a `std::string` as well, but its
  length is derived from the fuzz input and typically is hard to predict, though
  always deterministic. The caller must provide the max length argument.
* `ConsumeRemainingBytes` and `ConsumeRemainingBytesAsString` methods return
  `std::vector` and `std::string` objects respectively, initialized with all the
  bytes from the fuzz input that left unused.

For more information about the methods, their arguments and implementation
details, please refer to the [FuzzedDataProvider] source code. Every method has
a detailed comment in that file, and the implementation is relatively small.

### Examples of fuzz targets using `FuzzedDataProvider`

* [net_verify_name_match_fuzzer] splits the fuzz input into two parts.
* [net_http2_frame_decoder_fuzzer] reads data in small chunks in a loop in order
  to emulate a sequence of frames coming from the network connection.
* [net_crl_set_fuzzer] initialized multiple parameters and uses the rest of the
  fuzz input for the main argument (i.e. data to be parsed / processed). Note
  that using [Protobufs](#Protobufs) based fuzzing might be more efficient for
  such a target.
* [net_parse_cookie_line_fuzzer] is a slightly more sophisticated fuzz target
  that emulates different actions with different parameters initialized with the
  fuzz input.

[FuzzedDataProvider]: https://github.com/llvm/llvm-project/blob/master/compiler-rt/include/fuzzer/FuzzedDataProvider.h
[here]: https://raw.githubusercontent.com/llvm/llvm-project/master/compiler-rt/include/fuzzer/FuzzedDataProvider.h
[net_crl_set_fuzzer]: https://cs.chromium.org/chromium/src/net/cert/crl_set_fuzzer.cc?rcl=0be62a8d95f7fa1455fce1a76f0fa5b8484d0c8c
[net_http2_frame_decoder_fuzzer]: https://cs.chromium.org/chromium/src/net/spdy/fuzzing/http2_frame_decoder_fuzzer.cc?rcl=0be62a8d95f7fa1455fce1a76f0fa5b8484d0c8c
[net_parse_cookie_line_fuzzer]: https://cs.chromium.org/chromium/src/net/cookies/parse_cookie_line_fuzzer.cc?rcl=0be62a8d95f7fa1455fce1a76f0fa5b8484d0c8c
[net_verify_name_match_fuzzer]: https://cs.chromium.org/chromium/src/net/cert/internal/verify_name_match_fuzzer.cc?rcl=0be62a8d95f7fa1455fce1a76f0fa5b8484d0c8c

## Type-length-value
A custom [Type-length-value](https://en.wikipedia.org/wiki/Type-length-value), or TLV,
may sound like a good solution. However, we typically **do not recommend using a custom TLV**
to split your fuzzer-generated input for the following reasons:
* This is more test-only code for you to maintain, and easy to get wrong
* Typical mutations performed by fuzzing engines, such as inserting a byte,
  will break the TLV structure too often, making fuzzing less efficient

However, a TLV input combined with a custom mutator might be a good option.
See [Structure-Aware Fuzzing](structure-aware-fuzzing.md).

## Protobufs

Yet another option is to use one of the general-purpose serialization formats,
such as Protobufs, in combination with a custom mutator.
See [Structure-Aware Fuzzing](structure-aware-fuzzing.md).

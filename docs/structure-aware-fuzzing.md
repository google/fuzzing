# Structure-Aware Fuzzing with libFuzzer

Generation-based fuzzers usually target a single input type, generating inputs
according to a pre-defined grammar. Good examples of such fuzzers are
[csmith](https://embed.cs.utah.edu/csmith/) (generates valid C programs)
and
[Peach](https://www.peach.tech/)
(generates inputs of any type, but requires such a
type to be expressed as a grammar definition).

Coverage-guided mutation-based fuzzers, such as
[libFuzzer](http://libfuzzer.info) or
[AFL](http://lcamtuf.coredump.cx/afl/),
are not restricted to a single input type and do not require grammar
definitions.  Thus, mutation-based fuzzers are generally easier to set up and
use than their generation-based counterparts.  But the lack of an input grammar
can also result in inefficient fuzzing for complicated input types, where
any traditional mutation (e.g. bit flipping) leads to an invalid input
rejected by the target API in the early stage of parsing.

With some additional effort, however, libFuzzer can be turned into a
grammar-aware (i.e. **structure-aware**) fuzzing engine for a specific input
type.

## Example: Compression

Let us start from a simple example that demonstrates most aspects of
structure-aware fuzzing with libFuzzer.

Take a look at this
[example fuzz target](https://github.com/llvm-mirror/compiler-rt/blob/master/test/fuzzer/CompressedTest.cpp),
which consumes Zlib-compressed data, uncompresses it, and crashes if the first
two bytes of the uncompressed input are 'F' and 'U'.

```cpp
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  uint8_t Uncompressed[100];
  size_t UncompressedLen = sizeof(Uncompressed);
  if (Z_OK != uncompress(Uncompressed, &UncompressedLen, Data, Size))
    return 0;
  if (UncompressedLen < 2) return 0;
  if (Uncompressed[0] == 'F' && Uncompressed[1] == 'U')
    abort();  // Boom
  return 0;
}
```

This is a very simple target, yet traditional universal fuzzers (including
libFuzzer) have virtually no chance of discovering the crash.  Why?  Because
their mutations will operate on the compressed data, causing virtually all
generated inputs to be invalid for `uncompress`.

This is where **custom mutators** (a.k.a. libFuzzer plugins) come into play.
A custom mutator is a user-defined function with a fixed signature that does
the following:
  * Parses the input data according to the specified language grammar (in our
    example, it uncompresses the data).
    * If parsing fails, it returns a syntactically correct dummy input
      (here, it returns a compressed byte sequence `Hi`).
  * Mutates the parsed representation of the input (in our case,
    uncompressed raw data). The custom mutator *may* request libFuzzer to
    mutate some part of the raw data via the function `LLVMFuzzerMutate`.
  * Serializes the mutated representation (in our case, compresses it).

```cpp
extern "C" size_t LLVMFuzzerCustomMutator(uint8_t *Data, size_t Size,
                                          size_t MaxSize, unsigned int Seed) {
  uint8_t Uncompressed[100];
  size_t UncompressedLen = sizeof(Uncompressed);
  size_t CompressedLen = MaxSize;
  if (Z_OK != uncompress(Uncompressed, &UncompressedLen, Data, Size)) {
    // The data didn't uncompress. Return a dummy...
  }
  UncompressedLen =
      LLVMFuzzerMutate(Uncompressed, UncompressedLen, sizeof(Uncompressed));
  if (Z_OK != compress(Data, &CompressedLen, Uncompressed, UncompressedLen))
    return 0;
  return CompressedLen;
}

```

Let's run
[our example](https://github.com/llvm-mirror/compiler-rt/blob/master/test/fuzzer/CompressedTest.cpp).
First, let's compile the target without the custom mutator:

```console
% clang -O -g CompressedTest.cpp -fsanitize=fuzzer -lz
% ./a.out
...
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 2 ft: 3 corp: 1/1b lim: 4 exec/s: 0 rss: 25Mb
#2097152        pulse  cov: 2 ft: 3 corp: 1/1b lim: 4096 exec/s: 1048576 rss: 25Mb
#4194304        pulse  cov: 2 ft: 3 corp: 1/1b lim: 4096 exec/s: 1048576 rss: 25Mb
#8388608        pulse  cov: 2 ft: 3 corp: 1/1b lim: 4096 exec/s: 1198372 rss: 26Mb
#16777216       pulse  cov: 2 ft: 3 corp: 1/1b lim: 4096 exec/s: 1290555 rss: 26Mb
#33554432       pulse  cov: 2 ft: 3 corp: 1/1b lim: 4096 exec/s: 1342177 rss: 26Mb
#67108864       pulse  cov: 2 ft: 3 corp: 1/1b lim: 4096 exec/s: 1398101 rss: 26Mb
...
```

No luck. The coverage (`cov: 2`) doesn't grow because no new instrumented code in the target is executed.
Even if we also instrument Zlib, thus providing more coverage feedback during fuzzing,
libFuzzer is unlikely to discover the crash.

Now let's run the same target but this time with the custom mutator:

```console
% clang -O -g CompressedTest.cpp -fsanitize=fuzzer -lz -DCUSTOM_MUTATOR
% ./a.out
...
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 2 ft: 3 corp: 1/1b lim: 4 exec/s: 0 rss: 25Mb
#512    pulse  cov: 2 ft: 3 corp: 1/1b lim: 8 exec/s: 256 rss: 26Mb
#713    NEW    cov: 3 ft: 4 corp: 2/11b lim: 11 exec/s: 237 rss: 26Mb L: 10/10 MS: 1 Custom-
#740    NEW    cov: 4 ft: 5 corp: 3/20b lim: 11 exec/s: 246 rss: 26Mb L: 9/10 MS: 3 Custom-EraseBytes-Cus
#1024   pulse  cov: 4 ft: 5 corp: 3/20b lim: 11 exec/s: 341 rss: 26Mb
#2048   pulse  cov: 4 ft: 5 corp: 3/20b lim: 21 exec/s: 682 rss: 26Mb
#4096   pulse  cov: 4 ft: 5 corp: 3/20b lim: 43 exec/s: 1365 rss: 26Mb
#4548   NEW    cov: 5 ft: 6 corp: 4/30b lim: 48 exec/s: 1516 rss: 26Mb L: 10/10 MS: 6 ShuffleBytes-Custom
#8192   pulse  cov: 5 ft: 6 corp: 4/30b lim: 80 exec/s: 2730 rss: 26Mb
#16384  pulse  cov: 5 ft: 6 corp: 4/30b lim: 163 exec/s: 5461 rss: 26Mb
==157112== ERROR: libFuzzer: deadly signal...
    #7 0x4b024b in LLVMFuzzerTestOneInput CompressedTest.cpp:23:5
```

Here, every input that is received by the target function
(`LLVMFuzzerTestOneInput`) is valid compressed data and successfully
uncompresses. With that simple change, libFuzzer's usual mutations become
significantly more effective, and the crash can be found.


## Example: PNG

[PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics)
is a raster graphics file format. A PNG file is a sequence of
length-tag-value-checksum chunks. This data format represents a challenge for
non-specialized mutation-based fuzzing engines for these reasons:
* Every chunk contains a CRC checksum
 (although [libpng](http://www.libpng.org) allows disabling CRC checking with a
 call to `png_set_crc_action`).
* Every chunk has a length, and thus a mutation that increases the size of a
  chunk also needs to change the stored length.
* Some chunks contain Zlib-compressed data, and multiple `IDAT` chunks are
  parts of the same compressed data stream.

Here is an
[example of a fuzz target for libpng](https://github.com/google/oss-fuzz/blob/master/projects/libpng-proto/libpng_transforms_fuzzer.cc).
Non-specialized fuzzers can be relatively
effective for this target when CRC checking is disabled and a comprehensive seed
corpus is provided. But libFuzzer with a custom mutator
([example](https://github.com/google/fuzzer-test-suite/blob/master/libpng-1.2.56/png_mutator.h))
is even more effective. This example
mutator parses the PNG file into an in-memory data structure, mutates it,
and serializes the mutant back to PNG.

This custom mutator also does an extra twist: it randomly inserts a special
`fUZz` chunk that the fuzz target may later perform additional mutations on to
provide more coverage.

The resulting fuzzer achieves higher coverage starting from an empty corpus
than the same target does without the custom mutator, even with a good seed
corpus and many more iterations!

## Example: Protocol Buffers

Interface Definition Languages (IDLs), such as
[Protocol Buffers](https://developers.google.com/protocol-buffers/) (a.k.a. protobufs),
[Mojo](https://chromium.googlesource.com/chromium/src/+/master/mojo/README.md),
[FIDL](https://fuchsia.googlesource.com/fuchsia/+/master/docs/development/languages/fidl/README.md),
or [Thrift](https://thrift.apache.org/)
are all good examples of highly structured input types that are hard to fuzz
with generic mutation-based fuzzers.

Structure-aware fuzzing for IDLs is possible with libFuzzer using custom
mutators. One such mutator is implemented for protobufs:
[libprotobuf-mutator](https://github.com/google/libprotobuf-mutator) (a.k.a. LPM).

Let's look at the
[example proto definition](https://github.com/google/libprotobuf-mutator/blob/master/examples/libfuzzer/libfuzzer_example.proto)
and the corresponding
[fuzz target](https://github.com/google/libprotobuf-mutator/blob/master/examples/libfuzzer/libfuzzer_example.cc).

```protobuf
message Msg {
  optional float optional_float = 1;
  optional uint64 optional_uint64 = 2;
  optional string optional_string = 3;
}
```

```cpp
DEFINE_PROTO_FUZZER(const libfuzzer_example::Msg& message) {
  // Emulate a bug.
  if (message.optional_string() == "FooBar" &&
      message.optional_uint64() > 100 &&
      !std::isnan(message.optional_float()) &&
      std::fabs(message.optional_float()) > 1000 &&
      std::fabs(message.optional_float()) < 1E10) {
    abort();
  }
}

```

Here the crash will happen if the 3 fields of the message have specific values.

Note that LPM provides a convenience macro `DEFINE_PROTO_FUZZER` to define a
fuzz target that directly consumes a protobuf message.

Here are some real life examples of fuzzing protobuf-based APIs with libFuzzer
and LPM:
* [config_fuzz_test](https://github.com/envoyproxy/envoy/blob/568b2573341151b2d9f3c7e7db6ebb33380029c8/test/server/config_validation/config_fuzz_test.cc)
fuzzes the [Envoy](https://github.com/envoyproxy/envoy) configuration API.
* TODO

## Protocol Buffers As Intermediate Format

Protobufs provide a convenient way to serialize structured data,
and LPM provides an easy way to mutate protobufs for structure-aware fuzzing.
Thus, it is tempting to use libFuzzer+LPM for APIs that consume structured data
other than protobufs.

When fuzzing a data format `Foo` with LPM, these steps need to be made:
* Describe `Foo` as a protobuf message, say `FooProto`. Precise mapping from Foo
  to protobufs may not be possible, so `FooProto` may describe a subset of a superset of `Foo`.
* Implement a `FooProto` => `Foo` converter.
* Optionally implement a `Foo => FooProto` converter. This is more important if
  there's already an extensive corpus of `Foo` inputs you'd like to use.

Below we discuss several real-life examples of this approach.

### Example: SQLite

In Chromium, the SQLite database library backs many features, including WebSQL, which exposes SQLite to arbitrary websites and makes SQLite an interesting target for malicious websites. Because SQLite of course uses the highly structured, text-based SQL language, it is a good candidate for structure-aware fuzzing. Furthermore, it has a [very good description](https://www.sqlite.org/lang.html) of the language it consumes.

The first step is to convert this grammar into the protobuf format, which can be seen [in the Chromium source tree](https://chromium.googlesource.com/chromium/src/third_party/+/refs/heads/master/sqlite/fuzz/sql_query_grammar.proto). As a quick, simplified example, if we only wanted to fuzz the `CREATE TABLE` sql statement, we could make a protobuf grammar as such:

```protobuf
message SQLQueries {
    repeated CreateTable queries = 1;
}

message CreateTable {
    optional TempModifier temp_table = 1;
    required Table table = 2;
    required ColumnDef col_def = 3;
    repeated ColumnDef extra_col_defs = 4;
    repeated TableConstraint table_constraints = 5;
    required bool without_rowid = 6;
}

// Further definitions of TempModifier, Table, ColumnDef, and TableConstraint.
```

Then, we write the C++ required to convert the structured protobufs into actual textual SQL queries (the full version can be seen [in the Chromium source tree](https://chromium.googlesource.com/chromium/src/third_party/+/refs/heads/master/sqlite/fuzz/sql_query_proto_to_string.cc)):
```cpp
// Converters for TempModifier, Table, ColumnDef, and TableConstraint go here.

std::string CreateTableToString(const CreateTable& ct) {
    std::string ret("CREATE TABLE ");
    if (ct.has_temp_table()) {
        ret += TempModifierToString(ct.temp_table());
        ret += " ";
    }
    ret += TableToString(ct.table());
    ret += "(";
    ret += ColumnDefToString(ct.col_def());
    for (int i = 0; i < ct.extra_col_defs_size(); i++) {
        ret += ", ";
        ret += ColumnDefToString(ct.extra_col_defs(i));
    }
    for (int i = 0; i < ct.table_constraints_size(); i++) {
        ret += ", ";
        ret += TableConstraintToString(ct.table_constraints(i));
    }
    ret += ") ";
    if (ct.without_rowid())
        ret += "WITHOUT ROWID ";
    return ret;
}

std::string SQLQueriesToString(const SQLQueries& queries) {
    std::string queries;
    for (int i = 0; i < queries.queries_size(); i++) {
        queries += CreateTableToString(queries.queries(i));
        queries += ";\n";
    }
    return queries;
}
```

And finally, we write our fuzz target:
```cpp
DEFINE_BINARY_PROTO_FUZZER(const SQLQueries& sql_queries) {
    std::string queries = SQLQueriesToString(sql_queries);
    sql_fuzzer::RunSQLQueries(queries); // Helper that passes our queries to sqlite library to execute
}
```

With luck, libFuzzer and LPM will be able to create many interesting `CREATE TABLE` statements, with varying numbers of columns, table constraints, and other attributes. This basic definition of `SQLQueries` can be expanded to work with other SQL statements like `INSERT` or `SELECT`, and with care we can cause these other statements to insert or select from the tables created by the random `CREATE TABLE` statements. Without defining this protobuf structure, it's very difficult for a fuzzer to be able to generate valid `CREATE TABLE` statements that actually create tables without causing parsing errors&mdash; especially tables with valid table constraints.

## Fuzzing Stateful APIs

So far we have discussed fuzzing for APIs that consume a single structured input.
Some APIs could be very different. An API may not consume data directly at all,
and it could consist of many functions that work only when the API is in a certain
state. Such **stateful APIs** are common for e.g. networking software.
Fuzzing with protobufs could be useful here as well. All you need is to define a
protobuf message describing a sequence of API calls (or a *trace*) and implement a function to
*play* the trace.

### Example: gRPC API Fuzzer
The
[gRPC](https://github.com/grpc/grpc)'s
[API Fuzzer](https://github.com/grpc/grpc/blob/86953f66948aaf49ecda56a0b9f87cdcf4b3859a/test/core/end2end/fuzzers/api_fuzzer.cc)
is actually not using libFuzzer's custom mutator or protobufs.
But it's still a good and simple example of fuzzing a stateful API.
The fuzzer consumes an array of bytes and every individual byte is
interpreted as a single call to a specific API function
(in some cases, following bytes are used as parameters).

```cpp
    switch (grpc_fuzzer_get_next_byte(&inp)) {
      default:
      // terminate on bad bytes
      case 0: {
        grpc_event ev = grpc_completion_queue_next(...
      case 1: {
        g_now = gpr_time_add(...
```

This fuzz target is compatible with any mutation-based fuzzing engine
and has resulted in over
[100 bug reports](https://bugs.chromium.org/p/oss-fuzz/issues/list?can=1&q=label%3AProj-grpc+api_fuzzer+&colspec=ID+Type+Component+Status+Proj+Reported+Owner+Summary&cells=ids),
some discovered with libFuzzer and some with AFL.

However, a drawback of this approach is that the inputs created by the fuzzer are
meaningless outside of the fuzz target itself and will stop working with a slight
change in the target. They are also not human readable, which makes analysis of
bugs generated by this fuzz target more complicated.

### Example: Envoy Header Map Fuzzer
One of [Envoy](https://github.com/envoyproxy/envoy)'s fuzz targets
uses a different approach to fuzzing stateful APIs: it encodes 
a sequence of actions (a *trace*) using a
[custom protobuf message type](https://github.com/envoyproxy/envoy/blob/master/test/common/http/conn_manager_impl_fuzz.proto),
and implements a
[player for this type](https://github.com/envoyproxy/envoy/blob/master/test/common/http/header_map_impl_fuzz_test.cc).

```cpp
DEFINE_PROTO_FUZZER(const test::common::http::HeaderMapImplFuzzTestCase& input) { ...
  for (int i = 0; i < input.actions().size(); ++i) { ...
    const auto& action = input.actions(i);           ...
    switch (action.action_selector_case()) {         ...
```

This particular fuzz target has discovered at least one security regression:
[bug](https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=10038),
[fix](https://github.com/envoyproxy/envoy/pull/4245).
The [reproducer input for this bug](https://oss-fuzz.com/download?testcase_id=5689833624698880)
is a human-readable file with the message text.

Using protos for fuzzing stateful APIs might be a bit slower and a bit more
complicated than fuzzing action traces encoded as a sequence of bytes (as
described [above](#example-grpc-api-fuzzer)). But this approach is more flexible and
maintainable since the protobuf type is easier to understand and extend
than a custom byte encoding.

### Example: Chrome IPC Fuzzer

Chrome contains many stateful APIs that are very hard for humans to reason about
during code review, which makes fuzzing those APIs powerful and highly productive.
One such example is the AppCache subsystem. This is an old attempt at a richer
caching mechanism for HTTP that aims to make some applications available offline.

In Chrome, this is implemented in an interface between the sandboxed renderer process
and the privileged browser process, which runs unsandboxed. The API available to the
renderer process [is the following](https://cs.chromium.org/chromium/src/third_party/blink/public/mojom/appcache/appcache.mojom):

```cpp
// AppCache messages sent from the child process to the browser.
interface AppCacheBackend {
  RegisterHost(int32 host_id);
  UnregisterHost(int32 host_id);
  SetSpawningHostId(int32 host_id, int32 spawning_host_id);
  SelectCache(int32 host_id,
              url.mojom.Url document_url,
              int64 appcache_document_was_loaded_from,
              url.mojom.Url opt_manifest_url);
  SelectCacheForSharedWorker(int32 host_id, int64 appcache_id);
  MarkAsForeignEntry(int32 host_id,
                     url.mojom.Url document_url,
                     int64 appcache_document_was_loaded_from);
  [Sync] GetStatus(int32 host_id) => (AppCacheStatus status);
  [Sync] StartUpdate(int32 host_id) => (bool success);
  [Sync] SwapCache(int32 host_id) => (bool success);
  [Sync] GetResourceList(int32 host_id) => (array<AppCacheResourceInfo> resources);
};
```

A single AppCacheBackend is a stateful object that runs in the browser process
and is responsible for handling all of these messages. As part of its normal
operation, it's also possible for the backend to make HTTP requests. The content
of the responses to those requests actually affects control flow. Because they are
external, they are also part of the attack surface.

With this background, we can write a protobuf specification that lets us perform
a sequence of API calls to the backend and also handle any requests it makes. The
Chrome code facilitates this testing by letting us override the source of network
data to our local fuzzed inputs.

Here is a snippet from our fuzzer protobuf specification:

```protobuf
message Session {
  repeated Command commands = 1;
}

// Based on blink::mojom::AppCacheBackend interface
// See third_party/blink/public/mojom/appcache/appcache.mojom
message Command {
  oneof command {
    RegisterHost register_host = 1;
    UnregisterHost unregister_host = 2;
    SelectCache select_cache = 3;
    SetSpawningHostId set_spawning_host_id = 4;
    SelectCacheForSharedWorker select_cache_for_shared_worker = 5;
    MarkAsForeignEntry mark_as_foreign_entry = 6;
    GetStatus get_status = 7;
    StartUpdate start_update = 8;
    SwapCache swap_cache = 9;
    GetResourceList get_resource_list = 10;
    DoRequest do_request = 11;
    RunUntilIdle run_until_idle = 12;
  }
}

// We only need a few hosts to encapsulate all the logic
enum HostId {
  HOST_N2 = -2;
  HOST_N1 = -1;
  HOST_0 = 0;
  HOST_1 = 1;
  HOST_2 = 2;
}

message RegisterHost {
  required HostId host_id = 1;
}
```

We set up our protobuf to fuzz a sequence of "commands", which represent
API calls. Note that while the RegisterHost API takes a uint32_t as the
input, we only provide a few possible values that trigger interesting
control flow in the actual implementation (namely the normal case and
small negative numbers, which represent "preallocated" hosts).

Now let's look at how to handle HTTP requests:

```protobuf
enum HttpCode {
  RESPONSE_100 = 100;
  RESPONSE_200 = 200;
  RESPONSE_206 = 206;
  RESPONSE_301 = 301;
  RESPONSE_302 = 302;
  RESPONSE_303 = 303;
  RESPONSE_304 = 304;
  RESPONSE_307 = 307;
  RESPONSE_308 = 308;
  RESPONSE_401 = 401;
  RESPONSE_403 = 403;
  RESPONSE_404 = 404;
  RESPONSE_500 = 500;
  RESPONSE_501 = 501;
}

message ManifestResponse {
  repeated Url urls = 1;
}

// Make sure to test logic when fetching more than the max concurrent allowed.
enum UrlTestCaseIndex {
  EMPTY = 0;
  PATH_1 = 1;
  PATH_2 = 2;
  PATH_3 = 3;
  PATH_4 = 4;
  PATH_5 = 5;
}

message Url {
  required UrlTestCaseIndex url_test_case_idx = 1;
}

message DoRequest {
  required HttpCode http_code = 1;
  required bool do_not_cache = 2;
  required ManifestResponse manifest_response = 3;
  required Url url = 4;
}
```

To design the `DoRequest` message required reviewing the AppCache code manually.
The features that affect control flow in the AppCache backend are the HTTP codes,
headers indicating whether or not to cache the response, the manifest content for
manifest requests, and the requested URL. We store the URL as part of the
message to handle two possible situations: either we precache a response to be ready
immediately as soon as the request comes in, or we respond to a pending request.
The second case was needed for a Chrome sandbox escape bug, which is why we model
it here.

The C++ looks like this. Note that it uses the Session message as the fundamental
fuzzed message type:
```cpp

DEFINE_BINARY_PROTO_FUZZER(const fuzzing::proto::Session& session) {
  network::TestURLLoaderFactory mock_url_loader_factory;
  SingletonEnv().InitializeAppCacheService(&mock_url_loader_factory);

  // Create a context for mojo::ReportBadMessage.
  mojo::Message message;
  auto dispatch_context =
        std::make_unique<mojo::internal::MessageDispatchContext>(&message);

  blink::mojom::AppCacheBackendPtr host;
  AppCacheDispatcherHost::Create(SingletonEnv().appcache_service.get(),
                                 /*process_id=*/1, mojo::MakeRequest(&host));

  for (const fuzzing::proto::Command& command : session.commands()) {
    switch (command.command_case()) {
      case fuzzing::proto::Command::kRegisterHost: {
        int32_t host_id = command.register_host().host_id();
        host->RegisterHost(host_id);
        break;
      }
  // ...
      case fuzzing::proto::Command::kDoRequest: {
        uint32_t code = command.do_request().http_code();
        bool do_not_cache = command.do_request().do_not_cache();
        const fuzzing::proto::ManifestResponse& manifest_response =
            command.do_request().manifest_response();
        DoRequest(&mock_url_loader_factory, command.do_request().url(), code,
                  do_not_cache, manifest_response);
        break;
      }
```

We simply set up a single instance of the AppCache backend and communicate with it
and the mocked URL loader factory that we supplied when we set it up. We implement
`DoRequest` as a helper function to handle precaching or responding to any pending
requests.

You can view the full source of the protobuf component [here](https://cs.chromium.org/chromium/src/content/browser/appcache/appcache_fuzzer.proto) and
the full source of the C++ component [here](https://cs.chromium.org/chromium/src/content/browser/appcache/appcache_fuzzer.cc).

More details:

* [Attacking Chrome IPC: Reliably finding bugs to escape the Chrome sandbox](https://media.ccc.de/v/35c3-9579-attacking_chrome_ipc)

## Conclusions

Structure-aware fuzzing is one of the "next big things" in
program state exploration and vulnerability discovery.
Probably as big as coverage-guided fuzzing has been since the early 2000s.
Admittedly, structure-aware fuzzing, at least as described in this document,
requires substantial manual work for every input type.
Finding ways to automate structure-aware fuzzing further is becoming a hot
research topic.


## Related Links

* [libprotobuf-mutator](https://github.com/google/libprotobuf-mutator) -
  Mutator for protobufs.
* [Getting Started with libprotobuf-mutator in Chromium](https://chromium.googlesource.com/chromium/src/+/master/testing/libfuzzer/libprotobuf-mutator.md).
* [Adventures in Fuzzing Instruction Selection](https://www.youtube.com/watch?v=UBbQ_s6hNgg&t=979s):
  using libFuzzer with a custom mutator for LLVM IR to find bugs in LLVM
  optimization passes.
* [Structure-aware fuzzing for Clang and LLVM with libprotobuf-mutator](https://www.youtube.com/watch?v=U60hC16HEDY).
* [AFLSmart](https://arxiv.org/pdf/1811.09447.pdf) - It makes AFL
  input-structure aware by taking in high-level structural representation of
  seed files. This avoids random bit-flip mutations as in AFL, thereby rendering
  coverage based greybox fuzzing to be highly effective in testing applications
  processing structured file formats such as PDF, PNG WAV and so on.
* [syzkaller](https://github.com/google/syzkaller) - kernel fuzzer.
* [QuickCheck](http://hackage.haskell.org/package/QuickCheck): QuickCheck: Automatic testing of Haskell programs. Clones for other languages are available too. Essentially, a generation-based fuzzer. 
* [JQF](https://github.com/rohanpadhye/jqf): Combines QuickCheck-like generators with coverage-guided fuzzing for Java. One can write a *generator* function, which simply returns a random object of some type (e.g. an AST). JQF automatically biases the generator using code coverage feedback. The tool has found new bugs in software such as the Google Closure Compiler.

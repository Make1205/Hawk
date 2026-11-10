# Benchmark notes

Initial worktree: clean. `bench` was created from the recorded initial commit.
Only files under `bench/` are added. Algorithm source, parameters and original
build files remain unchanged. The runner extracts the selected commit with local
`git archive`, so an older upstream revision can be measured without resetting
the working branch or rewriting files. No network is used at run time.

Initial commit: `1b9fef52559273fe7b40fe3e22968eaedd3a4c2a`.
Tested commit: `1b9fef52559273fe7b40fe3e22968eaedd3a4c2a`.

## Timing source and measurement

The four unmodified timing files come from pq-crystals/dilithium
`bba9534a3c5cbef66481cd6518d786e23ae601b5`, `ref/test/` (there is no root
`test/` directory at that revision). They match the Antrag bench copies byte
for byte. RDTSC is used; USE_RDPMC is not defined. The unchanged print_results
function receives iterations+1 timestamps for exactly iterations operations,
subtracts its own counter overhead, and computes the median. Thousand cycles
means RDTSC ticks divided by 1000. Loop/call overhead is included consistently.

System GCC: -O3 -march=native -std=gnu11. Ref additionally disables automatic
loop/SLP vectorization; algorithm SIMD intrinsics are selected only in the
upstream SIMD implementation. Exact flags are recorded per variant in
config.json, metadata.json and build logs. Messages have 32 bytes. Signing and
verification receive serialized byte keys and include all decoding/expansion
performed by the upstream byte API. KeyGen returns serialized keys.
Expanded-key Sign is reported only when a supported API is available.

QUICK=1: 11 timing operations, upstream KAT/self-tests, 100 generated signatures.
Default FULL: 10001 timing operations, upstream KAT/self-tests, 10000 generated
signatures. Only QUICK is executed in cloud. Full-scale runs are for the user's
local machine. No test assertions are disabled. The correctness and length
statistics reuse the same signature pool, using a fixed-seed key per build/set.
All signatures self-verify, cross-verify when another build exists, and are
checked for rejection after changing the message. Equality expectations are
configured explicitly; floating-point implementations can use cross verification
when byte equality is inappropriate. No result is declared complete unless all
required commands finish with zero status. The CSV contains medians and sizes;
raw timer output and correctness evidence are retained.

A bench-only xorshift64* byte stream (seed 1234567, fixed update and byte order in
driver.c) supplies reproducible test entropy where the API accepts an external
provider. This is for correctness vectors, not production randomness. For NIST
APIs the timing provider is the original randombytes implementation, renamed only
in its provider translation unit and routed through a bench wrapper; the algorithm
call sites remain unchanged. Callback timing APIs use the repository SHAKE generator; the precise test
callback and OS-seeded timing setup are documented below.

Missing system-library headers/libraries are compile/link checked at startup,
before algorithm builds. The runner never installs packages. Bash, Python 3.8+,
Git and system GCC are required; additional apt dependencies are listed below.

## Scheme-specific configuration and format

Fixed/padded signatures. Actual byte length is read by parsing the sign/low-bit fixed region and the unary high-part region; no heuristic zero stripping. HAWK_AVX2 and NTRUGEN_AVX2 explicitly select scalar or upstream SIMD. KeyGen uses integer fixed-point NTRUGEN, signing is integer-based; neither needs floating point or GMP. Native self_test includes SHAKE KATs and 100-case aggregate key/signature hash KATs for degrees 256/512/1024. Expanded-key Sign uses hawk_sign_finish_alt with a decoded key; this API also changes how the RNG is consumed and is additional information, not an isolated decode-only speedup. Timing callback uses the repository SHAKE with an OS seed initialized before measurements.

## Native-source compilation changes

No algorithm source patches. All source copies come from the tested commit.
Additional source selection, preprocessor options and include paths appear in
config.json. Configuration-only exceptions, if any, are documented below.

## Timing file checksums

* cpucycles.c: `31cc4c16b1e896b2cf7cc154aa21430329c0047cf9898a1865f9bfe9904f25e4`
* cpucycles.h: `7f1e2b68cf3e3c76076364af3c5350de61442450c477aec809da8de10e131838`
* speed_print.c: `533fc7bd12204078a5390cfb58c05d550b214850876609f8e98f7377b9d4f1b3`
* speed_print.h: `3946b0226c63125664eb443a20d43563e6d1ba7fa3be5c1b952cd29d1477b2ef`

Additional apt dependencies: none beyond build-essential, python3 and git.

The requested 1b9fef5 is the initial commit itself and `git ls-remote
https://github.com/hawk-sign/dev.git HEAD` returned the identical full hash.
The fork therefore contains exactly the requested upstream implementation.
The source provenance is confirmed by content identity; GitHub fork-network
metadata is separate from that Git evidence.

HAWK_AVX2=0/1 and NTRUGEN_AVX2=0/1 explicitly select both scheme and keygen
paths. Both KeyGen and Sign are integer-only at this revision; NTRUGEN uses
integer fixed-point arithmetic. No GMP or floating-point library is linked.
The README's historical HAWK_FMA note does not describe an active switch in
these sources. The proposed floating-point division code in hawk_vrfy.c is
commented out and is not enabled by the measured configuration.

The original tests/test_self.c contains SHAKE known answers and strict aggregate
hash KATs for 100 key/signature cases at each of degrees 256, 512 and 1024.
It is compiled unchanged against each selected backend. The staged test tree
receives a byte-identical copy of src/hawk.h at Reference_Implementation/hawk.h,
the generated-distribution path expected by its relative include. No algorithm
file is patched, and no generated distribution is written into the checkout.

Public pk/signature fixed sizes match 1024/555 and 2440/1221. The bench parses
the actual LSB-first unary coding after the salt/sign/low-bit regions, then
cross-checks the resulting byte count against hawk_decode_signature() coefficients
and the native sign-magnitude-complement coding rule. It keeps the salt and
last partial byte and excludes only whole padding bytes.

Each generated test signature has an additional decoded-key/alt-API signature
check. The additional ExpandedSign row uses hawk_sign_finish_alt, which consumes
RNG output directly instead of seeding the standard signing SHAKE schedule;
it is not a pure decode-only performance comparison. The timing RNG callback is
an OS-seeded instance of the repository SHAKE, initialized before measurement.
The cross-test callback instead supplies the bench deterministic byte stream;
the native KAT uses its own original deterministic SHAKE callback unchanged.

GitHub REST metadata for Make1205/Hawk was also checked: fork=true,
parent.full_name=hawk-sign/dev and source.full_name=hawk-sign/dev. Thus both the
GitHub fork relationship and the exact requested upstream Git revision are confirmed.

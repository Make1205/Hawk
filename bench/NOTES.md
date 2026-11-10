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

Before any algorithm build, the runner also compile/links a standard C
header/runtime/libm probe. Failure reports apt package build-essential
(including libc6-dev). Exact extra per-build flags are printed near the
SUMMARY heading as well as retained in metadata and logs. This final
preflight/report change leaves the measured driver and QUICK values unchanged.


## FP contraction policy update (2026-11-10)

The shared rule in PROTOCOL.md now requires -ffp-contract=off for all actual
floating-point builds. The local c-fn-dsa index-3461 mismatch was traced to
compiler contraction (15 FMA -> 0; equality restored), not a source defect.
All runs now audit benchmark-owned objects and the linked driver with objdump
and save *-fma-audit.json. Missing objdump fails preflight: apt install binutils.
HAWK KeyGen/Sign are integer/fixed-point, with no double arithmetic or GMP. The only proposed floating verification block in hawk_vrfy.c is commented out. Thus no contraction flag is required or added. The existing arithmetic/timing boundaries are preserved; the old bench-added -fno-tree-vectorize/-fno-tree-slp-vectorize options are removed to follow the common GCC rule. Both backends must audit to zero FMA.


## GCC 11.4 scalar signer failure and minimal build workaround

User failure: 512-ref native selftest reported `8: verify error`. The prefix
8 is logn=8, i.e. n=256 in the native multi-degree selftest, not sample 8 or
n=512. The user's local result directory is not in this cloud checkout.

An unmodified Ubuntu GCC 11.4.0-1ubuntu1~22.04.3 toolchain was extracted into a
user-owned cloud workspace solely for diagnosis, without sudo/system changes.
The byte-identical pinned native sources and selftest reproduce the same error
with -O3 -march=skylake, HAWK_AVX2=0, NTRUGEN_AVX2=0. Skylake selects the
AVX2-class ISA of the user's i7-10700K; this is not a claim of identical hardware.
Normal bench/run.sh still uses the system GCC, not this diagnostic toolchain.

| GCC 11.4 full scalar build | Native selftest |
|---|---|
| -O3 -march=skylake | FAIL: 8: verify error |
| plus -fno-tree-loop-vectorize | FAIL |
| plus -fno-tree-slp-vectorize | PASS |
| plus -fno-tree-vectorize | PASS |
| plus -fno-strict-aliasing | PASS |
| plus -fwrapv | FAIL |
| -O2 -march=skylake | PASS |
| -O3 -march=x86-64 | PASS |

Mixed-object links localize the fault: replacing **only hawk_sign.c** with its
-fno-strict-aliasing or no-SLP object passes. Replacing only hawk_vrfy.c or only
hawk_kgen.c does not. Disabling only loop vectorization in the signer still
fails. Thus the verifier rejects a signature produced by the scalar signer;
the error message does not establish a verifier bug. GCC14/native in cloud
previously passed, so the compiler version/ISA/SLP interaction matters.

Upstream Reference_Implementation/Makefile and Optimized_Implementation/avx2/
Makefile specify c99 and -O2 with warnings; they do **not** mandate a
-fno-tree-vectorize or -fno-strict-aliasing flag. Native -O2 is a validated
fallback, not the same setting as the requested O3 benchmark.

Chosen minimal benchmark adjustment: **only ref hawk_sign.c adds
-fno-strict-aliasing**; all other objects remain -O3 -march=native and both
loop/SLP vectorization stay enabled. The AVX2 build is unchanged. source_flags
in config.json, build logs and metadata record this exception. No algorithm
source line, parameter, RNG or encoding is changed.

The scalar signer contains packed-buffer accesses through uint64_t pointers
(e.g. bp_xor_64/128/256/512 in src/hawk_sign.c:302ff). The experiments establish
SLP/alias-optimization sensitivity and the per-file workaround, but do not by
themselves prove a GCC defect versus a precise C effective-type/aliasing UB
violation. We report that boundary explicitly rather than label GCC faulty
without a minimized standards-level reproducer. Evidence and compiler package
hashes are in diagnosis/gcc11-scalar-signer/. The original native KAT/selftest
and cross-verification remain mandatory; the build does not retry silently with
weaker flags after a failed test.


### LOGN hypothesis checked explicitly

The original test_self.c always invokes test_multiple_sigs(8,...), (9,...),
(10,...), each with 100 cases and its original expected hash. Native src/*.c/h
and test_self.c never reference the uppercase LOGN macro: logn is a runtime API
argument. Only bench/adapter.h uses LOGN for its buffer sizes and selected API
argument. Thus defining LOGN=9 does not restrict the linked native implementation
to n=512. All ten GCC11 scalar algorithm object files are **byte-identical** with
and without -DLOGN=9 (hash evidence LOGN-macro-proof.json). The failing baseline
compiler matrix already omitted LOGN and still reproduced `8: verify error`.
Removing that macro alone therefore cannot fix the reported failure.

For build hygiene, LOGN is now in driver_flags only: the native universal objects
and selftest are compiled without it, and the timing driver alone gets 9 or 10.
The selftest continues to cover all three degrees; n=256 is not removed to hide
the error. Its scalar signer still needs the measured per-file alias workaround.
The earlier cloud run had GCC14/native on a Zen4-class CPU and passed all 8/9/10
cases; the older local build disabled SLP globally. Neither fact contradicts
this compiler/ISA/alias optimization sensitivity. A mixed-object GCC14/Skylake
signer also failed with the GCC11 baseline, so this is not claimed to be a
uniquely GCC11 defect fixed by every later compiler. No minimized GCC-vs-UB
classification is asserted.


### Timing protocol audit (same update)

The old driver used separate timing groups with a reused signing/verification key.
It now uses the shared Lithium driver: 20 discarded fresh-key triples, then a
fresh keypair and signature on every timed triple. Messages remain 32 bytes and
Sign/Verify start with serialized keys. Expanded-key Sign is supplemental.
This timing alignment requires a new HAWK FULL run independently of the compiler
workaround. Correctness corpus and upstream selftest semantics are unchanged.

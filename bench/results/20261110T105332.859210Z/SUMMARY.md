# HAWK — QUICK

QUICK is a workflow check only. Full-scale figures are measured locally.

CPU: AMD EPYC 9V74 80-Core Processor; affinity: 0. GCC: gcc (Debian 14.2.0-19) 14.2.0.

Tested commit: `1b9fef52559273fe7b40fe3e22968eaedd3a4c2a`. Benchmark commit: `feb97d54cb52647785053fc57bd6bfc18514bbff`.

Base flags: `-O3 -march=native -std=gnu11`. Each operation: 11 iterations; correctness and lengths: 100 signatures per build/set. Message: 32 bytes. Median units: thousand RDTSC ticks.

Exact per-build options (in addition to base flags and local include paths):

* 512 ref: `-DLOGN=9 -DHAWK_AVX2=0 -DNTRUGEN_AVX2=0`; link ``.
* 512 avx2: `-DLOGN=9 -DHAWK_AVX2=1 -DNTRUGEN_AVX2=1`; link ``.
* 1024 ref: `-DLOGN=10 -DHAWK_AVX2=0 -DNTRUGEN_AVX2=0`; link ``.
* 1024 avx2: `-DLOGN=10 -DHAWK_AVX2=1 -DNTRUGEN_AVX2=1`; link ``.

| 参数集 | n | q | pk | sig（仓库实际） | 论文 sig | ref KeyGen/Sign/Verify | AVX2 KeyGen/Sign/Verify |
|---|---:|---:|---:|---|---:|---|---|
| 512 | 512 | -- | 1024 | 555; unpadded avg 536.3000, max 548 | 555 | 6083.869/270.503/138.501 | 3810.117/36.659/53.065 |
| 1024 | 1024 | -- | 2440 | 1221; unpadded avg 1197.6000, max 1214 | 1221 | 28676.075/378.273/285.479 | 15754.543/77.869/110.499 |



Expanded-key Sign (thousand ticks; `--` means no supported expanded-key API):

| 参数集 | ref | AVX2 |
|---|---:|---:|
| 512 | 161.849 | 89.283 |
| 1024 | 323.361 | 176.149 |

Correctness: all configured upstream KAT/self-tests passed; all generated signatures verified; changed messages rejected. Cross-build/equality details are in correctness.json.


FMA audit: selected integer/fixed-point paths; zero FMA instructions in all benchmark-owned objects and linked drivers. See *-fma-audit.json.
Fixed/padded signatures. Actual byte length is read by parsing the sign/low-bit fixed region and the unary high-part region; no heuristic zero stripping. HAWK_AVX2 and NTRUGEN_AVX2 explicitly select scalar or upstream SIMD. KeyGen uses integer fixed-point NTRUGEN, signing is integer-based; neither needs floating point or GMP. Native self_test includes SHAKE KATs and 100-case aggregate key/signature hash KATs for degrees 256/512/1024. Expanded-key Sign uses hawk_sign_finish_alt with a decoded key; this API also changes how the RNG is consumed and is additional information, not an isolated decode-only speedup. Timing callback uses the repository SHAKE with an OS seed initialized before measurements.

Build flags and commands per variant are in config.json, metadata.json and build logs. All source is extracted locally from the tested commit; algorithm files are unchanged. See bench/NOTES.md.

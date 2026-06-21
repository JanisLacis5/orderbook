### Optimization 1: <name>

Commit:

#### Problem

Describe the observed bottleneck or suspected inefficiency.

#### Change

Describe what changed in the code.

#### Result Before

```txt
ns/command:
commands/sec:
perf counters:
```

#### Result After

```txt
ns/command:
commands/sec:
perf counters:
```

#### Conclusion

Explain whether the change helped, hurt, or was neutral. Keep the explanation honest.

## Future Benchmark Workloads

Planned benchmark scenarios:

* Add-only workload
* Cancel-heavy workload
* Modify-heavy workload
* Match-heavy workload
* Mixed random workload
* Large steady-state book workload

The current small scripted scenario should remain as a regression/smoke benchmark, but it should not be used as the only performance measurement.

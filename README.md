# wsl-demo

Smoke-test project for Orca's WSL host, living on the native Linux filesystem at
`/root/wsl-demo` (Debian (WSL) runtime), not under `/mnt/c` and not over `\\wsl.localhost`.

## Run

```sh
python3 app.py orca
```

## Test

```sh
python3 -m unittest -v
```

import sys


def greet(name: str) -> str:
    return f"hello {name}, from WSL"


def main() -> int:
    name = sys.argv[1] if len(sys.argv) > 1 else "world"
    print(greet(name))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

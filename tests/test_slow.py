from pathlib import Path
from pytest import mark
from utils import assert_command_success, assert_python_script_runs


def edit_test_file(python_file: Path, n: int) -> None:
    python_file.write_text(
        """
def main() -> None:
    nums = [1, c, d]
    print(f'The sum is {sum(nums)}')

if __name__ == "__main__":
    main()
""",
        encoding="utf-8",
    )

    instructions = f"Replace the variable `c` with 2 and `d` with {n}."

    assert_command_success(
        f"{python_file}",
        "--provider=ollama",
        f"--instructions='{instructions}'",
        f"--output={python_file}",
    )
    assert f"The sum is {1 + 2 + n}" in assert_python_script_runs(python_file)


@mark.slow
def test_omit_code_fences() -> None:
    python_file = Path("/tmp/funcgraft_test_file_2.py")
    passed = 0
    failed = 0

    print("\n")
    for n in range(1, 11):
        try:
            edit_test_file(python_file, n)
        except AssertionError:
            failed += 1
            print(f"  Trial {n} ✘")
        else:
            passed += 1
            print(f"  Trial {n} ✓")
        finally:
            if python_file.exists():
                python_file.unlink()

    success_rate = (100 * passed) / (passed + failed)
    print(f"\nSuccess rate: {success_rate}%")

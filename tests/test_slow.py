from pathlib import Path
from colorama import Fore
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


def enumerate_and_print_errors(errors: list[str]) -> None:
    if len(errors) == 0:
        return

    counts = {}

    for error in errors:
        if error not in counts:
            counts[error] = 1
        else:
            counts[error] += 1

    print("\nERRORS:\n")

    for error, count in counts.items():
        print("-" * 50)
        print(f"{Fore.LIGHTBLUE_EX}{error}{Fore.RESET}")
        print(f"Number of errors of this type: {count}\n")


@mark.slow
def test_omit_code_fences() -> None:
    python_file = Path("/tmp/funcgraft_test_file_2.py")
    passed = 0
    failed = 0

    errors = []

    print("\n")
    for n in range(1, 11):
        try:
            edit_test_file(python_file, n)
        except AssertionError as e:
            failed += 1
            print(f"  Trial {n} {Fore.RED}✘{Fore.RESET}")
            errors.append(str(e))
        else:
            passed += 1
            print(f"  Trial {n} {Fore.GREEN}✓{Fore.RESET}")
        finally:
            if python_file.exists():
                python_file.unlink()

    success_rate = (100 * passed) / (passed + failed)
    print(f"\nSuccess rate: {success_rate}%")

    enumerate_and_print_errors(errors)

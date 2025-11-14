def get_sum() -> int:
    return 10 + c
@@@


def main() -> None:
    c = 1
    nums = [c, get_sum(), 3]
    print(f"The sum is {sum(nums)}")


if __name__ == "__main__":
    main()

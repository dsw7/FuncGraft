def get_sum() -> int:
    return 10 + c


# The delimiters are next to each other so program should abort since
# the code block is empty
@@@
@@@


def main() -> None:
    c = 1
    nums = [c, get_sum(), 3]
    print(f"The sum is {sum(nums)}")

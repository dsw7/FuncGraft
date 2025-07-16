# Prototype CLI + help messages by leveraging click

import click


@click.command()
@click.option("-v", "--verbose", is_flag=True, help="Be more verbose with output")
@click.option("-m", "--model", help="Select model")
@click.option("-o", "--output", help="Place output into FILE")
@click.option("-f", "--file", help="Read instructions from FILE")
@click.option("-i", "--instructions", help="Read INSTRUCTIONS via command line")
@click.argument("filename", required=True)
def main(file):
    pass


if __name__ == "__main__":
    main()

def fill_file(file: str):
    with open(file, 'a') as f:
        f.write("0")
        for i in range(1, (128 << 10) - 1):
            f.write(f",{i}")


if __name__ == "__main__":
    fill_file('./src/lookup_table_values.def')

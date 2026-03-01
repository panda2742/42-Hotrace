import random
import sys

def generate_test_file(filename, num_pairs, num_searches):

    keys = []
    with open(filename, 'w') as f:
        for i in range(num_pairs):
            key = f"key_no_cache_hit_no_cache_L1_no_cache_L2{i}_{random.randint(1000, 9999)}"
            value = f"Lorem_ipsum_dolor_sit_amet_consectetur_adipiscing_elit_molestie._{i}"

            f.write(f"{key}\n{value}\n")
            keys.append(key)

        f.write("\n")

        for _ in range(num_searches):

            if random.random() < 0.8:
                search_key = random.choice(keys)
            else:
                search_key = f"not_a_real_key_{random.randint(0, 999999)}"

            f.write(f"{search_key}\n")

if __name__ == "__main__":

    num_insert = int(sys.argv[1])
    num_search = int(sys.argv[2])

    generate_test_file("input.htr", num_insert, num_search)
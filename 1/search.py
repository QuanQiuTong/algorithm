import edlib
import numpy as np

def calculate_value(ref, query, ref_st, ref_en, query_st, query_en):
    A = ref[:ref_st]
    a = query[:query_st]
    B = ref[ref_st:ref_en]
    b_rc = rc(query[query_st:query_en])
    C = ref[ref_en:]
    c = query[query_en:]
    return max(0, len(ref) - edlib.align(A, a)['editDistance']-edlib.align(B, b_rc)['editDistance']-edlib.align(C, c)['editDistance'])

def rc(seq):
    return seq[::-1].translate(str.maketrans('ACGT', 'TGCA'))

def read_sequence_from_file(file_path):
    with open(file_path, 'r') as file:
        return file.read()

def optimize_parameters(ref, query):
    best_value = -1
    best_params = (0, 0, 0, 0)
    ref_len = len(ref)
    query_len = len(query)
    
    # 缩小搜索范围
    step = 300
    ref_start_range = range(5000, 8000, step)
    ref_end_range = range(22000, 25000, step)
    query_start_range = range(5000, 8000, step)
    query_end_range = range(22000, 25000, step)
    
    for ref_st in ref_start_range:
        for ref_en in ref_end_range:
            for query_st in query_start_range:
                for query_en in query_end_range:
                    value = calculate_value(ref, query, ref_st, ref_en, query_st, query_en)
                    # print(f"ref_st={ref_st}, ref_en={ref_en}, query_st={query_st}, query_en={query_en}, value={value}")
                    if value > best_value:
                        best_value = value
                        best_params = (ref_st, ref_en, query_st, query_en)
    return best_params, best_value

def main():
    ref = read_sequence_from_file('reference.txt')
    query = read_sequence_from_file('sample.txt')
    best_params, best_value = optimize_parameters(ref, query)
    print(f"Best parameters: ref_st={best_params[0]}, ref_en={best_params[1]}, query_st={best_params[2]}, query_en={best_params[3]}")
    print(f"Best value: {best_value}")

if __name__ == '__main__':
    main()
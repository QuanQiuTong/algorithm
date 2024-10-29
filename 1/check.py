import edlib

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

def load(filename):
    with open(filename) as f:
        return f.read()
    
def main():
    # ref = 'ACGTACGTACGT'
    # query = 'ACGTACGTACGT'
    ref = load('reference.txt')
    query = load('sample.txt')

    ref_st = 6000
    ref_en = 24000
    query_st = 6000
    query_en = 24000
    
    print(calculate_value(ref, query, ref_st, ref_en, query_st, query_en))

if __name__ == '__main__':
    main()
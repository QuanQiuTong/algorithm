import numpy as np
import edlib


def rc(seq):
    return seq.translate(str.maketrans("ATGC", "TACG"))[::-1]

def get_points(tuples_str):

    data = []
    num = 0
    for c in tuples_str:
        if ord("0") <= c <= ord("9"):
            num = num * 10 + c - ord("0")
        elif ord(",") == c:
            data.append(num)
            num = 0
    if num != 0:
        data.append(num)
    return data


def calculate_distance(ref, query, ref_st, ref_en, query_st, query_en):
    A = ref[ref_st:ref_en]
    a = query[query_st:query_en]
    _a = rc(query[query_st:query_en])
    return min(edlib.align(A, a)["editDistance"], edlib.align(A, _a)["editDistance"])


def get_first(x):
    return x[0]


def calculate_value(tuples_str, ref, query):
    try:
        print(tuples_str)
        slicepoints = np.array(get_points(tuples_str.encode()))
        if len(slicepoints) > 0 and len(slicepoints) % 4 == 0:
            editdistance = 0
            aligned = 0
            preend = 0
            points = np.array(slicepoints).reshape((-1, 4)).tolist()
            points.sort(key=get_first)
            for onetuple in points:
                query_st, query_en, ref_st, ref_en = (
                    onetuple[0],
                    onetuple[1],
                    onetuple[2],
                    onetuple[3],
                )
                if preend > query_st:  # 检测重叠
                    return 0
                preend = query_en
                editdistance += calculate_distance(
                    ref, query, ref_st, ref_en, query_st, query_en
                )
                aligned += query_en - query_st

                print("[]{}".format(query_en - query_st - calculate_distance(
                    ref, query, ref_st, ref_en, query_st, query_en
                )))

            return max(
                aligned - editdistance - len(points) * 30, 0
            )  # 额外的惩罚碎片化的输出
        else:
            return 0
    except:
        return 0


def main():
    with open("reference.txt", "r") as f:
        ref = f.read().strip()

    with open("query.txt", "r") as f:
        query = f.read().strip()

    # with open("tuple2450.log", "r") as f:
    #     tuples = f.read()
    tuples = input("input tuples: ")

    print(calculate_value(tuples, ref, query))


if __name__ == "__main__":
    main()

# (0,2300,0,2400),(2600,9000,2700,9260),(10740, 19407, 12700, 21638)
# 11867
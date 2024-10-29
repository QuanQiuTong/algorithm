import matplotlib.pyplot as plt
import edlib

def calculate_edit_distance(ref, query):
    result = edlib.align(ref, query, mode='NW', task='distance')
    return result['editDistance']

def plot_edit_distance_curve(ref, query):
    lengths = range(1, min(len(ref), len(query)) + 1)
    distances = []

    for length in lengths:
        ref_prefix = ref[:length]
        query_prefix = query[:length]
        distance = calculate_edit_distance(ref_prefix, query_prefix)
        distances.append(distance)

    plt.plot(lengths, distances, marker='o')
    plt.xlabel('Prefix Length')
    plt.ylabel('Edit Distance')
    plt.title('Edit Distance vs Prefix Length')
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    # ref = "AGCTTAGCAGCTAGCTAGCTAGCTAGCTAGCTAGCT"
    # query = "AGCTAGCAGCTAGCTAGCTAGCTAGCTAGCTAGCT"

    with open('reference.txt', 'r') as f:
        ref = f.read()
    
    with open('query.txt', 'r') as f:
        query = f.read()

    plot_edit_distance_curve(ref, query)
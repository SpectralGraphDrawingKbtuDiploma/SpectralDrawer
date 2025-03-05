import random
import sys

# Создаем случайный граф с заданным числом вершин и средней степенью
def generate_random_connected_graph(n_nodes, avg_degree):
    edges = set()
    
    # Сначала соединяем вершины в цепочку, чтобы гарантировать связность
    for i in range(n_nodes - 1):
        edges.add((i, i+1))

    # Добавляем случайные ребра, чтобы обеспечить нужную среднюю степень
    additional_edges = max(0, int((avg_degree * n_nodes)/2 - len(edges)))

    while len(edges) < additional_edges + (n_nodes - 1):
        u = random.randint(0, n_nodes - 1)
        v = random.randint(0, n_nodes - 1)
        if u != v:
            edge = tuple(sorted((u,v)))
            edges.add(edge)

    return edges

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python generate_testcase.py num_nodes avg_degree")
        sys.exit(1)

    num_nodes = int(sys.argv[1])
    avg_degree = int(sys.argv[2])

    edges = generate_random_connected_graph(num_nodes, avg_degree)

    output_filename = f"graph_{num_nodes}_nodes_{avg_degree}_deg.txt"

    with open(output_filename, 'w') as f:
        for u,v in edges:
            f.write(f"{u} {v}\n")

    print(f"Generated connected graph with {num_nodes} nodes and average degree {avg_degree}.")
    print(f"Saved to {output_filename}")

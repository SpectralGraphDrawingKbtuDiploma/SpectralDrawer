import numpy as np
import matplotlib.pyplot as plt

# Читаем координаты вершин
coords = []
with open("embedding.txt") as f:
    for line in f:
        x, y = map(float, line.strip().split())
        coords.append((x,y))

# Читаем ребра исходного графа
edges = []
with open("graph.txt") as f:
    for line in f:
        u, v = map(int, line.strip().split())
        edges.append((u,v))

coords_np = np.array(coords)

# Определяем границы по перцентилям (отбрасываем крайние 1%)
low_pct, high_pct = 1, 99
x_low, x_high = np.percentile(coords_np[:,0], [low_pct, high_pct])
y_low, y_high = np.percentile(coords_np[:,1], [low_pct, high_pct])

# Автоматически фильтруем вершины за пределами диапазона
valid_vertices = ((coords_np[:,0] >= x_low) & (coords_np[:,0] <= x_high) &
                  (coords_np[:,1] >= y_low) & (coords_np[:,1] <= y_high))

filtered_indices = np.where(valid_vertices)[0]
filtered_coords = coords_np[filtered_indices]

# Перестраиваем индексацию для быстрой проверки
valid_indices_set = set(filtered_indices)

# фильтруем ребра, у которых оба конца входят в valid_vertices
filtered_edges = [(u,v) for (u,v) in edges if u in valid_indices_set and v in valid_indices_set]

# График
plt.figure(figsize=(50, 50))
plt.scatter(filtered_coords[:,0], filtered_coords[:,1], c='red', s=20)

# номера вершин (можно закомментировать если много точек)
for idx in filtered_indices:
    xi, yi = coords[idx]
    plt.text(xi, yi, str(idx), fontsize=10, ha='right')

# соединяем ребрами
for u, v in filtered_edges:
    xu, yu = coords[u]
    xv, yv = coords[v]
    plt.plot([xu, xv], [yu, yv], 'b-', alpha=0.6)

plt.title("Filtered Spectral Embedding (focus on main area)")
plt.xlabel("Eigenvector 1 (Fiedler vector)")
plt.ylabel("Eigenvector 2")
plt.grid(True)
plt.axis('equal')

# сохранение + вывод
plt.savefig("graph_embedding_filtered_a.png", dpi=300, bbox_inches='tight')
print("Filtered plot saved as graph_embedding_filtered_a.png")
# plt.show()

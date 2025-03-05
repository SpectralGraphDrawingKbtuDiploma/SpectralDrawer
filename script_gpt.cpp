#include <vector>
#include <fstream>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Spectra/SymEigsSolver.h>
#include <Spectra/MatOp/SparseSymMatProd.h>

using namespace Eigen;
using namespace Spectra;

struct EigenResult {
    std::vector<double> eigenvalues;
    std::vector<std::vector<double>> eigenvectors;
};

void read_graph_edges(const std::string& filename, std::vector<std::pair<int, int>>& edges, int& max_index)
{
    std::ifstream fin(filename);
    if (!fin) throw std::runtime_error("Cannot open file: " + filename);
    int u, v; max_index = -1;
    while(fin >> u >> v) {
        edges.emplace_back(u, v);
        max_index = std::max(max_index, std::max(u, v));
    }
}

MatrixXd dense_graph_laplacian(const std::vector<std::pair<int, int>>& edges, int size)
{
    MatrixXd A = MatrixXd::Zero(size, size);
    VectorXd degrees = VectorXd::Zero(size);

    for (auto [u,v] : edges) {
        A(u,v) = A(v,u) = 1.0;
        degrees[u] += 1;
        degrees[v] += 1;
    }
    MatrixXd D = degrees.asDiagonal();
    MatrixXd L = D - A;
    return L;
}

SparseMatrix<double> sparse_graph_laplacian(const std::vector<std::pair<int, int>>& edges, int size)
{
    std::vector<Triplet<double>> triplets;
    VectorXd degrees = VectorXd::Zero(size);

    for (auto [u,v] : edges) {
        triplets.emplace_back(u, v, -1.0);
        triplets.emplace_back(v, u, -1.0);
        degrees[u] += 1;
        degrees[v] += 1;
    }

    for (int i = 0; i < size; i++)
        triplets.emplace_back(i, i, degrees[i]);

    SparseMatrix<double> L(size, size);
    L.setFromTriplets(triplets.begin(), triplets.end());
    return L;
}

EigenResult dense_solver(const MatrixXd& L, int num_eigenvalues)
{
    SelfAdjointEigenSolver<MatrixXd> solver(L);
    if(solver.info() != Success) throw std::runtime_error("Dense eigen decomposition failed.");

    EigenResult result;
    auto evals = solver.eigenvalues();
    auto evecs = solver.eigenvectors();
    result.eigenvalues.assign(evals.data(), evals.data() + num_eigenvalues);
    // result.eigenvalues.assign(evals.data(), evals.data());
    // std::reverse(result.eigenvalues.begin(), result.eigenvalues.end());
    result.eigenvectors.resize(num_eigenvalues);
    std::cout << "EVALS IS\n";
    for (auto x: result.eigenvalues) {
        std::cout << x << ' ';
    }
    std::cout << std::endl;
    for (int i = 0; i < num_eigenvalues; ++i)
        result.eigenvectors[i] = std::vector<double>(evecs.col(i).data(), evecs.col(i).data() + evecs.rows());

    return result;
}

EigenResult sparse_solver(const SparseMatrix<double>& L, int num_eigenvalues)
{
    SparseSymMatProd<double> op(L);
    int nev = num_eigenvalues;
    int ncv = std::min(2 * num_eigenvalues + 1, int(L.rows()));

    SymEigsSolver<SparseSymMatProd<double>> eigs(op, nev, ncv);
    eigs.init();
    eigs.compute();

    if (eigs.info() != CompInfo::Successful) throw std::runtime_error("Sparse eigen decomposition failed.");

    VectorXd evals = eigs.eigenvalues();
    MatrixXd evecs = eigs.eigenvectors();


    EigenResult result;
    result.eigenvalues.resize(num_eigenvalues);
    std::cout << "EVALS IS\n";
    for (auto x: result.eigenvalues) {
        std::cout << x << ' ';
    }
    std::cout << std::endl;
    for(int i = 0; i < num_eigenvalues; ++i)
        result.eigenvalues[i] = evals[i];

    result.eigenvectors.resize(num_eigenvalues);
    for(int i = 0; i < num_eigenvalues; ++i) {
        result.eigenvectors[i] = std::vector<double>(evecs.col(i).data(), evecs.col(i).data() + evecs.rows());
    }
    return result;
}

EigenResult compute_smallest_laplacian_eigenpairs(const std::string& filename, int num_eigenvalues = 3, int threshold_dense = 1000)
{
    std::vector<std::pair<int, int>> edges; int max_index;
    read_graph_edges(filename, edges, max_index);
    int size = max_index + 1;

    if(size <= threshold_dense) {
        std::cout << "Using dense solver\n";
        MatrixXd L = dense_graph_laplacian(edges, size);
        return dense_solver(L, num_eigenvalues);
    }
    else {
        std::cout << "Using sparse solver (Lanczos via Spectra)\n";
        SparseMatrix<double> L = sparse_graph_laplacian(edges, size);
        return sparse_solver(L, num_eigenvalues);
    }
}


void compute_largest_laplacian_embedding(const SparseMatrix<double>& L, const std::string& output_file, int embedding_dim = 2)
{
    using namespace Spectra;

    SparseSymMatProd<double> op(L);
    int nev = embedding_dim + 1; // ищем на один вектор больше на случай, если самый большой — тривиальный
    int ncv = std::min(2 * nev + 1, int(L.rows()));

    // Меняем SMALLEST_ALGE на LARGEST_ALGE
    SymEigsSolver<SparseSymMatProd<double>> eigs(op, nev, ncv);
    eigs.init();
    eigs.compute();

    if (eigs.info() != CompInfo::Successful)
        throw std::runtime_error("Eigen decomposition (largest) failed.");

    auto evals = eigs.eigenvalues();
    auto evecs = eigs.eigenvectors();

    // cортируем собственные значения и векторы по убыванию
    std::vector<std::pair<double, VectorXd>> eig_pairs;
    for (int i = 0; i < evals.size(); ++i)
        eig_pairs.emplace_back(evals[i], evecs.col(i));

    std::sort(eig_pairs.begin(), eig_pairs.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    // Пропускаем первый собственный вектор, берем второй и третий по порядку
    std::ofstream fout(output_file);
    int N = L.rows();
    for (int i = 0; i < N; ++i)
    {
        double x = eig_pairs[1].second[i];  // eigenvector with second largest eigenvalue
        double y = eig_pairs[2].second[i];  // eigenvector with third largest eigenvalue
        fout << x << " " << y << "\n";
    }
    fout.close();

    std::cout << "Largest eigenvectors embedding saved to " << output_file << std::endl;
}

int main()
{
    try {
        EigenResult result = compute_smallest_laplacian_eigenpairs("graph.txt", 3, 1000);

        int N = result.eigenvectors[1].size();
        std::ofstream outfile("embedding.txt");
        for(int i = 0; i < N; ++i)
        {
            double x = result.eigenvectors[1][i]; // Fiedler vector (eigenvector 1)
            double y = result.eigenvectors[2][i]; // next vector (eigenvector 2)
            outfile << x << " " << y << "\n";
        }
        outfile.close();
        // std::vector<std::pair<int, int>> edges; int max_index;
        // read_graph_edges("graph.txt", edges, max_index);
        // int size = max_index + 1;

        // SparseMatrix<double> L = sparse_graph_laplacian(edges, size);
        // compute_largest_laplacian_embedding(L, "embedding_largest.txt", 2);
    }
    catch(const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
